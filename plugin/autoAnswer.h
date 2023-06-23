#pragma once

#include <Json.h>
#include <vector>
#include <string>
#include <format>
#include <filesystem>
#include <shared_mutex>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"
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
                // 配置文件
                std::ifstream infile("./plugin_config/AutoAnswer/config.json");

                qjson::JObject jo(qjson::JParser::fastParse(infile));
                const qjson::list_t& list = jo.getList();

                // 将配置文件内容加入 搜索树 和 答案表
                for (const auto& i : list)
                {
                    // 如果关键词重复则报错
                    if (m_answerMap.find(i["question"].getString()) != m_answerMap.end())
                        throw std::logic_error("问题关键词重复");

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

            localjo["question"] = "你是谁";
            localjo["answer"] = "我是qqbot, github地址: https://github.com/quqiOnfree/qqbot";
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

                    // 重载
                    if (args[0] == "reload")
                    {
                        if (!qqbot::ServerInfo::getPermission().hasUserOperator(senderID))
                        {
                            qqbot::Network::sendGroupMessage(groupID, "你没有权限");
                            return;
                        }

                        // 写入互斥锁
                        std::lock_guard<std::shared_mutex> lock(m_mutex);
                        // 重新加载 搜索树 和 答案表
                        m_searchTree = std::make_shared<SearchTreeLibrary::SearchTree>();
                        m_answerMap.clear();

                        try
                        {
                            // 配置文件
                            std::ifstream infile("./plugin_config/AutoAnswer/config.json");

                            // json object
                            qjson::JObject jo(qjson::JParser::fastParse(infile));
                            const qjson::list_t& list = jo.getList();

                            // 将配置文件内容加入 搜索树 和 答案表
                            for (const auto& i : list)
                            {
                                // 如果关键词重复则报错
                                if (m_answerMap.find(i["question"].getString()) != m_answerMap.end())
                                    throw std::logic_error("问题关键词重复");

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

                    // 获取问题

                    // 问题
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

                    {
                        // 读取互斥锁
                        std::shared_lock<std::shared_mutex> lock(m_mutex);
                        // 获取匹配内容回答
                        qqbot::Network::sendGroupMessage(groupID, m_answerMap[m_searchTree->getOriginalString(question)]);
                    }
                    return;
                },
                "ask [msg]",
                "询问问题"
                );

            // 权限设置为所有人都能用
            qqbot::ServerInfo::getPermission().setGroupDefaultPermission("ask", true);
        }

    private:
        // 模糊搜索树
        std::shared_ptr<SearchTreeLibrary::SearchTree>  m_searchTree;
        // 问题对应答案表
        std::unordered_map<std::string, std::string>    m_answerMap;
        // 互斥量
        mutable std::shared_mutex                       m_mutex;
    };
}
