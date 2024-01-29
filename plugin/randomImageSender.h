#pragma once

#include <Json.h>
#include <vector>
#include <string>
#include <format>
#include <filesystem>
#include <shared_mutex>
#include <httplib.h>
#include <atomic>
#include <random>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"
#include "searchTreeLibrary.h"
#include "definition.h"

namespace RandomImageSender
{
    class RandomImageSenderPlugin : public qqbot::CppPlugin
    {
    public:
        RandomImageSenderPlugin() :
            m_mt(std::random_device{}())
        {
            qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
            qqbot::CppPlugin::pluginInfo.name = "RandomImageSender";
            qqbot::CppPlugin::pluginInfo.version = "0.0.1";
        }
        ~RandomImageSenderPlugin() = default;

        void createConfig()
        {
            qjson::JObject json;
            json["is_enable"] = false;

            qjson::JObject localjson;
            localjson["function_name"] = "image";
            localjson["function_description"] = "This is a image function";
            localjson["file_path"] = "./path";

            json["paths"].push_back(localjson);

            std::filesystem::create_directory("./plugin_config/RandomImageSender");

            std::ofstream outfile("./plugin_config/RandomImageSender/config.json");
            outfile << qjson::JWriter::fastFormatWrite(json);
        }

        void onLoad() override
        {
            {
                std::ifstream infile("./plugin_config/RandomImageSender/config.json");
                if (!infile)
                    createConfig();
            }

            try
            {
                // 配置文件
                std::ifstream infile("./plugin_config/RandomImageSender/config.json");

                qjson::JObject jo(qjson::JParser::fastParse(infile));
                if (!jo["is_enable"].getBool())
                {
                    m_is_enabled = false;
                    return;
                }
                m_is_enabled = true;
                   
                const auto& list = jo["paths"].getList();
                for (const auto& i : list)
                {
                    m_file_path_map[i["function_name"].getString()] = { i["file_path"].getString(), i["function_description"].getString()};
                }
            }
            catch (const std::exception& e)
            {
                std::cout << ERROR_WITH_STACKTRACE(e.what()) << '\n';
            }
        }

        void onEnable() override
        {
            if (!m_is_enabled) return;

            for (const auto& [function_name, functionStruct] : m_file_path_map)
            {
                qqbot::ServerInfo::getCommander().addCommand(function_name,
                    [function_name, functionStruct, this](long long groupID,
                        long long senderID,
                        const std::string& commandName,
                        std::vector<std::string> Args) -> void {

                        if (Args.empty())
                        {
                            qqbot::Network::sendGroupMessage(groupID,
                                std::format("{} -{}", function_name, functionStruct.description));
                            return;
                        }
                        else if (Args.size() > 0)
                        {
                            qqbot::Network::sendGroupMessage(groupID, "参数错误");
                            return;
                        }

                        // 遍历图片表
                        std::vector<std::string> files;
                        std::filesystem::directory_iterator file_itor(functionStruct.path);
                        for (; !file_itor._At_end(); file_itor++)
                        {
                            if (file_itor->is_regular_file())
                                files.emplace_back(file_itor->path().string());
                        }

                        // 选取随机图片
                        size_t rannum = 0;
                        {
                            std::unique_lock<std::mutex> ul(m_mutex);
                            rannum = m_mt() % files.size();
                        }

                        qqbot::Network::sendGroupMessage(groupID,
                            std::format("[CQ:image,file=file:///{},id=40000]", 
                                std::filesystem::absolute(files.at(rannum)).string()));
                        return;
                    },
                    function_name,
                    functionStruct.description);
            }
        }

        void onDisable() override
        {

        }

    protected:
        struct FunctionStruct
        {
            std::string path;
            std::string description;
        };

    private:
        std::atomic<bool>   m_is_enabled;
        std::unordered_map<std::string,
            FunctionStruct> m_file_path_map;

        std::mt19937_64     m_mt;
        std::mutex          m_mutex;
    };
}

