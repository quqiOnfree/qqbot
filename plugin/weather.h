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
#include "searchTreeLibrary.h"

namespace Weather
{
	class WeatherPlugin : public qqbot::CppPlugin
	{
	public:
		WeatherPlugin() : m_apiKey("dffc4517b36352b24303ecd2493658c7")
		{
			qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";
			qqbot::CppPlugin::pluginInfo.name = "Weather";
			qqbot::CppPlugin::pluginInfo.version = "0.0.1";
		}

		virtual ~WeatherPlugin() = default;

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
气温：{}℃
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
							httplib::Client client("https://restapi.amap.com");
							httplib::Params params = {
								{ "key",m_apiKey },
								{ "city",code },
								{ "extensions","all" },
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
								std::string outStr = std::format(
									"{}省{}的预报天气：\n",
									jo["forecasts"][0]["province"].getString(),
									jo["forecasts"][0]["city"].getString()
								);
								qjson::list_t& list = jo["forecasts"][0]["casts"].getList();

								for (auto i = list.begin(); i != list.end(); i++)
								{
									outStr += std::format(R"(--------------------
日期：{}
天气状况：{}-{}
气温：{}℃-{}℃
风向：{}风{}级-{}风{}级
)",
										(*i)["date"].getString(),
										(*i)["dayweather"].getString(),
										(*i)["nightweather"].getString(),
										(*i)["daytemp"].getString(),
										(*i)["nighttemp"].getString(),
										(*i)["daywind"].getString(),
										(*i)["daypower"].getString(),
										(*i)["nightwind"].getString(),
										(*i)["nightpower"].getString()
									);
								}

								qqbot::Network::sendGroupMessage(groupID, outStr);

								//qqbot::Network::sendGroupMessage(groupID, );
							}
							else
							{
								qqbot::Network::sendGroupMessage(groupID, std::format("发生错误，无法获取相应的天气信息：{}", jo["info"].getString()));
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

		virtual void onDisable()
		{
			qqbot::ServerInfo::getCommander().removeCommand("tq", true);
		}

	private:
		static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_cityCode;
		static SearchTreeLibrary::SearchTree m_searchTree;

		std::string m_apiKey;
	};
}

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> Weather::WeatherPlugin::m_cityCode;
SearchTreeLibrary::SearchTree Weather::WeatherPlugin::m_searchTree;