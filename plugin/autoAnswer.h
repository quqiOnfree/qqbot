#pragma once

#include <Json.h>
#include <vector>
#include <string>
#include <format>
#include <filesystem>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"
#include "QuqiCoro.hpp"
#include "searchTreeLibrary.h"
#include "definition.h"

namespace AutoAnswer
{
    class AutoAnswerPlugin : public qqbot::CppPlugin
    {
    public:
        AutoAnswerPlugin() :
            m_searchTree(std::make_shared<SearchTreeLibrary::SearchTree>())
        {
            qqbot::CppPlugin::pluginInfo.author = "quqiOnfree and Error403";
            qqbot::CppPlugin::pluginInfo.name = "AutoAnswer";
            qqbot::CppPlugin::pluginInfo.version = "0.0.1";

            {
                std::ifstream infile("./plugin_config/AutoAnswer/config.json");
                if (!infile)
                    createConfig();
            }

            try
            {
                std::ifstream infile("./plugin_config/AutoAnswer/config.json");

                qjson::JObject jo(qjson::JParser::fastParse(infile));
                const qjson::list_t& list = jo.getList();

                //std::lock_guard<std::mutex> lock(m_serverMapMutex);
                for (const auto& i : list)
                {
                    m_searchTree->insert(i["question"].getString());
                    m_answerMap[i["question"].getString()] = i["answer"].getString();
                }
            }
            catch (const std::exception& e)
            {
                std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
            }
        }
        virtual ~AutoAnswerPlugin() = default;

        // 创建config文件
        void createConfig()
        {
            std::filesystem::create_directory("./plugin_config/AutoAnswer");
            qjson::JObject jo, localjo;
            localjo["question"] = "How are you?";
            localjo["answer"] = "I'm fine, thank you.";
            jo.push_back(localjo);
            std::ofstream outfile("./plugin_config/AutoAnswer/config.json");
            outfile << qjson::JWriter::fastFormatWrite(jo);
        }

        //插件启动
        virtual void onEnable()
        {
            qqbot::ServerInfo::getCommander().addCommand("ask",
                [&](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> args) {
                    if (args.empty())
                    {
                        qqbot::Network::sendGroupMessage(groupID,
                            "ask [msg] -询问问题\n"
                            "ask reload -重载\n"
                        );
                        return;
                    }

                    if (args[0] == "reload")
                    {
                        m_searchTree = std::make_shared<SearchTreeLibrary::SearchTree>();
                        m_answerMap.clear();

                        try
                        {
                            std::ifstream infile("./plugin_config/AutoAnswer/config.json");

                            qjson::JObject jo(qjson::JParser::fastParse(infile));
                            const qjson::list_t& list = jo.getList();

                            //std::lock_guard<std::mutex> lock(m_serverMapMutex);
                            for (const auto& i : list)
                            {
                                m_searchTree->insert(i["question"].getString());
                                m_answerMap[i["question"].getString()] = i["answer"].getString();
                            }
                        }
                        catch (const std::exception& e)
                        {
                            std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
                            qqbot::Network::sendGroupMessage(groupID, std::format("{}: {}", "重载失败", e.what()));
                        }

                        qqbot::Network::sendGroupMessage(groupID, "重载成功");

                        return;
                    }

                    std::string question;
                    for (size_t i = 0; i < args.size(); i++)
                    {
                        if (args.size() - 1 == i)
                        {
                            question += args[i];
                            break;
                        }
                        question += args[i] + ' ';
                    }

                    if (question.empty())
                    {
                        qqbot::Network::sendGroupMessage(groupID, "问题不能为空");
                        return;
                    }

                    qqbot::Network::sendGroupMessage(groupID, m_answerMap[m_searchTree->getOriginalString(question)]);
                    return;
                },
                "ask [msg]",
                "询问问题"
                );

            qqbot::ServerInfo::getPermission().setGroupDefaultPermission("ask", true);
        }

    private:
        std::shared_ptr<SearchTreeLibrary::SearchTree>  m_searchTree;
        std::unordered_map<std::string, std::string>    m_answerMap;
    };
}
