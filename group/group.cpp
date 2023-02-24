#include "group.h"

#include <string>
#include <Json.h>

#include "permission.h"
#include "network.h"
#include "command.h"

extern qqbot::Permission	permission;
extern qqbot::Command		command;

namespace qqbot
{
	httplib::Response groupMethod(const httplib::Request& req)
	{
		//发送消息函数

		//获取到的json消息
		auto getjson = qjson::JParser::fastParse(req.body);

		//消息发送者
		int senderUID = static_cast<int>(getjson["sender"]["user_id"].getInt());

		//消息发送的群
		int groupID = static_cast<int>(getjson["group_id"].getInt());

		//发送的消息
		std::string message = getjson["message"].getString();

		if (message.size() <= 1ll)
			return {};

		auto split = [](const std::string& data) {
			std::vector<std::string> dataList;

			long long begin = -1;
			long long i = 0;

			for (; i < data.size(); i++)
			{
				if (data[i] == ' ')
				{
					dataList.push_back(data.substr(begin + 1, i - begin - 1));
					begin = i;
				}
			}
			dataList.push_back(data.substr(begin + 1, i - begin - 1));

			return dataList;
		};

		if (groupID == 647026133)
		{
			if (message.size() > 1 && message[0] == '!')
			{
				std::vector<std::string> parseString = split(message.substr(1));

				if (parseString.empty())
				{
					Network::sendGroupMessage(groupID, "输入非法");
					return {};
				}

				std::string commandName = parseString[0];
				parseString.erase(parseString.begin());

				try
				{
					command.groupExcute(groupID, senderUID, commandName, std::move(parseString));
				}
				catch (const std::exception& e)
				{
					Network::sendGroupMessage(groupID, e.what());
				}
			}
			else if (!message.empty() && message[0] == '!')
			{
				Network::sendGroupMessage(groupID, "输入非法");
			}
		}

		return {};
	}
}