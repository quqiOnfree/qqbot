#ifndef INI_HPP
#define INI_HPP

#include <unordered_map>
#include <string>
#include <string_view>
#include <fstream>
#include <format>
#include <stdexcept>

namespace qini
{
	class INIParser;
	class INIWriter;

	class INIObject
	{
	public:
		class Section
		{
		public:
			Section(std::unordered_map<std::string, std::string>& section) :
				m_keys(section)
			{
			}

			Section(const Section&) = delete;
			Section(Section&&) = delete;

			Section& operator =(const Section&) = delete;
			Section& operator =(Section&&) = delete;

			std::string& operator [](const std::string& keyName)
			{
				return m_keys[keyName];
			}

			const std::string& operator [](const std::string& keyName) const
			{
				return m_keys[keyName];
			}

		private:
			std::unordered_map<std::string, std::string>& m_keys;
		};

		INIObject() = default;

		INIObject(const INIObject& ob) :
			m_sections(ob.m_sections)
		{
		}

		INIObject(INIObject&& ob) noexcept :
			m_sections(std::move(ob.m_sections))
		{
		}

		~INIObject() = default;

		INIObject& operator =(const INIObject& ob)
		{
			if (this == &ob)
				return *this;

			m_sections = ob.m_sections;
			return *this;
		}

		INIObject& operator =(INIObject&& ob) noexcept
		{
			if (this == &ob)
				return *this;

			m_sections = std::move(ob.m_sections);
			return *this;
		}

		Section operator [](const std::string sectionName)
		{
			return Section(m_sections[sectionName]);
		}

		friend bool operator ==(const INIObject& ia, const INIObject& ib) const
		{
			return ia.m_sections == ib.m_sections;
		}

		friend bool operator !=(const INIObject& ia, const INIObject& ib) const
		{
			return ia.m_sections != ib.m_sections;
		}

	private:
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_sections;

		friend class INIParser;
		friend class INIWriter;
	};

	class INIParser
	{
	public:
		INIParser() = default;
		~INIParser() = default;

		INIObject parse(std::string_view data)
		{
			INIObject localObject;

			std::string localSection;
			for (auto i = data.begin(); i != data.end(); i++)
			{
				if (!skipSpace(i, data))
					break;

				if (*i == '[')
				{
					i++;
					if (!skipSpace(i, data))
						throw std::logic_error("Lnvalided input.");

					localSection = getString(i, data);

					if (!skipSpace(i, data))
						throw std::logic_error("Lnvalided input.");

					if (*i == ']')
						i++;
					else
						throw std::logic_error("Lnvalided input.");
				}
				else if (*i == '=')
				{
					throw std::logic_error("Lnvalided input.");
				}
				else
				{
					if (localSection.empty())
						throw std::logic_error("Lnvalided input.");

					std::string localKey = getString(i, data);

					if (!skipSpace(i, data))
						throw std::logic_error("Lnvalided input.");

					if (*i == '=')
						i++;
					else
						throw std::logic_error("Lnvalided input.");

					if (!skipSpace(i, data))
						throw std::logic_error("Lnvalided input.");

					localObject.m_sections[localSection][localKey] = getString(i, data);
				}
			}

			return localObject;
		}

		static INIObject fastParse(std::string_view data)
		{
			static INIParser parser;
			return parser.parse(data);
		}

		/*INIObject parse(std::ifstream& file)
		{
			INIObject localObject;
		}*/

	protected:
		bool skipSpace(std::string_view::iterator& i, std::string_view data)
		{
			while (i != data.end() && (*i == ' ' || *i == '\n' || *i == '\t' || *i == ';'))
			{
				if (*i == ';')
				{
					for (; i != data.end() && *i != '\n'; i++) {}
				}
				else
				{
					i++;
				}
			}

			return i != data.end();
		}

		std::string getString(std::string_view::iterator& i, std::string_view data)
		{
			std::string localString;
			while (i != data.end() && (*i != ' ' && *i != '\n' && *i != '\t' && *i != '[' && *i != ']' && *i != '=' && *i != ';'))
			{
				localString += *i;
				i++;
			}
			return localString;
		}
	};

	class INIWriter
	{
	public:
		INIWriter() = default;
		~INIWriter() = default;

		std::string write(const INIObject& ob)
		{
			std::string localString;
			for (const auto& [section, keys] : ob.m_sections)
			{
				localString += std::format("[{}]\n", section);
				for (const auto& [key, value] : keys)
				{
					localString += std::format("{}={}\n", key, value);
				}
			}
			return localString;
		}

		bool write(const INIObject& ob, std::ofstream& file)
		{
			if (!file)
				return false;

			file.clear();
			for (const auto& [section, keys] : ob.m_sections)
			{
				file << std::format("[{}]\n", section);
				for (const auto& [key, value] : keys)
				{
					file << std::format("{}={}\n", key, value);
				}
			}
			file << std::endl;

			return true;
		}

		static std::string fastWrite(const INIObject& ob)
		{
			static INIWriter writer;
			return writer.write(ob);
		}

		static bool fastWrite(const INIObject& ob, std::ofstream& file)
		{
			static INIWriter writer;
			return writer.write(ob, file);
		}
	};
}

#endif // !INI_HPP
