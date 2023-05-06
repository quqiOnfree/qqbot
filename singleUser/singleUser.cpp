#include "singleUser.h"

#include <Json.h>

#include "permission.h"
#include "network.h"
#include "command.h"
#include "definition.h"

extern qqbot::Permission	permission;
extern qqbot::Command		command;

namespace qqbot
{
	httplib::Response privateMethod(const httplib::Request& req)
	{
		//发送消息函数

		//获取到的json消息
		auto getjson = qjson::JParser::fastParse(req.body);

		//消息发送者
		long long senderUID = getjson["sender"]["user_id"].getInt();

		//发送的消息
		std::string message = getjson["message"].getString();

		if (message.size() <= 1ll)
			return {};
		else if (message[0] != '!')
			return {};

		std::string commandName;
		std::vector<std::string> parseString;

		try
		{
			//函数分割
			Command::splitCommand(message, commandName, parseString);

			//群消息处理
			command.userExcute(senderUID, commandName, std::move(parseString));
		}
		catch (const std::logic_error& e)
		{
			Network::sendUserMessage(senderUID, e.what());
			std::cout << Error::outErrorMessage(e) << '\n';
		}

		return {};
	}
}
