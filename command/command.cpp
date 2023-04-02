#include "command.h"

#include <format>

#include "network.h"
#include "definition.h"
#include "register.h"

extern qqbot::Register pluginRegister;

namespace qqbot
{
	Command::~Command()
	{
		m_permission = nullptr;
	}

	Command::Command(qqbot::Permission* permission)
	{
		//初始化
		m_permission = permission;

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
					qqbot::Network::sendGroupMessage(groupID, "plugin list -插件列表");
				}
				else if (Args.size() == 1)
				{
					if (Args[0] == "list")
					{
						std::string outString;

						const std::unordered_map<std::string, std::shared_ptr<CppPlugin>>& plugins = pluginRegister.getPlugins();
						for (auto i = plugins.begin(); i != plugins.end(); i++)
						{
							outString += std::format("-----------------------\n[{}]\nAuthor: {}\nVersion: {}\n", i->first, i->second->pluginInfo.author, i->second->pluginInfo.version);
						}

						qqbot::Network::sendGroupMessage(groupID, outString);
					}
					else
					{
						qqbot::Network::sendGroupMessage(groupID, "参数错误");
					}
				}
			},
			"plugin list",
				"插件列表"
				);

		//添加help函数
		this->addCommand("help",
			[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
			{
				if (Args.empty())
				{
					std::string helpMsg;

					for (auto i = m_GroupHandlers.begin(); i != m_GroupHandlers.end(); i++)
					{
						bool canUseCommand = false;
						if (m_permission->hasUserOperator(senderID))
						{
							canUseCommand = true;
						}
						else if (m_permission->hasSingleGroupDefaultPermission(groupID, i->first))
						{
							if (m_permission->getSingleGroupDefaultPermission(groupID, i->first))
							{
								canUseCommand = true;
							}
							else
							{
								continue;
							}
						}
						else if (m_permission->hasGroupDefaultPermission(i->first))
						{
							if (m_permission->getGroupDefaultPermission(i->first))
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
							helpMsg += std::format("[!{}] {}\n", i->first, m_groupCommandDescriptions[i->first]);
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
		if (m_UserHandlers.find(commandName) != m_UserHandlers.end())
		{
			throw THROW_ERROR(std::format("已经存在此指令：{}", commandName));
		}

		//添加单人指令
		m_UserHandlers[commandName] = handler;
		m_userCommandDescriptions[commandName] = description;
	}

	void Command::groupExcute(long long groupID,
		long long senderID,
		const std::string& command,
		std::vector<std::string> args)
	{
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
		//此函数未完善!!!
		//此函数未完善!!!
		//此函数未完善!!!

		//查找是否有这个指令
		if (m_GroupHandlers.find(command) == m_GroupHandlers.end())
		{
			throw std::exception("could not find this command");
		}

		//是否是operator
		if (m_permission->hasUserOperator(senderID))
		{
			m_UserHandlers[command](senderID, command, args);
			return;
		}
		else
		{
			throw std::exception("you don't have permission");
		}
	}
}
