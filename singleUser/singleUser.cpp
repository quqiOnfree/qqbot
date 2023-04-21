#include "singleUser.h"

#include <Json.h>

#include "permission.h"
#include "network.h"
#include "command.h"

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
				Network::sendUserMessage(senderUID, "输入非法");
				return {};
			}

			//指令名称
			std::string commandName = parseString[0];
			parseString.erase(parseString.begin());

			try
			{
				//私聊消息处理
				command.userExcute(senderUID, commandName, std::move(parseString));
			}
			catch (const std::exception& e)
			{
				Network::sendUserMessage(senderUID, e.what());
			}
		}
		else if (!message.empty() && message[0] == '!')
		{
			Network::sendUserMessage(senderUID, "输入非法");
		}

		return {};
	}
}
