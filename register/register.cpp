#include "register.h"

#include <iostream>

//添加插件
#include "mcrcon.h"
#include "divination.h"
#include "mil.h"
#include "groupOperator.h"
#include "weather.h"
#include "autoAnswer.h"
#include "autoMuter.h"
#include "randomImageSender.h"

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

        //添加自动回答插件
        this->addPlugin(std::make_shared<AutoAnswer::AutoAnswerPlugin>());

        //添加自动检测发言插件
        this->addPlugin(std::make_shared<AutoMuter::AutoMuterPlugin>());

        //添加发送图片插件
        this->addPlugin(std::make_shared<RandomImageSender::RandomImageSenderPlugin>());
    }

    void Register::run()
    {
        for (auto& [pluginName, plugin] : m_plugins)
        {
            std::cout << "Plugin " << pluginName << " v" << plugin->pluginInfo.version << " onEnabled\n";
            plugin->onEnable();
        }
    }

    void Register::addPlugin(const std::shared_ptr<CppPlugin>& plugin)
    {
        std::cout << "Plugin " << plugin->pluginInfo.name << " v" << plugin->pluginInfo.version << " onLoaded\n";
        m_plugins[plugin->pluginInfo.name] = plugin;
        plugin->onLoad();
    }

    const std::unordered_map<std::string, std::shared_ptr<CppPlugin>>& Register::getPlugins() const
    {
        return m_plugins;
    }
}
