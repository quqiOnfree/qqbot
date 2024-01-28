#pragma once

#include <Json.h>
#include <vector>
#include <string>
#include <format>
#include <filesystem>
#include <shared_mutex>
#include <queue>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"
#include "definition.h"

namespace AutoAnswer
{
    class AutoAnswerPlugin : public qqbot::CppPlugin
    {
    public:
        class SearchTree
        {
        public:
            SearchTree() = default;
            ~SearchTree() = default;

            // 加入
            void insert(const std::string& data)
            {
                m_strings.push_back(data);
                size_t position = m_strings.size() - 1;
                for (auto i = data.begin(); i != data.end(); i++)
                {
                    m_tree[*i].insert(position);
                }
            }

            // 获取模糊搜索的匹配字符串
            std::string getOriginalString(const std::string& data) const
            {
                if (data.empty())
                {
                    throw std::logic_error("data is empty");
                }

                std::set<size_t> locSet;
                {
                    size_t i = 0;
                    while (data.size() > i && m_tree.find(data[i++]) == m_tree.end()) {}
                    auto itor = m_tree.find(data[i - 1]);
                    if (itor == m_tree.end())
                        throw std::logic_error("can't find this question");
                    locSet = itor->second;
                }

                for (auto i = data.begin() + 1; i != data.end(); i++)
                {
                    auto itor = m_tree.find(*i);
                    if (itor == m_tree.end()) continue;
                    locSet.insert(itor->second.begin(), itor->second.end());
                }

                std::priority_queue<std::pair<int, std::string>,
                    std::vector<std::pair<int, std::string>>,
                    std::greater<std::pair<int, std::string>>> buffer;
                for (auto i = locSet.begin(); i != locSet.end(); i++)
                {
                    const auto& str = m_strings[*i];
                    buffer.emplace(std::pair<int, std::string>{minDistance(data, str), str});
                }

                std::string restr = buffer.top().second;
                while (!buffer.empty() && kmp(data, buffer.top().second) == -1)
                {
                    restr = buffer.top().second;
                    buffer.pop();
                }
                return restr;
            }

        protected:
            static int minDistance(const std::string& word1, const std::string& word2) {
                int n = (int)word1.length();
                int m = (int)word2.length();
                if (n * m == 0) return n + m;

                std::vector<std::vector<int>> D(n + 1, std::vector<int>(m + 1));

                for (int i = 0; i < n + 1; i++)
                {
                    D[i][0] = i;
                }
                for (int j = 0; j < m + 1; j++)
                {
                    D[0][j] = j;
                }

                for (int i = 1; i < n + 1; i++)
                {
                    for (int j = 1; j < m + 1; j++)
                    {
                        int left = D[i - 1][j] + 1;
                        int down = D[i][j - 1] + 1;
                        int left_down = D[i - 1][j - 1];
                        if (word1[i - 1] != word2[j - 1]) left_down += 1;
                        D[i][j] = std::min(left, std::min(down, left_down));
                    }
                }
                return D[n][m];
            }

            static long long kmp(const std::string& match_string, const std::string& pattern)
            {
                if (match_string.length() < pattern.length()) return -1;

                for (size_t i = 0; i < match_string.size(); i++)
                {
                    size_t j = 0;
                    for (; j < match_string.size() - i && j < pattern.size(); j++)
                    {
                        if (match_string[i + j] != pattern[j])
                            break;
                    }
                    if (j == pattern.size())
                        return i;
                }
                return -1;
            }

        private:
            // 搜索树本体
            std::unordered_map<char, std::set<size_t>>	m_tree;
            // 匹配字符串
            std::vector<std::string>					m_strings;
        };

        AutoAnswerPlugin() :
            m_searchTree(std::make_shared<SearchTree>())
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
                        m_searchTree = std::make_shared<SearchTree>();
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

        virtual void onDisable()
        {
            qqbot::ServerInfo::getCommander().removeCommand("ask", true);
        }

    private:
        // 模糊搜索树
        std::shared_ptr<SearchTree>  m_searchTree;
        // 问题对应答案表
        std::unordered_map<std::string, std::string>    m_answerMap;
        // 互斥量
        mutable std::shared_mutex                       m_mutex;
    };
}
