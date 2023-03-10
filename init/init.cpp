#include "init.h"

#include <Json.h>
#include <asio.hpp>

#include "network.h"
#include "singleUser.h"
#include "group.h"

namespace qqbot
{
	void splitUserNGroup(const httplib::Request& req, httplib::Response& res)
	{
		//从go-cqhttp获取到的json消息
		auto getjson = qjson::JParser::fastParse(req.body);

		if (getjson["message_type"] == "private")
		{
			//当消息类型是私聊类型
			res = qqbot::privateMethod(req);
		}
		else if (getjson["message_type"] == "group")
		{
			//当消息类型是群聊类型
			res = qqbot::groupMethod(req);
		}
	}

	int init(httplib::Server& server, qqbot::Permission& permission, qqbot::Register& pluginRegister)
	{
		system("chcp 65001");

		//权限组导入
		permission.init();
		std::cout << "权限组导入成功！\n";

		//测试是否能够连接go-cqhttp
		if (!Network::testConnection())
		{
			std::cout << "无法连接至go-cqhttp(" << permission.m_gocq_ip << ':' << permission.m_gocq_port << ")\n";
			return -1;
		}
		std::cout << "成功连接至go-cqhttp(" << permission.m_gocq_ip << ':' << permission.m_gocq_port << ")\n";

		//插件注册
		pluginRegister.init();
		pluginRegister.run();
		std::cout << "全部插件已经成功加载！\n";

		//http服务器开启
		std::cout << "开启服务器监听(" << permission.m_server_ip << ':' << permission.m_server_port << ")\n";
		server.Post("/", splitUserNGroup);
		server.listen(permission.m_server_ip, permission.m_server_port);
		return -1;
	}
}
