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

		//初始化设置
		void init();
		//配置文件保存
		void save();

		//判断是否有默认群权限设置
		bool hasGroupDefaultPermission(const std::string& permissionName);
		//获取默认群权限设置
		bool getGroupDefaultPermission(const std::string& permissionName);
		//设置默认群权限设置
		void setGroupDefaultPermission(const std::string& permissionName, bool boolean);

		//判断是否有个性化群权限设置
		bool hasSingleGroupDefaultPermission(long long groupID, const std::string& permissionName);
		//获取个性化群权限
		bool getSingleGroupDefaultPermission(long long groupID, const std::string& permissionName);
		//设置个性化群权限
		void setSingleGroupDefaultPermission(long long groupID, const std::string& permissionName, bool boolean);

		//获取是否是operator
		bool hasUserOperator(long long userID);
		//设置operator
		void setUserOperator(long long userID, bool boolean);
		//获取operator列表
		const qjson::JObject& getUserOperatorList();

	public:
		//go-cqhttp的ip
		std::string		m_gocq_ip;

		//go-cqhttp的port
		unsigned short	m_gocq_port;

		//server的监听ip
		std::string		m_server_ip;

		//server的监听port
		unsigned short	m_server_port;

	protected:
		//配置文件初始化
		void makeConfigFile();

	private:
		const std::string	m_filepath;
		qjson::JObject		m_json;
	};
}
