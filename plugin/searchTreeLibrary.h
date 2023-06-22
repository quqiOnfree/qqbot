#pragma once

#include <unordered_map>
#include <set>
#include <string>
#include <string_view>

namespace SearchTreeLibrary
{
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

		std::string getOriginalString(const std::string& data) const
		{
			if (data.empty())
			{
				throw std::exception("data is empty");
			}

			std::set<size_t> locSet;
			size_t i = 0;
			while (data.size() > i && m_tree.find(data[i++]) == m_tree.end()) {}
			if (m_tree.find(data[i - 1]) == m_tree.end())
				throw std::exception("can't find this question");
			locSet = m_tree.find(data[i - 1])->second;

			for (auto i = data.begin() + 1; i != data.end(); i++)
			{
				std::set<size_t> locSet_2 = locSet;

				for (auto j = locSet_2.begin(); j != locSet_2.end(); j++)
				{
					if (m_tree.find(*i)->second.find(*j) == m_tree.find(*i)->second.end())
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
}
