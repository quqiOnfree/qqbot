#pragma once

#include <Json.h>
#include <vector>
#include <string>
#include <format>
#include <filesystem>
#include <shared_mutex>
#include <unordered_set>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"
#include "searchTreeLibrary.h"
#include "definition.h"

namespace AutoMuter
{
    class AutoMuterPlugin: public qqbot::CppPlugin
    {
    public:
        enum class MuteType
        {
            NOMAL = 0,
            MUTE
        };

        AutoMuterPlugin()
        {
            qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
            qqbot::CppPlugin::pluginInfo.name = "AutoMuter";
            qqbot::CppPlugin::pluginInfo.version = "0.0.1";
        }
        ~AutoMuterPlugin() = default;

        void createConfig()
        {
            qjson::JObject json;
            json["enable_groups"].push_back(123456789);

            qjson::JObject localjson;
            localjson["word"] = "fuck";
            localjson["is_mute"] = true;
            localjson["description"] = "检测到关键词";

            json["words"].push_back(localjson);

            std::filesystem::create_directory("./plugin_config/AutoMuter");

            std::ofstream outfile("./plugin_config/AutoMuter/config.json");
            outfile << qjson::JWriter::fastFormatWrite(json);
        }

        void onLoad() override
        {
            {
                std::ifstream infile("./plugin_config/AutoMuter/config.json");
                if (!infile)
                    createConfig();
            }

            try
            {
                // 配置文件
                std::ifstream infile("./plugin_config/AutoMuter/config.json");

                qjson::JObject jo(qjson::JParser::fastParse(infile));
                {
                    const qjson::list_t& list = jo["words"].getList();

                    // 将配置文件内容加入 搜索树
                    for (const auto& i : list)
                    {
                        if (i["is_mute"].getBool())
                        {
                            m_MuteType_map[i["word"].getString()] = { MuteType::MUTE, i["description"].getString() };
                        }
                        else
                        {
                            m_MuteType_map[i["word"].getString()] = { MuteType::NOMAL, i["description"].getString() };
                        }
                    }
                }
                {
                    const qjson::list_t& list = jo["enable_groups"].getList();

                    // 将配置文件内容加入 搜索树
                    for (const auto& i : list)
                    {
                        m_group_map.insert(i.getInt());
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
            }
        }

        void onEnable() override
        {
            qqbot::ServerInfo::getEventRegister().addGroupReceiveEventFunction("AutoMuterPlugin_GroupApplicationEventFunction",
                [this](long long group_id, long long user_id, long long message_id, std::string message) -> void {
                try
                {
                    std::shared_lock<std::shared_mutex> sl1(m_MuteType_map_mutex, std::defer_lock), sl2(m_group_map_mutex, std::defer_lock);
                    std::lock(sl1, sl2);

                    // 不在监管范围内则不判断
                    if (m_group_map.find(group_id) == m_group_map.end()) return;

                    std::string match_word;
                    for (const auto& [word, muteStruct] : m_MuteType_map)
                    {
                        if (SearchTreeLibrary::SearchTree::kmp(message, word) != -1)
                        {
                            match_word = word;
                            break;
                        }
                    }
                    if (match_word.empty()) return;

                    if (m_MuteType_map[match_word].muteType == MuteType::MUTE)
                    {
                        auto result = localGet("get_group_member_info",
                            { {"group_id", std::to_string(group_id)},
                                {"user_id" , std::to_string(user_id)},
                                {"no_cache", "false"}});

                        // 如果为管理则跳过
                        if (result->headers.find("role") != result->headers.end() &&
                            result->headers.find("role")->second != "member") return;

                        // 撤回消息
                        localGet("delete_msg", {{"message_id", std::to_string(message_id)}});
                        qqbot::Network::sendGroupMessage(group_id, std::format("[CQ:at,qq={}] {}",
                            user_id, m_MuteType_map[match_word].description));
                        return;
                    }
                }
                catch (...)
                {
                    return;
                }
                });
        }

        void onDisable() override
        {

        }

    protected:
        struct MuteStruct
        {
            MuteType    muteType;
            std::string description;
        };

        static httplib::Result localGet(const std::string& command, std::initializer_list<std::pair<std::string, std::string>> list)
        {
            httplib::Client cli(std::format("http://{}:{}", qqbot::ServerInfo::getPermission().m_gocq_ip, qqbot::ServerInfo::getPermission().m_gocq_port));
            httplib::Params params;

            for (auto i = list.begin(); i != list.end(); i++)
            {
                params.insert(*i);
            }

            return cli.Get("/" + command, params, httplib::Headers());
        }

    private:
        std::unordered_map<std::string, MuteStruct> m_MuteType_map;
        std::shared_mutex                           m_MuteType_map_mutex;

        std::unordered_set<long long>               m_group_map;
        std::shared_mutex                           m_group_map_mutex;
    };
}
