#include "group.h"

#include <string>
#include "Json.h"

#include "permission.h"

extern qqbot::Permission permission;

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

		if (senderUID == 2098332747 && groupID == 647026133)
		{
			sendmsg(groupID, senderUID, "你好！");
		}

		return {};
	}
}