#pragma once

#include <Json.h>
#include <httplib.h>
#include <vector>
#include <string>
#include <format>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"

namespace MIL
{
	class MILPlugin : public qqbot::CppPlugin
	{
	public:
		MILPlugin()
		{
			qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
			qqbot::CppPlugin::pluginInfo.name = "MIL";
			qqbot::CppPlugin::pluginInfo.version = "0.0.1";
		}

		virtual ~MILPlugin() = default;

		virtual void onEnable()
		{
			qqbot::ServerInfo::getCommander().addCommand("mil",
				[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
				{
					if (Args.empty())
					{
						qqbot::Network::sendGroupMessage(groupID, "mil [美军(a)/苏军(c)] 数值 -人间地狱大炮测算");
					}
					else if (Args.size() == 2)
					{
						long long number = 0;
						{
							qjson::JObject jo;
							try
							{
								jo = qjson::JParser::fastParse(Args[1]);
							}
							catch (...)
							{
								qqbot::Network::sendGroupMessage(groupID, "参数错误，数值要为数字");
								return;
							}

							if (jo.getType() != qjson::JValueType::JInt)
							{
								qqbot::Network::sendGroupMessage(groupID, "参数错误，数值要为数字");
								return;
							}

							number = jo.getInt();
						}

						if (Args[0] == "a")
						{
							qqbot::Network::sendGroupMessage(groupID, std::format("{}对应美军密位：{}", Args[1], (622.0 + 0.237333333 * (1600.0 - number))));
						}
						else if (Args[0] == "c")
						{
							qqbot::Network::sendGroupMessage(groupID, std::format("{}对应苏军密位：{}", Args[1], (800.0 + 0.213333333 * (1600.0 - number))));
						}
						else
						{
							qqbot::Network::sendGroupMessage(groupID, "参数错误，美军(a)/苏军(c)");
						}
					}
					else
					{
						qqbot::Network::sendGroupMessage(groupID, "参数错误");
					}
				},
				"mil [美军(a)/苏军(c)] 数值",
				"人间地狱大炮测算"
					);

			qqbot::ServerInfo::getPermission().setGroupDefaultPermission("mil", true);
		}
	};
}
