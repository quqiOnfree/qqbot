#pragma once

#include "Json.h"

namespace qqbot
{
	class CPPPlugin
	{
	public:
		CPPPlugin() = default;

		~CPPPlugin() = default;

		CPPPlugin& operator =(const CPPPlugin&) = delete;

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
		qjson::JObject pluginInfo;

	private:

	};
}
