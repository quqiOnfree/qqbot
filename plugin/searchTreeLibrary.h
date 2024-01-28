#pragma once

#include <unordered_map>
#include <set>
#include <string>
#include <string_view>
#include <stdexcept>
#include <vector>
#include <queue>

namespace SearchTreeLibrary
{
	//字符串关键字搜索树
	class SearchTree
	{
	public:
		SearchTree() = default;
		~SearchTree() = default;

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
				throw std::logic_error("data is empty");

			// 字符集
			std::set<size_t> locSet;
			{
				size_t i = 0;
				while (data.size() > i && m_tree.find(data[i++]) == m_tree.end()) {}
				auto itor = m_tree.find(data[i - 1]);
				if (itor == m_tree.end())
					throw std::logic_error("can't find this question");
				locSet = itor->second;
			}

			for (auto i = ++data.begin(); i != data.end(); i++)
			{
				auto itor = m_tree.find(*i);
				if (itor == m_tree.end()) continue;
				locSet.insert(itor->second.begin(), itor->second.end());
			}

			// 获取最小值
			std::priority_queue<std::pair<double, std::string>,
				std::vector<std::pair<double, std::string>>,
				std::greater<std::pair<int, std::string>>> buffer;
			for (auto i = locSet.begin(); i != locSet.end(); i++)
			{
				const auto& str = m_strings[*i];

				int sum = 1;
				int result = 0;
				long long itor = 0;
				for (int j = 1; j <= str.size(); j++)
				{
					for (int k = 0; k < str.size(); k += j)
					{
						std::string tempstr = str.substr(k, j);
						long long tempitor = kmp(data, tempstr);
						itor = std::max(tempitor, itor);
						if (tempitor != -1) result++;
						sum++;
					}
				}
				buffer.emplace(std::pair<double, std::string>{sum * 1.0 / result - itor, str});
			}

			return buffer.top().second;
		}

		static int minDistance(const std::string& word1, const std::string& word2) noexcept
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

		static long long kmp(const std::string& match_string, const std::string& pattern) noexcept
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
}
