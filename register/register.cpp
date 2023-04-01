#include "register.h"

#include <iostream>

//添加插件
#include "mcrcon.h"
#include "divination.h"
#include "mil.h"

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
	}

	void Register::run()
	{
		for (auto i = m_plugins.begin(); i != m_plugins.end(); i++)
		{
			std::cout << "Plugin " << i->first << " v" << i->second->pluginInfo.version << " onEnabled\n";
			i->second->onEnable();
		}
	}

	void Register::addPlugin(const std::shared_ptr<CppPlugin>& plugin)
	{
		std::cout << "Plugin " << plugin->pluginInfo.name << " v" << plugin->pluginInfo.version << " onLoaded\n";
		m_plugins[plugin->pluginInfo.name] = plugin;
		plugin->onLoad();
	}
}
