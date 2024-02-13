﻿#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <shared_mutex>

#include "permission.h"
#include "register.h"

namespace qqbot
{
    class Command
    {
    public:
        //群消息处理函数模板
        //args: groupID, senderID, commandName, Args
        using GroupHandler = std::function<void(long long, long long, const std::string&, std::vector<std::string>)>;
        
        //单人消息处理函数模板
        //args: senderID, commandName, Args
        using UserHandler = std::function<void(long long, const std::string&, std::vector<std::string>)>;

    public:
        Command() = delete;
        Command(qqbot::Permission* permission, qqbot::Register* reg);
        Command(const Command&) = delete;
        Command(Command&& command) noexcept;
        ~Command();

        Command& operator=(const Command&) = delete;
        Command& operator=(Command&& command) noexcept;

        //添加群命令
        void addCommand(const std::string& commandName,
            GroupHandler handler,
            const std::string& commandFormat,
            const std::string& description);

        //添加私聊命令
        void addCommand(const std::string& commandName,
            UserHandler handler,
            const std::string& commandFormat,
            const std::string& description);

        //删除指令
        //isGroup为true则删除群指令，否则删除私聊指令
        void removeCommand(const std::string& commandName, bool isGroup);

        //群消息处理
        void groupExcute(long long groupID,
            long long senderID,
            const std::string& command,
            std::vector<std::string> args);

        //私聊消息处理
        void userExcute(long long senderID,
            const std::string& command,
            std::vector<std::string> args);

        //命令分割
        static void splitCommand(const std::string& command, std::string& commandHead, std::vector<std::string>& args);

    private:
        std::unordered_map<std::string, GroupHandler>    m_GroupHandlers;
        std::unordered_map<std::string, std::string>    m_groupCommandDescriptions;
        std::unordered_map<std::string, std::string>    m_groupCommandFormats;
        std::unordered_map<std::string, UserHandler>    m_UserHandlers;
        std::unordered_map<std::string, std::string>    m_userCommandDescriptions;
        std::unordered_map<std::string, std::string>    m_userCommandFormats;
        std::shared_mutex                                m_shareMutex;
        qqbot::Permission*                                m_permission;
        qqbot::Register*                                m_register;
    };
}
