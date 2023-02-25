#include "init.h"

#include <Json.h>
#include <asio.hpp>

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

		//插件注册
		pluginRegister.init();
		pluginRegister.run();

		//http服务器开启
		server.Post("/", splitUserNGroup);
		server.listen("127.0.0.1", 5800);
		return 0;
	}
}
