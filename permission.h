#pragma once

#include <string>
#include <string_view>
#include <fstream>

#include "Json.h"

namespace qqbot
{
	class Permission
	{
	public:
		Permission();
		~Permission();

		bool hasGroupDefaultPermission(const std::string& permissionName);
		qjson::JObject getGroupDefaultPermission(const std::string& permissionName);
		void setGroupDefaultPermission(const std::string& permissionName, const qjson::JObject& json);

		bool hasSingleGroupDefaultPermission(int groupID, const std::string& permissionName);
		qjson::JObject getSingleGroupDefaultPermission(int groupID, const std::string& permissionName);
		void setSingleGroupDefaultPermission(int groupID, const std::string& permissionName, const qjson::JObject& json);

		bool hasUserOperator(int userID);
		void setUserOperator(int userID, bool boolean);

	private:

	};
}
