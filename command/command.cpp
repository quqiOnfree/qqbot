#include "command.h"

#include <format>

#include "network.h"
#include "definition.h"

namespace qqbot
{
	Command::~Command()
	{

	}

	Command::Command(qqbot::Permission* permission)
	{
		//初始化
		m_permission = permission;

		//添加permission的函数,因为permission没有自定义的函数 :(
		this->addCommand("permission", [this, &permission](int groupID, int senderID, const std::string& commandName, std::vector<std::string> Args) {
			
			if (!permission->hasUserOperator(senderID))
			{
				Network::sendGroupMessage(groupID, "你没有权限");
			}
			else if (Args.empty())
			{
				Network::sendGroupMessage(groupID, "set	group/user id permissionName true/false -设置");
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
							permission->setSingleGroupDefaultPermission(static_cast<int>(num.getInt()), Args[3], true);
							Network::sendGroupMessage(groupID, "成功设置");
						}
						else if (num.getType() == qjson::JValueType::JInt && Args[4] == "false")
						{
							permission->setSingleGroupDefaultPermission(static_cast<int>(num.getInt()), Args[3], false);
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
		m_GroupHandlers[commandName] = handler;
	}

	void Command::addCommand(const std::string& commandName, Command::UserHandler handler)
	{
		m_UserHandlers[commandName] = handler;
	}

	void Command::groupExcute(int groupID,
		int senderID,
		const std::string& command,
		std::vector<std::string> args)
	{
		if (m_GroupHandlers.find(command) == m_GroupHandlers.end())
		{
			throw std::exception("could not find this command");
		}

		if (m_permission->hasUserOperator(senderID))
		{
			m_GroupHandlers[command](groupID, senderID, command, args);
			return;
		}
		else if (m_permission->hasSingleGroupDefaultPermission(groupID, command))
		{
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

	void Command::userExcute(int senderID,
		const std::string& command,
		std::vector<std::string> args)
	{
		if (m_GroupHandlers.find(command) == m_GroupHandlers.end())
		{
			throw std::exception("could not find this command");
		}

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