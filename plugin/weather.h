#pragma once

#include <iostream>
#include <Json.h>
#include <httplib.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"

namespace Weather
{
	class WeatherPlugin : public qqbot::CppPlugin
	{
	public:
		//字符串关键字搜索树
		class SearchTree
		{
		public:
			SearchTree() = default;
			~SearchTree() = default;

			void insert(const std::string& data)
			{
				m_strings.push_back(data);
				size_t position = m_strings.size() - 1;
				for (auto i = data.begin(); i != data.end(); i++)
				{
					m_tree[*i].insert(position);
				}
			}

			std::string getOriginalString(const std::string& data)
			{
				if (data.empty())
				{
					throw std::exception("data is empty");
				}

				std::set<size_t> locSet;
				locSet = m_tree[data[0]];

				for (auto i = data.begin() + 1; i != data.end(); i++)
				{
					std::set<size_t> locSet_2 = locSet;

					for (auto j = locSet_2.begin(); j != locSet_2.end(); j++)
					{
						if (m_tree[*i].find(*j) == m_tree[*i].end())
						{
							locSet.erase(*j);
						}
					}

					if (locSet.empty())
					{
						size_t min = m_strings[*locSet_2.begin()].size();
						size_t pos = *locSet_2.begin();

						for (auto i = locSet_2.begin(); i != locSet_2.end(); i++)
						{
							if (m_strings[*i].size() < min)
							{
								min = m_strings[*i].size();
								pos = *i;
							}
						}

						return m_strings[pos];
					}
					else if (locSet.size() == 1)
					{
						return m_strings[*locSet.begin()];
					}
				}

				{
					size_t min = m_strings[*locSet.begin()].size();
					size_t pos = *locSet.begin();

					for (auto i = locSet.begin(); i != locSet.end(); i++)
					{
						if (m_strings[*i] == data)
						{
							return data;
						}
						else if (m_strings[*i].size() < min)
						{
							min = m_strings[*i].size();
							pos = *i;
						}
					}

					bool canFindout = false;
					std::string_view getStr = m_strings[pos];
					for (size_t i = 0; i < getStr.size() - data.size(); i++)
					{
						if (getStr.substr(i, data.size()) == data)
						{
							canFindout = true;
							break;
						}
					}


					if (canFindout)
					{
						return m_strings[pos];
					}
					else
					{
						throw std::exception("找不到此地点");
					}
				}
			}
		private:
			std::unordered_map<char, std::set<size_t>> m_tree;
			std::vector<std::string> m_strings;
		};

		WeatherPlugin() : m_apiKey("dffc4517b36352b24303ecd2493658c7")
		{
			qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
			qqbot::CppPlugin::pluginInfo.name = "Weather";
			qqbot::CppPlugin::pluginInfo.version = "0.0.1";
		}

		~WeatherPlugin() = default;

		virtual void onLoad()
		{
			std::cout << "[Weather]Hello!I'm on Loading...\n";
			if (m_cityCode.empty())
			{
				std::ifstream infile("./citycodes.json");
				qjson::JObject cityCode(qjson::JParser::fastParse(infile));
				qjson::dict_t& locDict = cityCode.getDict();
				for (auto i = locDict.begin(); i != locDict.end(); i++)
				{
					qjson::dict_t& locDict_2 = i->second.getDict();
					for (auto j = locDict_2.begin(); j != locDict_2.end(); j++)
					{
						m_cityCode[i->first][j->first] = j->second.getString();
						m_searchTree.insert(i->first);
					}
				}
			}
			std::cout << "[Weather]Sussessfully Loaded!\n";
		}

		virtual void onEnable()
		{
			qqbot::ServerInfo::getCommander().addCommand("tq",
				[this](long long groupID, long long senderID, const std::string& commandName, std::vector<std::string> Args)
				{
					if (Args.empty())
					{
						qqbot::Network::sendGroupMessage(groupID, "tq 位置 a(实况)/b(预报) -天气查询");
					}
					else if (Args.size() == 2)
					{
						//地点
						std::string position = m_searchTree.getOriginalString(Args[0]);
						std::string code = m_cityCode[position]["adcode"];

						//实况
						if (Args[1] == "a")
						{
							//请求API
							httplib::Client client("https://restapi.amap.com");
							httplib::Params params = {
								{ "key",m_apiKey },
								{ "city",code },
								{ "extensions","base" },
								{ "output","JSON" }
							};

							httplib::Result result = client.Get("/v3/weather/weatherInfo", params, httplib::Headers());

							if (result.error() != httplib::Error::Success && result.error() != httplib::Error::Connection)
							{
								qqbot::Network::sendGroupMessage(groupID, std::format("请求API出现问题，请稍后再试"));
								return;
							}
							else if (!result)
							{
								qqbot::Network::sendGroupMessage(groupID, std::format("发生错误，无法连接API"));
								return;
							}
							else if (result->status != 200)
							{
								qqbot::Network::sendGroupMessage(groupID, std::format("发生错误，API错误码：{}", result->status));
								return;
							}

							qjson::JObject jo = qjson::JParser::fastParse(result->body);

							if (jo["status"].getString() == "1" && jo["infocode"].getString() == "10000")
							{
								qqbot::Network::sendGroupMessage(groupID, std::format(
R"({}省{}的天气：
天气状况：{}
气温：{}
风向：{}风{}级
空气湿度：{}%
数据发布时间：{})", 
									jo["lives"][0]["province"].getString(),
									jo["lives"][0]["city"].getString(),
									jo["lives"][0]["weather"].getString(),
									jo["lives"][0]["temperature"].getString(),
									jo["lives"][0]["winddirection"].getString(),
									jo["lives"][0]["windpower"].getString(),
									jo["lives"][0]["humidity"].getString(),
									jo["lives"][0]["reporttime"].getString()
									));
							}
							else
							{
								qqbot::Network::sendGroupMessage(groupID, std::format("发生错误，无法获取相应的天气信息：{}", jo["info"].getString()));
							}
						}
						//预报
						else if (Args[1] == "b")
						{
							//请求API
							httplib::SSLClient client("https://restapi.amap.com", 443);
							httplib::Params params = {
								{ "key",m_apiKey },
								{ "city",code },
								{ "extensions","base" },
								{ "output","JSON" }
							};
							httplib::Headers headers;
							httplib::Result result = client.Get("/v3/weather/weatherInfo", params, headers,
								[&](const char* data, size_t data_length) {
									//qjson::JObject jo = qjson::JParser::fastParse(std::string(data, data_length));
									//qqbot::Network::sendGroupMessage(groupID, std::string(data, data_length));
									return true;
								});

							if (result.error() != httplib::Error::Success)
							{
								qqbot::Network::sendGroupMessage(groupID, "请求API出现问题，请稍后再试");
								return;
							}
						}
						else
						{
							qqbot::Network::sendGroupMessage(groupID, "参数错误");
						}
					}
					else
					{
						qqbot::Network::sendGroupMessage(groupID, "参数错误");
					}
				},
				"tq 位置 a(实况)/b(预报)",
					"天气查询"
					);

			qqbot::ServerInfo::getPermission().setGroupDefaultPermission("tq", true);
		}

	private:
		static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_cityCode;
		static SearchTree m_searchTree;

		std::string m_apiKey;
	};
}

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> Weather::WeatherPlugin::m_cityCode;
Weather::WeatherPlugin::SearchTree Weather::WeatherPlugin::m_searchTree;