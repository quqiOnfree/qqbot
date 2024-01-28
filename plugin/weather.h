#pragma once

#include <iostream>
#include <Json.h>
#include <httplib.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <shared_mutex>
#include <queue>

#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"

namespace Weather
{
	class WeatherPlugin : public qqbot::CppPlugin
	{
	public:
		class CitySearchTree
		{
		public:
			CitySearchTree() = default;
			~CitySearchTree() = default;

			// 加入
			void insert(const std::string& data)
			{
				m_strings.push_back(data);
				size_t position = m_strings.size() - 1;
				for (auto i = data.begin(); i != data.end(); i++)
				{
					m_tree[*i].insert(position);
				}
			}

			// 获取模糊搜索的匹配字符串
			std::string getOriginalString(const std::string& data) const
			{
				if (data.empty())
				{
					throw std::logic_error("data is empty");
				}

				bool is_key = kmp(data, "区") != -1 ? true : false;

				std::set<size_t> locSet;
				{
					size_t i = 0;
					while (data.size() > i && m_tree.find(data[i++]) == m_tree.end()) {}
					auto itor = m_tree.find(data[i - 1]);
					if (itor == m_tree.end())
						throw std::logic_error("can't find this question");
					locSet = itor->second;
				}

				for (auto i = data.begin() + 1; i != data.end(); i++)
				{
					auto itor = m_tree.find(*i);
					if (itor == m_tree.end()) continue;
					locSet.insert(itor->second.begin(), itor->second.end());
				}

				std::priority_queue<std::pair<int, std::string>,
					std::vector<std::pair<int, std::string>>,
					std::greater<std::pair<int, std::string>>> buffer;
				for (auto i = locSet.begin(); i != locSet.end(); i++)
				{
					const auto& str = m_strings[*i];
					buffer.emplace(std::pair<int, std::string>{minDistance(data, str), str});
				}

				int min = buffer.top().first;
				std::string restr = buffer.top().second;
				buffer.pop();
				while (!buffer.empty() && (buffer.top().first == min || is_key))
				{
					if (kmp(buffer.top().second, "区") != -1)
					{
						restr = buffer.top().second;
						break;
					}
					buffer.pop();
				}
				return restr;
			}

		protected:
			static int minDistance(const std::string& word1, const std::string& word2)
			{
				int n = (int)word1.length();
				int m = (int)word2.length();
				if (n * m == 0) return n + m;

				std::vector<std::vector<int>> D(n + 1, std::vector<int>(m + 1));

				for (int i = 0; i < n + 1; i++)
				{
					D[i][0] = i;
				}
				for (int j = 0; j < m + 1; j++)
				{
					D[0][j] = j;
				}

				// 计算所有 DP 值
				for (int i = 1; i < n + 1; i++)
				{
					for (int j = 1; j < m + 1; j++)
					{
						int left = D[i - 1][j] + 1;
						int down = D[i][j - 1] + 1;
						int left_down = D[i - 1][j - 1];
						if (word1[i - 1] != word2[j - 1]) left_down += 1;
						D[i][j] = std::min(left, std::min(down, left_down));
					}
				}
				return D[n][m];
			}

			static long long kmp(const std::string& match_string, const std::string& pattern)
			{
				if (match_string.length() < pattern.length()) return -1;

				for (size_t i = 0; i < match_string.size(); i++)
				{
					size_t j = 0;
					for (; j < match_string.size() - i && j < pattern.size(); j++)
					{
						if (match_string[i + j] != pattern[j])
							break;
					}
					if (j == pattern.size())
						return i;
				}
				return -1;
			}

		private:
			// 搜索树本体
			std::unordered_map<char, std::set<size_t>>	m_tree;
			// 匹配字符串
			std::vector<std::string>					m_strings;
		};

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
						qqbot::Network::sendGroupMessage(groupID, "tq 位置 [a(实况)/b(预报)] -天气查询");
						return;
					}
					else if (Args.size() > 2)
					{
						qqbot::Network::sendGroupMessage(groupID, "参数错误");
						return;
					}

					// 地理位置
					std::string position, code;
					{
						//上锁
						std::shared_lock<std::shared_mutex> searchTree_lock(m_searchTree_mutex, std::defer_lock),
							cityCode_lock(m_cityCode_mutex, std::defer_lock);
						std::lock(searchTree_lock, cityCode_lock);

						position = m_searchTree.getOriginalString(Args[0]);
						code = m_cityCode[position]["adcode"];
					}

					if (Args.size() == 1)
					{
						getCurrentWeather(position, code, groupID);
						return;
					}

					//实况
					if (Args[1] == "a")
					{
						getCurrentWeather(position, code, groupID);
						return;
					}
					//预报
					else if (Args[1] == "b")
					{
						getFutureWeather(position, code, groupID);
						return;
					}
					else
					{
						qqbot::Network::sendGroupMessage(groupID, "参数错误");
						return;
					}
				},
				"tq 位置 [a(实况)/b(预报)]",
					"天气查询"
					);

			qqbot::ServerInfo::getPermission().setGroupDefaultPermission("tq", true);
		}

		virtual void onDisable()
		{
			qqbot::ServerInfo::getCommander().removeCommand("tq", true);
		}

	protected:
		void getCurrentWeather(const std::string& positon, const std::string& code, long long groupID)
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
					R"({}-{}的天气：
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

		void getFutureWeather(const std::string& positon, const std::string& code, long long groupID)
		{
			//string按空格分割函数
			auto split = [](const std::string& data, char chr = ' ') {
				std::vector<std::string> dataList;

				long long begin = -1;
				long long i = 0;

				for (; static_cast<size_t>(i) < data.size(); i++)
				{
					if (data[i] == chr)
					{
						if ((i - begin - 1) > 0)
						{
							dataList.push_back(data.substr(begin + 1, i - begin - 1));
						}
						begin = i;
					}
				}
				dataList.push_back(data.substr(begin + 1, i - begin - 1));

				return dataList;
				};

			auto getDayOfWeek = [](unsigned int year, unsigned int month, unsigned int day) {
				int week = 0;
				unsigned int y = 0, c = 0, m = 0, d = 0;
				if (month == 1 || month == 2)
				{
					c = (year - 1) / 100;
					y = (year - 1) % 100;
					m = month + 12;
					d = day;
				}
				else
				{
					c = year / 100;
					y = year % 100;
					m = month;
					d = day;
				}

				week = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1; //蔡勒公式
				week = week >= 0 ? (week % 7) : (week % 7 + 7); //week为负时取模
				if (week == 0) //星期日不作为一周的第一天
				{
					week = 7;
				}

				return week;
				};

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
					"{}-{}的预报天气：\n",
					jo["forecasts"][0]["province"].getString(),
					jo["forecasts"][0]["city"].getString()
				);
				qjson::list_t& list = jo["forecasts"][0]["casts"].getList();

				for (auto i = list.begin(); i != list.end(); i++)
				{
					std::string date = (*i)["date"].getString();
					std::vector<std::string> date_list = split(date, '-');
					int day = getDayOfWeek(std::stoi(date_list[0]), std::stoi(date_list[1]), std::stoi(date_list[2]));
					std::string day_string;

					switch (day)
					{
					case 1:
						day_string = "一";
						break;
					case 2:
						day_string = "二";
						break;
					case 3:
						day_string = "三";
						break;
					case 4:
						day_string = "四";
						break;
					case 5:
						day_string = "五";
						break;
					case 6:
						day_string = "六";
						break;
					case 7:
						day_string = "日";
						break;
					default:
						break;
					}

					outStr += std::format(R"(--------------------
日期：{} 星期{}
天气状况：{}-{}
气温：{}℃-{}℃
风向：{}风{}级-{}风{}级
)",
date, day_string,
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
			}
			else
			{
				qqbot::Network::sendGroupMessage(groupID, std::format("发生错误，无法获取相应的天气信息：{}", jo["info"].getString()));
			}
		}

	private:
		std::unordered_map<std::string,
			std::unordered_map<std::string,
				std::string>>				m_cityCode;
		mutable std::shared_mutex			m_cityCode_mutex;
		CitySearchTree						m_searchTree;
		mutable std::shared_mutex			m_searchTree_mutex;

		std::string							m_apiKey;
	};
}