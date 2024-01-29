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

            qjson::JObject localjson;
            localjson["word"] = "fuck";
            localjson["is_mute"] = true;

            json.push_back(localjson);

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
                const qjson::list_t& list = jo.getList();

                // 将配置文件内容加入 搜索树
                for (const auto& i : list)
                {
                    if (i["is_mute"].getBool())
                    {
                        m_MuteType_map[i["word"].getString()] = MuteType::MUTE;
                        m_SearchTree.insert(i["word"].getString());
                    }
                    else
                    {
                        m_MuteType_map[i["word"].getString()] = MuteType::NOMAL;
                        m_SearchTree.insert(i["word"].getString());
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
                    std::shared_lock<std::shared_mutex> sl1(m_MuteType_map_mutex, std::defer_lock), sl2(m_SearchTree_mutex, std::defer_lock);
                    std::lock(sl1, sl2);

                    auto itor = m_MuteType_map.find(m_SearchTree.getOriginalString(message));
                    if (itor == m_MuteType_map.end()) return;

                    if (itor->second == MuteType::MUTE)
                    {
                        auto result = localGet("get_group_member_info",
                            { {"group_id", std::to_string(group_id)},
                                {"user_id" , std::to_string(user_id)},
                                {"no_cache", "false"}});

                        // 如果为管理则跳过
                        if (result.has_request_header("role") &&
                            result.get_request_header_value("role") != "member") return;

                        // 撤回消息
                        localGet("delete_msg", {{"message_id", std::to_string(message_id)}});
                        qqbot::Network::sendGroupMessage(group_id, std::format("[CQ:at,qq={}] 检测到关键词，你的消息已被撤回！", user_id));
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
        std::unordered_map<std::string, MuteType>   m_MuteType_map;
        std::shared_mutex                           m_MuteType_map_mutex;

        SearchTreeLibrary::SearchTree               m_SearchTree;
        std::shared_mutex                           m_SearchTree_mutex;
    };
}
