#include "command.h"

#include <format>

#include "network.h"
#include "definition.h"

namespace qqbot
{
	Command::Command()
	{

	}

	Command::~Command()
	{

	}

	void Command::init(qqbot::Permission* permission)
	{
		m_permission = permission;
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
			throw THROW_ERROR("could not find this command");
		}

		if (m_permission->hasUserOperator(senderID))
		{
			m_GroupHandlers[command](groupID, senderID, command, args);
		}
	}

	void Command::userExcute(int senderID,
		const std::string& command,
		std::vector<std::string> args)
	{

	}
}