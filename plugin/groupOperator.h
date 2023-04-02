#pragma once

#include <httplib.h>
#include <Json.h>
#include <vector>
#include <string>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"

namespace GroupOperator
{
	class GroupOperatorPlugin : public qqbot::CppPlugin
	{
	public:
		GroupOperatorPlugin()
		{
			qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
			qqbot::CppPlugin::pluginInfo.name = "GroupOperator群组管理插件";
			qqbot::CppPlugin::pluginInfo.version = "0.0.1";
		}

		~GroupOperatorPlugin() = default;

		virtual void onEnable()
		{
			qqbot::ServerInfo::getCommander().addCommand("kick",
				[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
				{
					if (Args.empty())
					{
						qqbot::Network::sendGroupMessage(groupID, "kick userid true/false(可不填) -踢出群成员");
					}
					else if (Args.size() == 1)
					{
						long long userID = 0;
						{
							qjson::JObject jo;
							try
							{
								if (Args[0].substr(0, 10) == "[CQ:at,qq=")
								{
									jo = qjson::JParser::fastParse(Args[0].substr(10, Args[0].size() - 10 - 1));
									userID = jo.getInt();
								}
								else
								{
									jo = qjson::JParser::fastParse(Args[0]);
									userID = jo.getInt();
								}
							}
							catch (const std::exception&)
							{
								qqbot::Network::sendGroupMessage(groupID, "userid无效");
								return;
							}
						}
						
						groupGet("set_group_kick", { {"group_id", std::to_string(groupID)}, {"user_id", std::to_string(userID)}, {"reject_add_request", "false"}});

					}
					else if (Args.size() == 2)
					{
						long long userID = 0;
						{
							qjson::JObject jo;
							try
							{
								if (Args[0].substr(0, 10) == "[CQ:at,qq=")
								{
									jo = qjson::JParser::fastParse(Args[0].substr(10, Args[0].size() - 10 - 1));
									userID = jo.getInt();
								}
								else
								{
									jo = qjson::JParser::fastParse(Args[0]);
									userID = jo.getInt();
								}
							}
							catch (const std::exception&)
							{
								qqbot::Network::sendGroupMessage(groupID, "userid无效");
								return;
							}
						}

						bool refuse = false;
						{
							qjson::JObject jo;
							try
							{
								jo = qjson::JParser::fastParse(Args[1]);
								refuse = jo.getBool();
							}
							catch (const std::exception&)
							{
								qqbot::Network::sendGroupMessage(groupID, "bool值无效");
								return;
							}
						}

						if (refuse)
						{
							groupGet("set_group_kick", { {"group_id", std::to_string(groupID)}, {"user_id", std::to_string(userID)}, {"reject_add_request", "true"} });
						}
						else
						{
							groupGet("set_group_kick", { {"group_id", std::to_string(groupID)}, {"user_id", std::to_string(userID)}, {"reject_add_request", "false"} });
						}

						qqbot::Network::sendGroupMessage(groupID, "踢出成功");
					}
					else
					{
						qqbot::Network::sendGroupMessage(groupID, "参数错误");
					}
				},
				"kick userid true/false",
				"踢出群成员"
					);

			qqbot::ServerInfo::getCommander().addCommand("mute",
				[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
				{
					if (Args.empty())
					{
						qqbot::Network::sendGroupMessage(groupID, "mute userid second -禁言群成员");
					}
					else if (Args.size() == 2)
					{
						long long userID = 0;
						{
							qjson::JObject jo;
							try
							{
								if (Args[0].substr(0, 10) == "[CQ:at,qq=")
								{
									jo = qjson::JParser::fastParse(Args[0].substr(10, Args[0].size() - 10 - 1));
									userID = jo.getInt();
								}
								else
								{
									jo = qjson::JParser::fastParse(Args[0]);
									userID = jo.getInt();
								}
							}
							catch (const std::exception&)
							{
								qqbot::Network::sendGroupMessage(groupID, "userid无效");
								return;
							}
						}

						long long duration = 0;
						{
							qjson::JObject jo;
							try
							{
								jo = qjson::JParser::fastParse(Args[1]);
								duration = jo.getInt();
							}
							catch (const std::exception&)
							{
								qqbot::Network::sendGroupMessage(groupID, "持续时间无效");
								return;
							}
						}

						groupGet("set_group_ban", { {"group_id", std::to_string(groupID)}, {"user_id", std::to_string(userID)},{"duration", std::to_string(duration)} });

						qqbot::Network::sendGroupMessage(groupID, "禁言成功");
					}
					else
					{
						qqbot::Network::sendGroupMessage(groupID, "参数错误");
					}
				},
				"mute userid second",
					"禁言群成员"
					);

			//设置权限
			qqbot::ServerInfo::getPermission().setGroupDefaultPermission("kick", false);
			qqbot::ServerInfo::getPermission().setGroupDefaultPermission("mute", false);
		}

	private:
		httplib::Result groupGet(const std::string& command, std::initializer_list<std::pair<std::string, std::string>> list)
		{
			static httplib::Client cli(std::format("http://{}:{}", qqbot::ServerInfo::getPermission().m_gocq_ip, qqbot::ServerInfo::getPermission().m_gocq_port));
			httplib::Params params;
			
			for (auto i = list.begin(); i != list.end(); i++)
			{
				params.insert(*i);
			}

			return cli.Get("/" + command, params, httplib::Headers());
		}
	};
}