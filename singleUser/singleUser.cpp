#include "singleUser.h"

#include <Json.h>

namespace qqbot
{
	httplib::Response privateMethod(const httplib::Request& req)
	{
		auto getjson = qjson::JParser::fastParse(req.body);
		long long senderUID = getjson["sender"]["user_id"].getInt();
		httplib::Client cli("http://127.0.0.1:5700");

		{
			//使私聊只会发hello
			//以后会改
			httplib::Params params;
			httplib::Headers headers;
			params.insert({ {"user_id", std::to_string(senderUID)}, {"message", "Hello!"} });

			cli.Get("/send_private_msg", params, headers);

		}

		return {};
	}
}
