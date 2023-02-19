#pragma once

#include "httplib.h"
#include <string>

namespace qqbot
{
	class Network
	{
	public:
		Network() = default;
		~Network() = default;

		static void sendGroupMessage(int groupID, int senderUID, const std::string& message);
		static void sendUserMessage(int senderUID, const std::string& message);
	};
}
