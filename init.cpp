#include "init.h"

#include "Json.h"
#include "include/asio.hpp"

#include "singleUser.h"
#include "group.h"
#include "register.h"

namespace qqbot
{
	void splitUserNGroup(const httplib::Request& req, httplib::Response& res)
	{
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

	int init(httplib::Server& server, qqbot::Permission& permission)
	{
		//插件注册
		qqbot::reg::init();

		//权限组导入
		permission.init();

		server.Post("/", splitUserNGroup);
		server.listen("127.0.0.1", 5800);
		return 0;
	}
}
