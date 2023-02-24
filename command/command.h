#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "permission.h"

namespace qqbot
{
	class Command
	{
	public:
		//args: groupID, senderID, commandName, Args
		using GroupHandler = std::function<void(int, int, const std::string&, std::vector<std::string>)>;
		
		//args: senderID, commandName, Args
		using UserHandler = std::function<void(int, const std::string&, std::vector<std::string>)>;

	public:
		Command();
		~Command();

		void init(qqbot::Permission* permission);

		void addCommand(const std::string& commandName, GroupHandler handler);
		void addCommand(const std::string& commandName, UserHandler handler);

		void groupExcute(int groupID,
			int senderID,
			const std::string& command,
			std::vector<std::string> args);

		void userExcute(int senderID,
			const std::string& command,
			std::vector<std::string> args);

	private:
		std::unordered_map<std::string, GroupHandler>	m_GroupHandlers;
		std::unordered_map<std::string, UserHandler>	m_UserHandlers;
		qqbot::Permission*								m_permission;
	};
}
