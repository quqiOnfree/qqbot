#pragma once

#include <string>

namespace qqbot
{
	struct PluginInfo
	{
		//插件名称
		std::string name;

		//插件作者名
		std::string author;

		//插件版本
		std::string version;
	};

	class CppPlugin
	{
	public:
		//禁止复制，只能移动
		CppPlugin() = default;
		CppPlugin(const CppPlugin&) = delete;
		virtual ~CppPlugin() = default;

		CppPlugin& operator =(const CppPlugin&) = delete;

		//插件加载
		virtual void onLoad() {}

		//插件启动
		virtual void onEnable() {}

		//插件关闭
		virtual void onDisable() {}

	public:
		//插件信息
		PluginInfo pluginInfo;

	private:

	};
}
