#include "register.h"

#include <iostream>

#include "definition.h"

//添加插件
#include "mcrcon.h"
#include "divination.h"
#include "mil.h"
#include "groupOperator.h"
#include "weather.h"

namespace qqbot
{
	void Register::init()
	{
		//添加mcrcon插件
		this->addPlugin(std::make_shared<MCRCON::MCRCONPlugin>());

		//添加占卜插件
		this->addPlugin(std::make_shared<Divination::DivinationPlugin>());

		//添加人间地狱密位换算插件
		this->addPlugin(std::make_shared<MIL::MILPlugin>());

		//添加群组管理插件
		this->addPlugin(std::make_shared<GroupOperator::GroupOperatorPlugin>());

		//添加天气插件
		this->addPlugin(std::make_shared<Weather::WeatherPlugin>());
	}

	void Register::run()
	{
		for (auto& [name, plugin] : m_plugins)
		{
			std::cout << std::format("Plugin: {} v{} onEnabled", name, plugin->pluginInfo.version);
			plugin->onEnable();
		}
	}

	void Register::addPlugin(const std::shared_ptr<CppPlugin>& plugin)
	{
		std::cout << std::format("Plugin: {} v{} onLoaded", plugin->pluginInfo.name, plugin->pluginInfo.version);
		if (m_plugins.find(plugin->pluginInfo.name) != m_plugins.end())
		{
			throw THROW_ERROR("plugin has the same name!");
		}
		m_plugins[plugin->pluginInfo.name] = plugin;
		plugin->onLoad();
	}

	const std::unordered_map<std::string, std::shared_ptr<CppPlugin>>& Register::getPlugins() const
	{
		return m_plugins;
	}
}
