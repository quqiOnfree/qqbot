#include "network.h"

#include <format>
#include <iostream>

#include "permission.h"

extern qqbot::Permission permission;

namespace qqbot
{
    bool Network::testConnection()
    {
        httplib::Client cli(std::format("http://{}:{}", permission.m_gocq_ip, permission.m_gocq_port));
        return cli.Get("/").error() == httplib::Error::Success;
    }

    void Network::sendGroupMessage(long long groupID, const std::string& message)
    {
        httplib::Client cli(std::format("http://{}:{}", permission.m_gocq_ip, permission.m_gocq_port));
        httplib::Params params;
        httplib::Headers headers;
        params.insert({ {"group_id", std::to_string(groupID)}, {"message", message} });

        std::cout << std::format("[sender]向群{}发送消息：{}\n", groupID, message);

        cli.Get("/send_group_msg", params, headers);
    };

    void Network::sendUserMessage(long long senderUID, const std::string& message)
    {
        httplib::Client cli(std::format("http://{}:{}", permission.m_gocq_ip, permission.m_gocq_port));
        httplib::Params params;
        httplib::Headers headers;
        params.insert({ {"user_id", std::to_string(senderUID)}, {"message", message} });

        std::cout << std::format("[sender]向用户{}发送消息：{}\n", senderUID, message);

        cli.Get("/send_private_msg", params, headers);
    };
}
