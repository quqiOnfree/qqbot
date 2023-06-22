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
#include <unordered_map>
#include <asio.hpp>
#include <Json.h>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"
#include "definition.h"

namespace MCRCON
{
	//RCON数据包
	struct RCONPackage
	{
		int		length;
		int		requestID;
		int		type;
		char	data[2];

		//生成RCONPack
		static std::shared_ptr<RCONPackage> makePackage(const std::string& data, int type, int requestID, size_t& size)
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
		static std::string packToString(std::shared_ptr<RCONPackage> packptr, size_t size)
		{
			std::string local(reinterpret_cast<char*>(packptr.get()), size);
			return local;
		}

		//二进制数据转换为RCONPack
		static std::shared_ptr<RCONPackage> stringToPack(const std::string& data)
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

		//获取RCONPack的数据长度
		static int getDataSize(std::shared_ptr<RCONPackage> pack)
		{
			return pack->length - sizeof(int) - 2;
		}
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

	class MCServerRCONClient
	{
	public:
		struct EndPoint
		{
			std::string		serverName;
			std::string		host;
			unsigned short	port;
			std::string		password;
		};

		MCServerRCONClient() :
			m_coroutineIsRunning(true),
			m_serverSocket(m_io_context),
			m_mt(std::random_device()())
		{
			// 线程运行函数
			auto workfunc = [this]() {
				while (m_coroutineIsRunning)
				{
					{
						// 协程运行锁
						std::unique_lock<std::mutex> lock(m_coroutineMutex);
						// 等待协程启动
						m_coroutineCV.wait(lock, [&]() {return !m_coroutineIsRunning || m_socketIsRunning; });
						if (!m_coroutineIsRunning)
							return;
						else if (!m_socketIsRunning)
							continue;
					}

					// 连接
					try
					{
						m_serverSocket.connect(asio::ip::tcp::endpoint(
							asio::ip::address::from_string(m_endPoint.host),
							m_endPoint.port));

						size_t packSize = 0;
						// 发送包
						auto sendpack = RCONPackage::makePackage(m_endPoint.password, 3, m_mt(), packSize);
						m_serverSocket.send(asio::buffer(RCONPackage::packToString(sendpack, packSize)));

						Package<int> package;
						char dataBuffer[8192]{ 0 };
						do
						{
							size_t n = m_serverSocket.read_some(asio::buffer(dataBuffer));
							package.write(std::string(dataBuffer, n));
						} while (!package.canRead());

						// 接收包
						auto recvpack = RCONPackage::stringToPack(package.read());
						if (recvpack->type != 2)
							throw std::runtime_error("password is invalid");
					}
					catch (...)
					{
						m_socketIsRunning = false;
						m_coroutineCV.notify_all();
						continue;
					}

					m_coroutineIsRunning = true;
					m_socketIsRunning = true;
					m_coroutineCV.notify_all();

					// 接收数据
					char dataBuffer[8192]{ 0 };
					Package<int> package;
					while (m_socketIsRunning)
					{
						try
						{
							// 发送消息
							{
								std::unique_lock<std::mutex> lock(m_queueMutex);
								m_queueCV.wait(lock,
									[&]() {return !m_coroutineIsRunning || !m_socketIsRunning || !m_queue.empty(); });
								if (!m_coroutineIsRunning)
									return;
								else if (!m_socketIsRunning)
									break;

								if (!m_queue.empty())
								{
									auto [requestID, command] = m_queue.front();
									m_queue.pop();
									size_t packSize = 0;
									auto pack = RCONPackage::makePackage(command, 2, requestID, packSize);
									m_serverSocket.send(asio::buffer(RCONPackage::packToString(pack, packSize)));
								}
							}

							// 接收消息
							do
							{
								size_t n = m_serverSocket.read_some(asio::buffer(dataBuffer));
								package.write(std::string(dataBuffer, n));
							} while (!package.canRead());

							{
								while (package.canRead())
								{
									auto pack = RCONPackage::stringToPack(package.read());

									if (pack->requestID == -1)
									{
										std::cout << ERROR_WITH_STACKTRACE("requestID is -1") << '\n';
										continue;
									}

									std::unique_lock<std::mutex> lock(m_mapMutex);
									long long groupId = m_requestIDMap[pack->requestID];

									std::string buffer(pack->data, pack->getDataSize(pack));
									std::string restr;

									for (const auto& i : buffer)
									{
										if (i == '/')
											restr += '\n' + i;
										else
											restr += i;
									}

									qqbot::Network::sendGroupMessage(groupId,
										std::format("[{}]{}", m_endPoint.serverName, restr));
									m_requestIDMap.erase(pack->requestID);
								}
							}
						}
						catch (...)
						{
							m_socketIsRunning = false;
							break;
						}
					}
				}
				};

			// 运行线程
			m_workThread = std::thread(workfunc);
			std::thread([this]() {m_io_context.run(); }).detach();
		}

		MCServerRCONClient(const MCServerRCONClient&) = delete;
		MCServerRCONClient(MCServerRCONClient&&) = delete;

		~MCServerRCONClient()
		{
			stop();
		}

		MCServerRCONClient& operator=(const MCServerRCONClient&) = delete;
		MCServerRCONClient& operator=(MCServerRCONClient&&) = delete;

		void setEndPoint(const EndPoint& ep)
		{
			if (m_socketIsRunning)
				throw std::runtime_error("it can't be set to new EndPoint");
			m_endPoint = ep;
		}

		// 发送command到rcon
		void postRequest(long long groupID, const std::string& command)
		{
			if (!m_socketIsRunning)
				throw std::runtime_error("you can't post the request because socket is closed");

			int requestId = std::abs(static_cast<int>(m_mt())) % 1000000000;
			while (m_requestIDMap.find(requestId) != m_requestIDMap.end())
			{
				requestId = std::abs(static_cast<int>(m_mt())) % 1000000000;
			}

			{
				std::unique_lock<std::mutex> lock(m_mapMutex);
				m_requestIDMap[requestId] = groupID;
			}
			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_queue.push({ requestId ,command });
				m_queueCV.notify_all();
			}
		}

		void run()
		{
			if (m_socketIsRunning)
				throw std::runtime_error("socket is running");

			m_socketIsRunning = true;
			m_coroutineCV.notify_all();
			std::unique_lock<std::mutex> lock(m_coroutineMutex);
			m_coroutineCV.wait(lock);
			if (!m_socketIsRunning)
				throw std::runtime_error("socket can't be opened");
		}

		void stop()
		{
			m_coroutineIsRunning = false;
			m_socketIsRunning = false;
			try
			{
				m_serverSocket.close();
			}
			catch (...) {}

			m_coroutineCV.notify_all();
			m_queueCV.notify_all();
			
			if (m_workThread.joinable())
				m_workThread.join();
		}

		bool isRunning() const
		{
			return static_cast<bool>(m_socketIsRunning);
		}

	private:
		asio::io_context								m_io_context;
		asio::ip::tcp::socket							m_serverSocket;
		EndPoint										m_endPoint;
		std::mt19937									m_mt;

		std::atomic<bool>								m_coroutineIsRunning;
		std::atomic<bool>								m_socketIsRunning;
		std::thread										m_workThread;

		std::mutex										m_coroutineMutex;
		std::condition_variable							m_coroutineCV;

		std::mutex										m_queueMutex;
		std::queue<std::pair<int, std::string>>			m_queue;
		std::condition_variable							m_queueCV;

		std::mutex										m_mapMutex;
		std::unordered_map<int, long long>				m_requestIDMap;
	};

	class MCRCONPlugin : public qqbot::CppPlugin
	{
	public:
		MCRCONPlugin()
		{
			qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
			qqbot::CppPlugin::pluginInfo.name = "mcrcon";
			qqbot::CppPlugin::pluginInfo.version = "0.0.1";

			{
				std::ifstream infile("./plugin_config/mcrcon/config.json");
				if (!infile)
				{
					createConfig();
				}
			}

			try
			{
				std::ifstream infile("./plugin_config/mcrcon/config.json");

				qjson::JObject jo(qjson::JParser::fastParse(infile));
				const qjson::dict_t& dict = jo.getDict();

				//std::lock_guard<std::mutex> lock(m_serverMapMutex);
				for (const auto& [serverName, localjo] : dict)
				{
					m_serverMap[serverName] = std::make_shared<MCServerRCONClient>();
					m_serverMap[serverName]->setEndPoint({
						serverName,
						localjo["host"].getString(),
						static_cast<unsigned short>(localjo["port"].getInt()),
						localjo["password"].getString()});
				}
			}
			catch (const std::exception& e)
			{
				std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
			}

			//m_serverMap["a"] = std::make_shared<MCServerRCONClient>();
			//m_serverMap["a"]->setEndPoint({ "a", "192.168.1.3", 25575, "123456" });
		}

		void createConfig()
		{
			std::filesystem::create_directory("./plugin_config/mcrcon");
			qjson::JObject jo;
			jo["lobby"]["host"] = "192.168.1.3";
			jo["lobby"]["port"] = 25575;
			jo["lobby"]["password"] = "123456";
			std::ofstream outfile("./plugin_config/mcrcon/config.json");
			outfile << qjson::JWriter::fastFormatWrite(jo);
		}

		virtual void onEnable()
		{
			qqbot::ServerInfo::getCommander().addCommand("rcon",
				[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> args) {
					if (args.empty())
					{
						qqbot::Network::sendGroupMessage(groupID,
							"rcon [serverName] command [command]\n"
							"rcon [serverName] connect\n"
							"rcon list\n"
							"rcon reload\n"
						);
						return;
					}

					if (args.size() == 1)
					{
						if (args[0] == "list")
						{
							std::lock_guard<std::mutex> lock(m_serverMapMutex);
							std::string restr;
							for (const auto& [serverName, server] : m_serverMap)
							{
								restr += serverName + ',';
							}
							qqbot::Network::sendGroupMessage(groupID, std::format("RCON list: \n[{}]", restr));
							return;
						}
						else if (args[0] == "reload")
						{
							std::ifstream infile("./plugin_config/mcrcon/config.json");
							if (!infile)
							{
								qqbot::Network::sendGroupMessage(groupID, "无法打开配置文件 (./plugin_config/mcrcon/config.json)");
								return;
							}

							try
							{
								{
									std::unique_lock<std::mutex> lock(m_serverMapMutex);
									while (!m_serverMap.empty())
									{
										m_serverMap.erase(m_serverMap.begin());
									}
								}

								qjson::JObject jo(qjson::JParser::fastParse(infile));
								const qjson::dict_t& dict = jo.getDict();

								std::lock_guard<std::mutex> lock(m_serverMapMutex);
								for (const auto& [serverName, localjo] : dict)
								{
									m_serverMap[serverName] = std::make_shared<MCServerRCONClient>();
									m_serverMap[serverName]->setEndPoint({
										serverName,
										localjo["host"].getString(),
										static_cast<unsigned short>(localjo["port"].getInt()),
										localjo["password"].getString() });
								}

								qqbot::Network::sendGroupMessage(groupID, "successfully reload");
							}
							catch (const std::exception& e)
							{
								std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
								qqbot::Network::sendGroupMessage(groupID, e.what());
							}
						}
						else
						{
							qqbot::Network::sendGroupMessage(groupID, "命令参数错误");
							return;
						}
						return;
					}

					if (m_serverMap.find(args[0]) == m_serverMap.end())
					{
						qqbot::Network::sendGroupMessage(groupID, "invalid");
						return;
					}

					// server名
					std::string& serverName = args[0];
					if (args.size() >= 2)
					{
						if (args[1] == "connect")
						{
							try
							{
								//std::lock_guard<std::mutex> lock(m_serverMapMutex);
								m_serverMap[serverName]->run();
								qqbot::Network::sendGroupMessage(groupID, "open rcon successful!");
								return;
							}
							catch (const std::exception& e)
							{
								std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
								qqbot::Network::sendGroupMessage(groupID, e.what());
								return;
							}
						}
						else if (args[1] == "command")
						{
							std::string command;
							for (size_t i = 2; i < args.size(); i++)
							{
								if (i == args.size() - 1)
									command += args[i];
								else
									command += args[i] + ' ';
							}

							//std::lock_guard<std::mutex> lock(m_serverMapMutex);
							m_serverMap[serverName]->postRequest(groupID, command);
						}
						else
						{
							qqbot::Network::sendGroupMessage(groupID, "没有此指令");
							return;
						}
					}
				},
				"rcon command [command]",
				"发送指令");
		}

		virtual ~MCRCONPlugin() = default;

	private:
		std::mutex																m_serverMapMutex;
		std::unordered_map<std::string, std::shared_ptr<MCServerRCONClient>>	m_serverMap;
	};
}
