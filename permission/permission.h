#pragma once

#include <string>
#include <string_view>
#include <fstream>

#include <Json.h>

namespace qqbot
{
	class Permission
	{
	public:
		Permission();
		~Permission();

		void init();
		void save();

		bool					hasGroupDefaultPermission(const std::string& permissionName);
		bool					getGroupDefaultPermission(const std::string& permissionName);
		void					setGroupDefaultPermission(const std::string& permissionName, bool boolean);

		bool					hasSingleGroupDefaultPermission(int groupID, const std::string& permissionName);
		bool					getSingleGroupDefaultPermission(int groupID, const std::string& permissionName);
		void					setSingleGroupDefaultPermission(int groupID, const std::string& permissionName, bool boolean);

		bool					hasUserOperator(int userID);
		void					setUserOperator(int userID, bool boolean);
		const qjson::JObject&	getUserOperatorList();

	protected:
		void makeConfigFile();

	private:
		const std::string	m_filepath;
		qjson::JObject		m_json;
	};
}
