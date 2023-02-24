#include "permission.h"

#include <filesystem>
#include <iostream>

namespace qqbot
{
	Permission::Permission()
		: m_filepath("./config/config.json")
	{
	}

	Permission::~Permission()
	{
		this->save();
	}

	void Permission::init()
	{
		if (std::filesystem::create_directory("./config"))
		{
			this->makeConfigFile();
			exit(-1);
		}

		//config.json
		std::ifstream infile(m_filepath);
		if (!infile)
		{
			this->makeConfigFile();
			exit(-1);
		}

		try
		{
			//读取config.json json信息
			m_json =  qjson::JParser::fastParse(infile);
		}
		catch (const std::exception&)
		{
			this->makeConfigFile();
			exit(-1);
		}
	}

	void Permission::save()
	{
		//保存config.json
		std::ofstream outfile(m_filepath);

		outfile << qjson::JWriter::fastFormatWrite(m_json);
	}

	bool Permission::hasGroupDefaultPermission(const std::string& permissionName)
	{
		if (m_json["permission"]["default"].hasMember(permissionName)
			)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Permission::getGroupDefaultPermission(const std::string& permissionName)
	{
		if (this->hasGroupDefaultPermission(permissionName) &&
			m_json["permission"]["default"][permissionName.c_str()].getBool()
			)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void Permission::setGroupDefaultPermission(const std::string& permissionName, bool boolean)
	{
		m_json["permission"]["default"][permissionName.c_str()] = boolean;
		this->save();
	}


	bool Permission::hasSingleGroupDefaultPermission(int groupID, const std::string& permissionName)
	{
		if (m_json["permission"].hasMember("group") &&
			m_json["permission"]["group"].hasMember(std::to_string(groupID)) &&
			m_json["permission"]["group"][std::to_string(groupID).c_str()].hasMember(permissionName)
			)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Permission::getSingleGroupDefaultPermission(int groupID, const std::string& permissionName)
	{
		if (this->hasSingleGroupDefaultPermission(groupID, permissionName) &&
			m_json["permission"]["group"][std::to_string(groupID).c_str()][permissionName.c_str()].getBool()
			)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void Permission::setSingleGroupDefaultPermission(int groupID, const std::string& permissionName, bool boolean)
	{
		m_json["permission"]["group"][std::to_string(groupID).c_str()][permissionName.c_str()] = boolean;
		this->save();
	}

	bool Permission::hasUserOperator(int userID)
	{
		qjson::list_t& list = m_json["permission"]["op"].getList();

		if (std::find(list.begin(),
			list.end(),
			qjson::JObject(userID)
		) != list.end())
		{
			return true;
		}
		return false;
	}

	void Permission::setUserOperator(int userID, bool boolean)
	{
		qjson::list_t& list = m_json["permission"]["op"].getList();
		auto itor = std::find(list.begin(),
			list.end(),
			qjson::JObject(userID)
		);

		if (itor != list.end() && !boolean)
		{
			list.erase(itor);
		}
		else if (boolean)
		{
			list.push_back(userID);
		}

		this->save();
	}

	const qjson::JObject& Permission::getUserOperatorList()
	{
		return m_json["permission"]["op"];
	}

	void Permission::makeConfigFile()
	{
		//创建config.json
		std::cout << "请填写 ./config/config.json\n";

		std::filesystem::create_directory("./config");

		std::ofstream outfile("./config/config.json");

		qjson::JObject json;

		json["permission"]["op"].push_back(2098332747);
		json["permission"]["default"]["command"].push_back("help");

		outfile << qjson::JWriter::fastFormatWrite(json);
	}
}
