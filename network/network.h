#pragma once

#include <httplib.h>
#include <string>

namespace qqbot
{
	class Network
	{
	public:
		Network() = default;
		~Network() = default;

		//测试是否能够连接
		static bool testConnection();

		//群组发送消息
		//args: groupID: 发送消息给qq群号, message: 需要发送的消息
		static void sendGroupMessage(long long groupID, const std::string& message);

		//私聊发送消息
		//args: senderID: 发送消息给qq号, message: 需要发送的消息
		static void sendUserMessage(long long senderUID, const std::string& message);
	};
}
