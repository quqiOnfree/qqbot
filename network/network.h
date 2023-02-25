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

		static void sendGroupMessage(long long groupID, const std::string& message);
		static void sendUserMessage(long long senderUID, const std::string& message);
	};
}
