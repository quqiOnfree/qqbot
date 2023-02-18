#include "group.h"

#include <string>
#include "Json.h"

namespace qqbot
{
	httplib::Response groupMethod(const httplib::Request& req)
	{
		auto getjson = qjson::JParser::fastParse(req.body);
		int senderUID = getjson["sender"]["user_id"];
		int groupID = getjson["group_id"];
		httplib::Client cli("http://127.0.0.1:5700");

		if (senderUID == 2098332747 && groupID == 1141652008)
		{
			httplib::Params params;
			httplib::Headers headers;
			params.insert({ {"group_id", std::to_string(groupID)}, {"user_id", std::to_string(senderUID)}, {"message", "Hello!"} });
			cli.Get("/send_group_msg", params, headers);
		}

		return {};
	}
}