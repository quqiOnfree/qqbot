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
		long long senderUID = getjson["sender"]["user_id"].getInt();

		//消息发送的群
		long long groupID = getjson["group_id"].getInt();

		//发送的消息
		std::string message = getjson["message"].getString();

		if (message.size() <= 1ll)
			return {};

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
				Network::sendGroupMessage(groupID, "输入非法");
				return {};
			}

			//指令名称
			std::string commandName = parseString[0];
			parseString.erase(parseString.begin());

			try
			{
				//群消息处理
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

		return {};
	}
}