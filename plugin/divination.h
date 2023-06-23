#pragma once

#include <vector>
#include <string>
#include <format>
#include <random>
#include <ctime>
#include <mutex>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"

namespace Divination
{
	class DivinationPlugin : public qqbot::CppPlugin
	{
	public:
		DivinationPlugin() : m_mt(std::random_device()())
		{
			qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
			qqbot::CppPlugin::pluginInfo.name = "Divination";
			qqbot::CppPlugin::pluginInfo.version = "0.0.1";
		}

		virtual ~DivinationPlugin() = default;

		virtual void onEnable()
		{
			qqbot::ServerInfo::getCommander().addCommand("div",
				[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
				{
					if (Args.empty())
					{
						qqbot::Network::sendGroupMessage(groupID, "div 事件 -占卜事件");
					}
					else if (Args.size() == 1)
					{
						std::unique_lock<std::mutex> lock(m_mutex);

						qqbot::Network::sendGroupMessage(groupID, std::format(R"(事件：
{}

成功的概率：
{}%
)",
Args[0], static_cast<long double>(m_mt() % 100000) / 1000));
					}
					else
					{
						std::string outString;

						for (auto i = Args.begin(); i != Args.end(); i++)
						{
							outString += *i + ' ';
						}

						std::unique_lock<std::mutex> lock(m_mutex);

						qqbot::Network::sendGroupMessage(groupID, std::format(R"(事件：
{}

成功的概率：
{}%
)",
outString, static_cast<long double>(m_mt() % 100000) / 1000));
					}
				},
				"div event",
				"占卜事件"
					);

			qqbot::ServerInfo::getPermission().setGroupDefaultPermission("div", true);
		}

	private:
		std::mt19937_64 m_mt;
		std::mutex		m_mutex;
	};
}
