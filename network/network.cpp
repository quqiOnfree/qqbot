#include "network.h"

#include <format>

#include "permission.h"

extern qqbot::Permission permission;

namespace qqbot
{
	bool Network::testConnection()
	{
		static httplib::Client cli(std::format("http://{}:{}", permission.m_gocq_ip, permission.m_gocq_port));
		if (cli.Get("/").error() != httplib::Error::Success)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	void Network::sendGroupMessage(long long groupID, const std::string& message)
	{
		static httplib::Client cli(std::format("http://{}:{}", permission.m_gocq_ip, permission.m_gocq_port));
		httplib::Params params;
		httplib::Headers headers;
		params.insert({ {"group_id", std::to_string(groupID)}, {"message", message} });

		cli.Get("/send_group_msg", params, headers);
	};

	void Network::sendUserMessage(long long senderUID, const std::string& message)
	{
		static httplib::Client cli(std::format("http://{}:{}", permission.m_gocq_ip, permission.m_gocq_port));
		httplib::Params params;
		httplib::Headers headers;
		params.insert({ {"user_id", std::to_string(senderUID)}, {"message", message} });

		cli.Get("/send_private_msg", params, headers);
	};
}
