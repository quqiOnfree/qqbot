﻿#include "singleUser.h"

#include <Json.h>

#include "permission.h"
#include "network.h"
#include "command.h"
#include "definition.h"

extern qqbot::Permission    permission;
extern qqbot::Command        command;

namespace qqbot
{
    httplib::Response privateMethod(const httplib::Request& req)
    {
        auto strip = [](const std::string& str) -> std::string {
            if (str.empty())
                return std::string();

            size_t first = 0;
            size_t end = str.size() - 1;
            for (auto i = str.begin(); i != str.end(); i++)
            {
                if (*i == ' ' || *i == '\n' || *i == '\t')
                    first++;
                else
                    break;
            }

            for (auto i = str.end() - 1; i != str.begin(); i--)
            {
                if (*i == ' ' || *i == '\n' || *i == '\t')
                    end--;
                else
                    break;
            }

            if (first < end)
                return std::string(str.begin() + first, str.begin() + end + 1);
            else
                return std::string();
        };

        //发送消息函数

        //获取到的json消息
        auto getjson = qjson::JParser::fastParse(req.body);

        //消息发送者
        long long senderUID = getjson["sender"]["user_id"].getInt();

        //发送的消息
        std::string message = strip(getjson["message"].getString());

        if (message.size() <= 1ll)
            return {};

        if (message[0] != '!')
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
        catch (const std::exception& e)
        {
            Network::sendUserMessage(senderUID, e.what());
            std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
        }

        return {};
    }
}
