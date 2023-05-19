#pragma once

#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <memory>
#include <random>
#include <ctime>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <asio.hpp>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"
#include "definition.h"

namespace MCRCON
{
	class MCRCONPlugin : public qqbot::CppPlugin
	{
	public:
		struct EndPoint
		{
			std::string		host;
			unsigned short	port;
			std::string		password;
		};

		MCRCONPlugin() :
			m_mt(static_cast<unsigned int>(std::time(nullptr))),
			m_coroutineIsRunning(false)
		{
			qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
			qqbot::CppPlugin::pluginInfo.name = "mcrcon";
			qqbot::CppPlugin::pluginInfo.version = "0.0.1";

			//赋值
			m_endPoint = { "192.168.1.3", 25575, "123456" };
			m_requestID = m_mt() % 1000000000;
		}

		~MCRCONPlugin() = default;

		virtual void onEnable()
		{
			qqbot::ServerInfo::getCommander().addCommand("rcon",
				[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
				{
					if (Args.empty())
					{
						qqbot::Network::sendGroupMessage(groupID, "rcon command [command] -发送指令");
						return;
					}

					if (Args.size() == 1)
					{
						if (Args[0] == "connect")
						{
							static bool hasStart = false;
							if (m_coroutineIsRunning || hasStart)
							{
								qqbot::Network::sendGroupMessage(groupID, "连接RCON服务器协程已经在运作");
								return;
							}
							this->connect();
							hasStart = true;
							qqbot::Network::sendGroupMessage(groupID, "连接RCON服务器协程启动成功");
						}
						/*else if (Args[0] == "disconnect")
						{
							if (!m_coroutineIsRunning)
							{
								qqbot::Network::sendGroupMessage(groupID, "连接RCON服务器协程已经停止");
								return;
							}
							this->disconnect();
							qqbot::Network::sendGroupMessage(groupID, "连接RCON服务器协程终止成功");
						}*/
						else if (Args[0] == "state")
						{
							if (m_coroutineIsRunning)
							{
								qqbot::Network::sendGroupMessage(groupID, "RCON服务器协程正在运作");
								return;
							}
							else
							{
								qqbot::Network::sendGroupMessage(groupID, "RCON服务器协程已停止");
								return;
							}
						}
						else
						{
							qqbot::Network::sendGroupMessage(groupID, "参数错误");
							return;
						}
					}
					else if (Args[0] == "command")
					{
						if (!m_coroutineIsRunning)
						{
							qqbot::Network::sendGroupMessage(groupID, "连接RCON服务器协程不在运作，可能是：\n1. 没有连接到RCON服务器\n2. RCON服务器密码错误");
							return;
						}

						//定时器指令
						std::string taskName;

						for (auto i = Args.begin() + 1; i != Args.end(); i++)
						{
							if (i + 1 != Args.end())
								taskName += *i + ' ';
							else
								taskName += *i;
						}

						std::unique_lock<std::mutex> lock(m_mutex);
						m_queue.push({ groupID, taskName });
						m_cv.notify_all();
					}
				},
				"rcon command [command]",
				"发送指令"
				);

			qqbot::ServerInfo::getPermission().setGroupDefaultPermission("rcon", false);
		}

	protected:
		//RCON数据包
		struct RCONPackage
		{
			int		length;
			int		requestID;
			int		type;
			char	data[2];
		};

		template<typename Ty>
		requires std::integral<Ty>
		class Package
		{
		public:
			Package() = default;
			~Package() = default;

			Package(const Package&) = delete;
			Package(Package&&) = delete;

			Package& operator =(const Package&) = delete;
			Package& operator =(Package&&) = delete;

			void write(std::string_view data)
			{
				m_buffer += data;
			}

			bool canRead() const
			{
				if (m_buffer.size() < sizeof(Ty))
					return false;

				Ty length = 0;
				memcpy_s(&length, sizeof(Ty), m_buffer.c_str(), sizeof(Ty));
				if (length + sizeof(Ty) > m_buffer.size())
					return false;

				return true;
			}

			Ty firstMsgLength()
			{
				Ty length = 0;
				memcpy_s(&length, sizeof(Ty), m_buffer.c_str(), sizeof(Ty));
				return length;
			}

			std::string read()
			{
				if (!canRead())
					throw std::logic_error("Can't read data");

				std::string result = m_buffer.substr(0, firstMsgLength() + sizeof(Ty));
				m_buffer = m_buffer.substr(firstMsgLength() + sizeof(Ty));

				return result;
			}

			static std::string makePackage(std::string_view data)
			{
				Ty lenght = static_cast<Ty>(data.size());
				std::string result;
				result.resize(sizeof(Ty));
				memcpy_s(result.data(), sizeof(Ty), result, sizeof(Ty));
				result += data;

				return result;
			}

		private:
			std::string m_buffer;
		};

		//生成RCONPack
		std::shared_ptr<RCONPackage> makePackage(const std::string& data, int type, int requestID, size_t& size)
		{
			size_t packSize = sizeof(int) * 3ll + data.size() + 2ll;

			std::shared_ptr<RCONPackage> localPack = std::shared_ptr<RCONPackage>(
				reinterpret_cast<RCONPackage*>(
					new char[packSize] {0}));

			localPack->type = type;
			localPack->requestID = requestID;
			memcpy_s(localPack->data, data.size(), data.c_str(), data.size());
			localPack->length = static_cast<int>(packSize - sizeof(int));

			size = packSize;
			return localPack;
		}

		//RCONPack转换为二进制数据
		std::string packToString(std::shared_ptr<RCONPackage> packptr, size_t size)
		{
			std::string local(reinterpret_cast<char*>(packptr.get()), size);
			return local;
		}

		//二进制数据转换为RCONPack
		std::shared_ptr<RCONPackage> stringToPack(const std::string& data)
		{
			if (data.size() < sizeof(RCONPackage) - 2)
				throw THROW_ERROR("Data is too small.");

			std::shared_ptr<RCONPackage> localPack = std::shared_ptr<RCONPackage>(
				reinterpret_cast<RCONPackage*>(
					new char[data.size()] {0}));

			memcpy_s(localPack.get(), data.size(), data.c_str(), data.size());

			if (localPack->length + sizeof(int) > data.size())
				throw THROW_ERROR("Data is too small.");

			return localPack;
		}

		//异步获取命令和对应的群聊
		template<asio::completion_token_for<void(std::pair<long long, std::string>)> CompletionToken>
		auto async_get_command(CompletionToken&& token)
		{
			auto func = [&]() -> std::pair<long long, std::string> {
				std::unique_lock<std::mutex> lock(m_mutex);

				for (;;)
				{
					m_cv.wait(lock, [&]() {return !m_queue.empty(); });

					if (!m_queue.empty())
					{
						long long groupID = m_queue.front().first;
						std::string command = m_queue.front().second;
						m_queue.pop();
						return { groupID, command };
					}
				}
			};

			return asio::async_initiate<CompletionToken, void(std::pair<long long, std::string>)>([func]
			(asio::completion_handler_for<void(std::pair<long long, std::string>)> auto handler) {
					// 放置一个空的追踪任务, 防止executor以为当前没有任务而退出
					auto work = asio::make_work_guard(handler);
					std::thread([func, handler = std::move(handler), work = std::move(work)]() mutable {
						asio::dispatch(work.get_executor(), [func, handler = std::move(handler)]() mutable {
							// 结束追踪任务 返回回调结果
							std::move(handler)(func());
							});
						}).detach();
				}, token);
		}

		void connect()
		{
			asio::co_spawn(m_io_context, [this]() -> asio::awaitable<void> {

				//将运行设置为true
				m_coroutineIsRunning = true;

				using namespace asio;

				auto executor = co_await this_coro::executor;

				ip::tcp::endpoint endpoint(ip::make_address(m_endPoint.host), m_endPoint.port);
				

				long long groupID = 0;
				std::string command;

				while (m_coroutineIsRunning)
				{
					ip::tcp::socket socket(m_io_context);

					try
					{
						co_await socket.async_connect(endpoint, use_awaitable);

						//登录
						{
							size_t packSize = 0;
							auto pack = makePackage(m_endPoint.password, 3, m_requestID, packSize);

							co_await async_write(socket, buffer(packToString(pack, packSize)), use_awaitable);
						}

						//获取是否登录成功
						{
							char dataBuffer[1024]{ 0 };

							size_t n = co_await socket.async_read_some(buffer(dataBuffer), use_awaitable);

							if (!n)
								throw THROW_ERROR("无法接收数据");

							auto pack = stringToPack(std::string(dataBuffer, n));

							if (pack->requestID != m_requestID)
								throw THROW_ERROR("RCON密码错误");
						}

						//发送命令
						while (m_coroutineIsRunning)
						{
							std::pair<long long, std::string> get = co_await async_get_command(use_awaitable);
							groupID = get.first;
							command = get.second;

							size_t packSize = 0;
							auto pack = makePackage(command, 2, m_requestID, packSize);

							co_await async_write(socket, buffer(packToString(pack, packSize)), use_awaitable);

							//返回数据

							Package<int> package;
							do
							{
								char dataBuffer[8192]{ 0 };
								size_t n = co_await socket.async_read_some(buffer(dataBuffer), use_awaitable);
								package.write(std::string(dataBuffer, n));
							} while (!package.canRead());

							std::string result;
							int n = package.firstMsgLength();
							auto repack = stringToPack(package.read());
							std::string data(repack->data, n - sizeof(int) * 2 - 2);

							if (data.empty())
							{
								qqbot::Network::sendGroupMessage(groupID, "true");
								continue;
							}

							for (char& i : data)
							{
								if (i == '/')
								{
									result += "\n/";
								}
								else
								{
									result += i;
								}
							}

							qqbot::Network::sendGroupMessage(groupID, result);
						}
					}
					catch (const std::exception& e)
					{
						qqbot::Network::sendGroupMessage(groupID, std::format("无法发送指令：{}", e.what()));
						std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
					}
				}
				co_return;
				},
				asio::detached);

			std::thread([&]() {m_io_context.run(); }).detach();
		}

		void disconnect()
		{
			m_coroutineIsRunning = false;
			m_io_context.stop();
		}

	private:
		asio::io_context		m_io_context;
		EndPoint				m_endPoint;
		int						m_requestID;
		std::mt19937			m_mt;
		std::atomic<bool>		m_coroutineIsRunning;

		std::mutex				m_mutex;
		std::queue<std::pair<long long, std::string>> m_queue;
		std::condition_variable m_cv;
	};
}
