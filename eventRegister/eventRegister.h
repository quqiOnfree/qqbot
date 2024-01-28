#pragma once

#include <unordered_map>
#include <memory>
#include <functional>
#include <string>
#include <shared_mutex>

namespace qqbot
{
    class EventRegister
    {
    public:
        /*
        * @brief 群聊消息接收函数体
        * @param 1 group_id
        * @param 2 user_id
        * @param 3 requestId
        * @param 4 message
        */
        using GroupReceiveEventFunction = std::function<void(long long, long long, long long, std::string)>;

        /*
        * @brief 私聊消息接收函数体
        * @param 1 user_id
        * @param 2 requestId
        * @param 3 message
        */
        using UserReceiveEventFunction = std::function<void(long long, long long, std::string)>;

        /*
        * @brief 用户申请接收函数体
        * @param 1 user_id
        * @param 2 message
        */
        using FriendApplicationEventFunction = std::function<void(long long, std::string)>;

        using GroupApplicationEventFunction = std::function<void(long long, long long, std::string)>;

        EventRegister() = default;
        ~EventRegister() = default;

        /*
        * @brief 添加私聊消息接收函数
        * @param function_name 用于函数标识
        * @param func 回调函数
        */
        void addUserReceiveEventFunction(const std::string& function_name,
            const UserReceiveEventFunction& func);

        /*
        * @brief 添加群聊消息接收函数
        * @param function_name 用于函数标识
        * @param func 回调函数
        */
        void addGroupReceiveEventFunction(const std::string& function_name,
            const GroupReceiveEventFunction& func);

        [[deprecated("未完成")]] void addFriendApplicationEventFunction(const std::string& function_name,
            const FriendApplicationEventFunction& func);

        [[deprecated("未完成")]] void addGroupApplicationEventFunction(const std::string& function_name,
            const GroupApplicationEventFunction& func);

        void updateUserReceiveEvent(long long user_id, long long requestId, const std::string& message);
        void updateGroupReceiveEvent(long long group_id, long long user_id, long long requestId, const std::string& message);

    private:
        std::unordered_map<std::string,
            UserReceiveEventFunction>   m_UserReceiveEventFunction_map;
        std::shared_mutex               m_UserReceiveEventFunction_map_mutex;

        std::unordered_map<std::string,
            GroupReceiveEventFunction>  m_GroupReceiveEventFunction_map;
        std::shared_mutex               m_GroupReceiveEventFunction_map_mutex;
    };
}
