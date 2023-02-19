#include "network.h"

namespace qqbot
{
	void Network::sendGroupMessage(int groupID, int senderUID, const std::string& message)
	{
		static httplib::Client cli("http://127.0.0.1:5700");
		httplib::Params params;
		httplib::Headers headers;
		params.insert({ {"group_id", std::to_string(groupID)}, {"user_id", std::to_string(senderUID)}, {"message", message} });

		cli.Get("/send_group_msg", params, headers);
	};

	void Network::sendUserMessage(int senderUID, const std::string& message)
	{
		static httplib::Client cli("http://127.0.0.1:5700");
		httplib::Params params;
		httplib::Headers headers;
		params.insert({ {"user_id", std::to_string(senderUID)}, {"message", message} });

		cli.Get("/send_private_msg", params, headers);
	};
}
