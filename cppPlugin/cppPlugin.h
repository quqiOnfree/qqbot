#pragma once

#include <string>

#include "command.h"
#include "permission.h"

namespace qqbot
{
	struct PluginInfo
	{
		std::string name;
		std::string author;
		std::string version;
	};

	class CppPlugin
	{
	public:
		CppPlugin() = default;
		CppPlugin(const CppPlugin&) = delete;
		~CppPlugin() = default;

		CppPlugin& operator =(const CppPlugin&) = delete;

		virtual void onLoad()
		{
		}

		virtual void onEnable()
		{
		}

		virtual void onDisable()
		{
		}

	public:
		PluginInfo pluginInfo;

	private:

	};
}
