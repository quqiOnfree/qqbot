#include "command.h"

#include <format>
#include <Json.h>

#include "network.h"
#include "definition.h"
#include "register.h"
#include "timer.h"
#include "definition.h"

namespace qqbot
{
	Command::~Command()
	{
		m_permission = nullptr;
	}

	Command::Command(qqbot::Permission* permission, qqbot::Register* reg)
	{
		//初始化
		m_permission = permission;
		m_register = reg;

		//添加permission的函数,因为permission没有自定义的函数 :(
		this->addCommand("permission",
			[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
			{
			if (!m_permission->hasUserOperator(senderID))
			{
				Network::sendGroupMessage(groupID, "你没有权限");
			}
			else if (Args.empty())
			{
				Network::sendGroupMessage(groupID, "permission set group/user id permissionName true/false -设置");
			}
			else if (Args.size() == 5)
			{
				if (Args[0] == "set")
				{
					if (Args[1] == "group")
					{
						auto num = qjson::JParser::fastParse(Args[2]);
						if (num.getType() == qjson::JValueType::JInt && Args[4] == "true")
						{
							m_permission->setSingleGroupDefaultPermission(num.getInt(), Args[3], true);
							Network::sendGroupMessage(groupID, "成功设置");
						}
						else if (num.getType() == qjson::JValueType::JInt && Args[4] == "false")
						{
							m_permission->setSingleGroupDefaultPermission(num.getInt(), Args[3], false);
							Network::sendGroupMessage(groupID, "成功设置");
						}
						else
						{
							Network::sendGroupMessage(groupID, "指令参数错误");
						}
					}
					else if (Args[1] == "user")
					{
						Network::sendGroupMessage(groupID, "暂时没有user设置");
					}
					else
					{
						Network::sendGroupMessage(groupID, "没有符合的指令");
					}
				}
				else
				{
					Network::sendGroupMessage(groupID, "没有符合的指令");
				}
			}
			else
			{
				Network::sendGroupMessage(groupID, "没有符合的指令");
			}

			},
			"permission set group/user id permissionName true/false",
			"设置"
			);

		//添加operator的函数
		this->addCommand("op",
			[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
			{
				if (Args.empty())
				{
					Network::sendGroupMessage(groupID, "op userid -添加管理员");
				}
				else if (Args.size() == 1)
				{
					long long userID = 0;
					{
						qjson::JObject jo;
						try
						{
							if (Args[0].substr(0, 10) == "[CQ:at,qq=")
							{
								jo = qjson::JParser::fastParse(Args[0].substr(10, Args[0].size() - 10 - 1));
								userID = jo.getInt();
							}
							else
							{
								jo = qjson::JParser::fastParse(Args[0]);
								userID = jo.getInt();
							}
						}
						catch (const std::exception&)
						{
							qqbot::Network::sendGroupMessage(groupID, "userid无效");
							return;
						}
					}
					m_permission->setUserOperator(userID, true);
					Network::sendGroupMessage(groupID, "设置成功");
				}
				else
				{
					Network::sendGroupMessage(groupID, "参数错误");
				}
			},
			"op userid",
			"添加管理员"
			);

		//删除operator的函数
		this->addCommand("deop",
			[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
			{
				if (Args.empty())
				{
					Network::sendGroupMessage(groupID, "deop userid -删除管理员");
				}
				else if (Args.size() == 1)
				{
					long long userID = 0;
					{
						qjson::JObject jo;
						try
						{
							if (Args[0].substr(0, 10) == "[CQ:at,qq=")
							{
								jo = qjson::JParser::fastParse(Args[0].substr(10, Args[0].size() - 10 - 1));
								userID = jo.getInt();
							}
							else
							{
								jo = qjson::JParser::fastParse(Args[0]);
								userID = jo.getInt();
							}
						}
						catch (const std::exception&)
						{
							qqbot::Network::sendGroupMessage(groupID, "userid无效");
							return;
						}
					}
					m_permission->setUserOperator(userID, false);
					Network::sendGroupMessage(groupID, "设置成功");
				}
				else
				{
					Network::sendGroupMessage(groupID, "参数错误");
				}
			},
			"deop userid",
			"删除管理员"
			);

		this->addCommand("plugin",
			[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
			{
				if (Args.empty())
				{
					qqbot::Network::sendGroupMessage(groupID,
						"plugin list -插件列表\n"
						"plugin reload [pluginName] -重载插件\n"
					);
				}
				else if (Args.size() == 1)
				{
					if (Args[0] == "list")
					{
						//输出数据
						std::string outString;
						//读取锁
						std::shared_lock<std::shared_mutex> lock(m_shareMutex);
						const auto& plugins = m_register->getPlugins();

						for (auto const& [pluginName, plugin] : plugins)
						{
							outString += std::format(
								"-----------------------\n"
								"[{}]\n"
								"Author: {}\n"
								"Version: {}\n",
								pluginName, plugin->pluginInfo.author,
								plugin->pluginInfo.version);
						}

						qqbot::Network::sendGroupMessage(groupID, outString);
					}
					else
					{
						qqbot::Network::sendGroupMessage(groupID, "参数错误");
						return;
					}
				}
				else if (Args.size() == 2)
				{
					if (Args[0] == "reload")
					{
						if (m_register->getPlugins().find(Args[1]) == m_register->getPlugins().end())
						{
							qqbot::Network::sendGroupMessage(groupID, "找不到此插件");
							return;
						}
						
						//互斥锁
						std::lock_guard<std::shared_mutex> lock(m_shareMutex);

						m_register->getPlugins().find(Args[1])->second->onDisable();
						m_register->getPlugins().find(Args[1])->second->onEnable();
					}
					else
					{
						qqbot::Network::sendGroupMessage(groupID, "参数错误");
						return;
					}
				}
				else
				{
					qqbot::Network::sendGroupMessage(groupID, "参数错误");
					return;
				}
			},
			"plugin list",
				"插件列表"
				);

		//定时器
		this->addCommand("timer",
			[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
			{
				static qqbot::Timers timers;

				if (Args.empty())
				{
					Network::sendGroupMessage(groupID,
						"timer interval(second) command\n"
						"timer list\n"
						"timer remove position(int)\n"
						"-定时器\n"
					);
				}
				else if (Args.size() == 1)
				{
					if (Args[0] == "list")
					{
						const auto& list = timers.getTaskList();

						std::string outString("timer任务列表\n");

						long long position = 0;
						for (const auto& taskName : list)
						{
							outString += std::format("{}: {}\n", position++, taskName);
						}

						Network::sendGroupMessage(groupID, outString);
						return;
					}
					else
					{
						Network::sendGroupMessage(groupID, "没有此指令");
						return;
					}
				}
				else
				{
					auto addTask = [&]() -> int{
						{
							long long interval = 0;
							try
							{
								auto jo = qjson::JParser::fastParse(Args[0]);
								if (jo.getType() != qjson::JValueType::JInt)
								{
									return 1;
								}
								interval = jo.getInt();
							}
							catch (const std::exception&)
							{
								return 1;
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

							std::string commandName;
							std::vector<std::string> commandArgs;

							try
							{
								//检测command是否合法
								Command::splitCommand(taskName, commandName, commandArgs);

								//测试是否能成功
								this->groupExcute(groupID, senderID, commandName, commandArgs);
							}
							catch (const std::exception& e)
							{
								Network::sendGroupMessage(groupID, std::format("指令\"{}\"错误：{}", taskName, e.what()));
								std::cout << ERROR_WITH_STACKTRACE(std::format("指令\"{}\"错误：{}", taskName, e.what())) << '\n';
								return 2;
							}

							if (!timers.addTask(taskName,
								interval * 1000,
								&Command::groupExcute,
								this,
								groupID,
								senderID,
								commandName,
								commandArgs)
								)
							{
								Network::sendGroupMessage(groupID, "已经有此任务在运行");
								std::cout << ERROR_WITH_STACKTRACE("已经有此任务在运行") << '\n';
								return 2;
							}

							Network::sendGroupMessage(groupID, std::format("添加\"{}\"指令到定时器中成功，间隔为{}秒", taskName, interval));
							return 0;
						}
					};

					int isAddTask = addTask();
					if (isAddTask != 1)
					{
						return;
					}

					if (Args[0] == "remove")
					{
						long long position = 0;
						{
							auto jo = qjson::JParser::fastParse(Args[1]);
							if (jo.getType() != qjson::JValueType::JInt)
							{
								Network::sendGroupMessage(groupID, "参数错误");
								return;
							}
							position = jo.getInt();
						}

						try
						{
							timers.removeTask(position);
							Network::sendGroupMessage(groupID, "删除成功");
							return;
						}
						catch (const std::exception& e)
						{
							Network::sendGroupMessage(groupID, e.what());
							std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
							return;
						}
					}
					else
					{
						Network::sendGroupMessage(groupID, "没有此指令或参数错误");
						return;
					}
				}
			},
			"timer interval(second) command\n"
				"timer list\n"
				"timer remove position(int)\n"
				,
			"定时器"
			);

		//添加help函数
		this->addCommand("help",
			[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
			{
				if (Args.empty())
				{
					std::string helpMsg;

					for (auto& [functionName, functionHandler] : m_GroupHandlers)
					{
						bool canUseCommand = false;
						if (m_permission->hasUserOperator(senderID))
						{
							canUseCommand = true;
						}
						else if (m_permission->hasSingleGroupDefaultPermission(groupID, functionName))
						{
							if (m_permission->getSingleGroupDefaultPermission(groupID, functionName))
							{
								canUseCommand = true;
							}
							else
							{
								continue;
							}
						}
						else if (m_permission->hasGroupDefaultPermission(functionName))
						{
							if (m_permission->getGroupDefaultPermission(functionName))
							{
								canUseCommand = true;
							}
							else
							{
								continue;
							}
						}

						if (canUseCommand)
						{
							helpMsg += std::format("[!{}] {}\n", functionName, m_groupCommandDescriptions[functionName]);
						}
					}

					Network::sendGroupMessage(groupID, helpMsg);
				}
				else
				{
					if (m_permission->hasUserOperator(senderID))
					{
						Network::sendGroupMessage(groupID, std::format("[!{}]\n命令格式：!{}\n命令定义：{}", Args[0], m_groupCommandFormats[Args[0]], m_groupCommandDescriptions[Args[0]]));
						return;
					}
					else if (m_permission->hasSingleGroupDefaultPermission(groupID, Args[0]))
					{
						if (m_permission->getSingleGroupDefaultPermission(groupID, Args[0]))
						{
							Network::sendGroupMessage(groupID, m_groupCommandDescriptions[Args[0]]);
							return;
						}
						else
						{
							Network::sendGroupMessage(groupID, "你没有此命令权限");
							return;
						}
					}
					else if (m_permission->hasGroupDefaultPermission(Args[0]))
					{
						if (m_permission->getGroupDefaultPermission(Args[0]))
						{
							Network::sendGroupMessage(groupID, m_groupCommandDescriptions[Args[0]]);
							return;
						}
						else
						{
							Network::sendGroupMessage(groupID, "你没有此命令权限");
							return;
						}
					}
				}
			},
			"help command(可不填)",
			"帮助");
	}

	Command::Command(Command&& command) noexcept
	{
		//只允许移动，不允许复制
		m_permission = command.m_permission;
		command.m_permission = nullptr;
	}

	Command& Command::operator=(Command&& command) noexcept
	{
		//只允许移动，不允许复制
		if (this == &command)
			return *this;

		m_permission = command.m_permission;
		command.m_permission = nullptr;
		return *this;
	}

	void Command::addCommand(const std::string& commandName, Command::GroupHandler handler, const std::string& commandFormat, const std::string& description)
	{
		//互斥锁
		std::lock_guard<std::shared_mutex> lock(m_shareMutex);

		if (m_GroupHandlers.find(commandName) != m_GroupHandlers.end())
		{
			throw THROW_ERROR(std::format("已经存在此指令：{}", commandName));
		}

		//添加群指令
		m_GroupHandlers[commandName] = handler;
		m_groupCommandDescriptions[commandName] = description;
		m_groupCommandFormats[commandName] = commandFormat;
	}

	void Command::addCommand(const std::string& commandName, Command::UserHandler handler, const std::string& commandFormat, const std::string& description)
	{
		//互斥锁
		std::lock_guard<std::shared_mutex> lock(m_shareMutex);

		if (m_UserHandlers.find(commandName) != m_UserHandlers.end())
		{
			throw THROW_ERROR(std::format("已经存在此指令：{}", commandName));
		}

		//添加单人指令
		m_UserHandlers[commandName] = handler;
		m_userCommandDescriptions[commandName] = description;
		m_userCommandFormats[commandName] = commandFormat;
	}

	void Command::removeCommand(const std::string& commandName, bool isGroup)
	{
		//互斥锁
		std::lock_guard<std::shared_mutex> lock(m_shareMutex);

		if (isGroup)
		{
			//删除群聊指令
			if (m_GroupHandlers.find(commandName) == m_GroupHandlers.end())
				throw THROW_ERROR("can't find this command");

			m_GroupHandlers.erase(m_GroupHandlers.find(commandName));
			m_groupCommandFormats.erase(m_groupCommandFormats.find(commandName));
			m_groupCommandDescriptions.erase(m_groupCommandDescriptions.find(commandName));
		}
		else
		{
			//删除私聊指令
			if (m_UserHandlers.find(commandName) == m_UserHandlers.end())
				throw THROW_ERROR("can't find this command");

			m_UserHandlers.erase(m_UserHandlers.find(commandName));
			m_userCommandFormats.erase(m_userCommandFormats.find(commandName));
			m_userCommandDescriptions.erase(m_userCommandDescriptions.find(commandName));
		}
	}

	void Command::groupExcute(long long groupID,
		long long senderID,
		const std::string& command,
		std::vector<std::string> args)
	{
		//读取锁
		std::shared_lock<std::shared_mutex> lock(m_shareMutex);

		//查找指令
		if (m_GroupHandlers.find(command) == m_GroupHandlers.end())
		{
			throw std::exception("could not find this command");
		}

		//是否是operator
		if (m_permission->hasUserOperator(senderID))
		{
			m_GroupHandlers[command](groupID, senderID, command, args);
			return;
		}
		else if (m_permission->hasSingleGroupDefaultPermission(groupID, command))
		{
			//是否在个性化群设置中有这个
			if (m_permission->getSingleGroupDefaultPermission(groupID, command))
			{
				m_GroupHandlers[command](groupID, senderID, command, args);
				return;
			}
			else
			{
				throw std::exception("you don't have permission");
			}
		}
		else if (m_permission->hasGroupDefaultPermission(command))
		{
			//查找默认群权限设置
			if (m_permission->getGroupDefaultPermission(command))
			{
				m_GroupHandlers[command](groupID, senderID, command, args);
				return;
			}
			else
			{
				throw std::exception("you don't have permission");
			}
		}
		else
		{
			throw std::exception("you don't have permission");
		}
	}

	void Command::userExcute(long long senderID,
		const std::string& command,
		std::vector<std::string> args)
	{
		//读取锁
		std::shared_lock<std::shared_mutex> lock(m_shareMutex);

		//查找是否有这个指令
		if (m_UserHandlers.find(command) == m_UserHandlers.end())
		{
			throw std::exception("could not find this command");
		}

		//是否是operator
		if (m_permission->hasUserOperator(senderID))
		{
			m_UserHandlers[command](senderID, command, args);
			return;
		}
		if (m_permission->hasGroupDefaultPermission(command))
		{
			//查找默认群权限设置
			if (m_permission->getGroupDefaultPermission(command))
			{
				m_UserHandlers[command](senderID, command, args);
				return;
			}
			else
			{
				throw std::exception("you don't have permission");
			}
		}
		else
		{
			throw std::exception("you don't have permission");
		}
	}

	void Command::splitCommand(const std::string& command, std::string& commandHead, std::vector<std::string>& args)
	{
		const std::string& message = command;

		if (message.size() <= 1ll)
			throw THROW_ERROR("命令过小");

		//string按空格分割函数
		auto split = [](const std::string& data) {
			std::vector<std::string> dataList;

			long long begin = -1;
			long long i = 0;

			for (; static_cast<size_t>(i) < data.size(); i++)
			{
				if (data[i] == ' ')
				{
					if ((i - begin - 1) > 0)
					{
						dataList.push_back(data.substr(begin + 1, i - begin - 1));
					}
					begin = i;
				}
			}
			dataList.push_back(data.substr(begin + 1, i - begin - 1));

			return dataList;
		};

		//判断消息第一个字符是 '!'
		if (message.size() > 1 && message[0] == '!')
		{
			//分割后的参数
			std::vector<std::string> parseString = split(message.substr(1));

			//如果args为0就报错
			if (parseString.empty())
			{
				throw THROW_ERROR("输入非法");
			}

			//指令名称
			std::string commandName = parseString[0];
			parseString.erase(parseString.begin());

			//成功返回
			commandHead = std::move(commandName);
			args = std::move(parseString);
			return;
		}
		else if (!message.empty() && message[0] != '!')
		{
			throw THROW_ERROR("输入非法");
		}
	}
}
