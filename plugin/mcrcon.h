#pragma once

#include <vector>
#include <string>
#include <asio.hpp>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"

namespace MCRCON
{
	class MCRCONPlugin : public qqbot::CppPlugin
	{
	public:
		MCRCONPlugin()
		{
			qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
			qqbot::CppPlugin::pluginInfo.name = "rcon";
			qqbot::CppPlugin::pluginInfo.version = "0.0.1";
		}

		~MCRCONPlugin() = default;

		virtual void onEnable()
		{
			qqbot::ServerInfo::getCommander().addCommand("rcon",
				[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
				{
					if (Args.empty())
					{
						qqbot::Network::sendGroupMessage(groupID, "rcon ip port password command -发送指令");
					}
				},
				"rcon ip port password command -发送指令"
				);

			qqbot::ServerInfo::getPermission().setGroupDefaultPermission("rcon", true);
		}
	};
}
