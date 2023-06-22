#pragma once

#include <Json.h>
#include <vector>
#include <string>
#include <format>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"
#include "QuqiCoro.hpp"
#include "searchTreeLibrary.h"

namespace AutoAnswer
{
    class AutoAnswerPlugin : public qqbot::CppPlugin
    {
    public:
        AutoAnswerPlugin()
        {
            qqbot::CppPlugin::pluginInfo.author = "quqiOnfree and Error403";
            qqbot::CppPlugin::pluginInfo.name = "AutoAnswer";
            qqbot::CppPlugin::pluginInfo.version = "0.0.1";
        }
        virtual ~AutoAnswerPlugin() = default;

        //插件启动
        virtual void onEnable()
        {
            qqbot::ServerInfo::getCommander()
        }

    private:
        static SearchTreeLibrary::SearchTree m_searchTree;
    };
}

SearchTreeLibrary::SearchTree AutoAnswer::AutoAnswerPlugin::m_searchTree;
