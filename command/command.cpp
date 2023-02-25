#include "command.h"

#include <format>

#include "network.h"
#include "definition.h"

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

			});

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
					auto num = qjson::JParser::fastParse(Args[0]);
					if (num.getType() == qjson::JValueType::JInt)
					{
						m_permission->setUserOperator(num.getInt(), true);
						Network::sendGroupMessage(groupID, "设置成功");
					}
					else
					{
						Network::sendGroupMessage(groupID, "参数错误");
					}
				}
				else
				{
					Network::sendGroupMessage(groupID, "参数错误");
				}
			});

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
					auto num = qjson::JParser::fastParse(Args[0]);
					if (num.getType() == qjson::JValueType::JInt)
					{
						//m_permission->setUserOperator(num.getInt(), false);
						Network::sendGroupMessage(groupID, "设置成功");
					}
					else
					{
						Network::sendGroupMessage(groupID, "参数错误");
					}
				}
				else
				{
					Network::sendGroupMessage(groupID, "参数错误");
				}
			});
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

	void Command::addCommand(const std::string& commandName, Command::GroupHandler handler)
	{
		//添加群指令
		m_GroupHandlers[commandName] = handler;
	}

	void Command::addCommand(const std::string& commandName, Command::UserHandler handler)
	{
		//添加单人指令
		m_UserHandlers[commandName] = handler;
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