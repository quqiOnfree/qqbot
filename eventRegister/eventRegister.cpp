#include "eventRegister.h"

namespace qqbot
{
    void EventRegister::addUserReceiveEventFunction(const std::string& function_name, const UserReceiveEventFunction& func)
    {
        std::unique_lock<std::shared_mutex> ul(m_UserReceiveEventFunction_map_mutex);
        m_UserReceiveEventFunction_map.emplace(function_name, func);
    }

    void EventRegister::addGroupReceiveEventFunction(const std::string& function_name, const GroupReceiveEventFunction& func)
    {
        std::unique_lock<std::shared_mutex> ul(m_GroupReceiveEventFunction_map_mutex);
        m_GroupReceiveEventFunction_map.emplace(function_name, func);
    }

    void EventRegister::updateUserReceiveEvent(long long user_id, long long requestId, const std::string& message)
    {
        std::shared_lock<std::shared_mutex> sl(m_UserReceiveEventFunction_map_mutex);
        for (const auto& [_, func] : m_UserReceiveEventFunction_map)
        {
            func(user_id, requestId, message);
        }
    }

    void EventRegister::updateGroupReceiveEvent(long long group_id, long long user_id, long long requestId, const std::string& message)
    {
        std::shared_lock<std::shared_mutex> sl(m_GroupReceiveEventFunction_map_mutex);
        for (const auto& [_, func] : m_GroupReceiveEventFunction_map)
        {
            func(group_id, user_id, requestId, message);
        }
    }
}