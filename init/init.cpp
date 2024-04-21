#include "init.h"

#include <iostream>
#include <format>
#include <ctime>

#include <Json.h>

#include "network.h"
#include "singleUser.h"
#include "group.h"
#include "definition.h"
#include "eventRegister.h"

extern qqbot::EventRegister    eventRegister;

namespace qqbot
{
    void splitUserNGroup(const httplib::Request& req, httplib::Response& res)
    {
        {
            char buffer[50]{ 0 };
            std::time_t t = time(nullptr);
            std::tm* tm = std::localtime(&t);
            std::strftime(buffer, sizeof(buffer), "[%T]", tm);
            std::cout << std::format("{}{}:{}, -- {} --", buffer, req.remote_addr, req.remote_port, req.method) << '\n';
        }

        qjson::JObject getjson;
        try
        {
            // 从go-cqhttp获取到的json消息
            getjson = qjson::JParser::fastParse(req.body);
        }
        catch (const std::exception& e)
        {
            std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
            return;
        }

        // debug
        // std::cout << qjson::JWriter::fastFormatWrite(getjson) << '\n';

        if (getjson["message_type"] == "private")
        {
            // 当消息类型是私聊类型
            res = qqbot::privateMethod(req);
            eventRegister.updateUserReceiveEvent(getjson["user_id"].getInt(),
                getjson["message_id"].getInt(),
                getjson["message"].getString());
        }
        else if (getjson["message_type"] == "group")
        {
            // 当消息类型是群聊类型
            res = qqbot::groupMethod(req);
            eventRegister.updateGroupReceiveEvent(getjson["group_id"].getInt(),
                getjson["user_id"].getInt(),
                getjson["message_id"].getInt(),
                getjson["message"].getString());
        }
    }

    int init(httplib::Server& server, qqbot::Permission& permission, qqbot::Register& pluginRegister)
    {
#ifdef WIN32
        system("chcp 65001");
#endif

        // 权限组导入
        permission.init();
        std::cout << "权限组导入成功！\n";

        // 测试是否能够连接go-cqhttp
        if (!Network::testConnection())
        {
            std::cout << std::format("无法连接至go-cqhttp({}:{})\n", permission.m_gocq_ip, permission.m_gocq_port);
            return -1;
        }
        std::cout << std::format("成功连接至go-cqhttp({}:{})\n", permission.m_gocq_ip, permission.m_gocq_port);

        std::filesystem::create_directory("./plugin_config");

        // 插件注册
        try
        {
            pluginRegister.init();
            pluginRegister.run();
        }
        catch (const std::exception& e)
        {
            std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
            return -1;
        }
        std::cout << "全部插件已经成功加载！\n";

        //http服务器开启
        std::cout << std::format("开启服务器监听({}:{})\n", permission.m_server_ip, permission.m_server_port);
        server.Post("/", splitUserNGroup);
        server.listen(permission.m_server_ip, permission.m_server_port);
        return 0;
    }
}
