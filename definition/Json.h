#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <string_view>
#include <fstream>
#include <sstream>

namespace qjson
{
	enum class JValueType
	{
		JNull,
		JInt,
		JDouble,
		JBool,
		JString,
		JList,
		JDict
	};

	class JObject;

	using null_t = bool;
	using int_t = long long;
	using bool_t = bool;
	using double_t = long double;
	using string_t = std::string;
	using list_t = std::vector<JObject>;
	using dict_t = std::unordered_map<std::string, JObject>;

	class JObject
	{
	public:
		JObject()
			:m_type(JValueType::JNull),
			m_value(new value_t)
		{
			*m_value = 0ll;
		}

		JObject(const JObject& jo)
			:m_type(jo.m_type),
			m_value(new value_t)
		{
			switch (jo.m_type)
			{
			case JValueType::JString:
				*m_value = *std::get_if<string_t>(jo.m_value);
				break;
			case JValueType::JList:
				*m_value = *std::get_if<list_t>(jo.m_value);
				break;
			case JValueType::JDict:
				*m_value = *std::get_if<dict_t>(jo.m_value);
				break;
			default:
				*m_value = *jo.m_value;
				break;
			}
		}

		JObject(JObject&& jo) noexcept
			:m_type(jo.m_type)
		{
			m_value = jo.m_value;
			jo.m_value = nullptr;
		}
		JObject(JValueType jvt)
			:m_type(jvt),
			m_value(new value_t)
		{
			switch (jvt)
			{
			case qjson::JValueType::JNull:
				*m_value = null_t();
				break;
			case qjson::JValueType::JInt:
				*m_value = int_t();
				break;
			case qjson::JValueType::JDouble:
				*m_value = double_t();
				break;
			case qjson::JValueType::JBool:
				*m_value = bool_t();
				break;
			case qjson::JValueType::JString:
				*m_value = string_t();
				break;
			case qjson::JValueType::JList:
				*m_value = list_t();
				break;
			case qjson::JValueType::JDict:
				*m_value = dict_t();
				break;
			default:
				break;
			}
		}

		JObject(long long value)
			:m_type(JValueType::JInt),
			m_value(new value_t)
		{
			*m_value = value;
		}

		JObject(long value)
			:m_type(JValueType::JInt),
			m_value(new value_t)
		{
			*m_value = static_cast<long long>(value);
		}

		JObject(int value)
			:m_type(JValueType::JInt),
			m_value(new value_t)
		{
			*m_value = static_cast<long long>(value);
		}

		JObject(short value)
			:m_type(JValueType::JInt),
			m_value(new value_t)
		{
			*m_value = static_cast<long long>(value);
		}

		JObject(bool value)
			:m_type(JValueType::JBool),
			m_value(new value_t)
		{
			*m_value = value;
		}

		JObject(long double value)
			:m_type(JValueType::JDouble),
			m_value(new value_t)
		{
			*m_value = value;
		}

		JObject(double value)
			:m_type(JValueType::JDouble),
			m_value(new value_t)
		{
			*m_value = static_cast<long double>(value);
		}

		JObject(float value)
			:m_type(JValueType::JDouble),
			m_value(new value_t)
		{
			*m_value = static_cast<long double>(value);
		}

		JObject(const char* data)
			:m_type(JValueType::JString),
			m_value(new value_t)
		{
			*m_value = std::string(data);
		}

		JObject(const std::string& data)
			:m_type(JValueType::JString),
			m_value(new value_t)
		{
			*m_value = data;
		}

		JObject(std::string&& data)
			:m_type(JValueType::JString),
			m_value(new value_t)
		{
			*m_value = std::move(data);
		}

		~JObject()
		{
			if (m_value != nullptr)
			{
				delete m_value;
			}
		}

		JObject& operator =(const JObject& jo)
		{
			if (this == &jo)
				return *this;

			m_type = jo.m_type;
			switch (jo.m_type)
			{
			case JValueType::JString:
				*m_value = *std::get_if<string_t>(jo.m_value);
				break;
			case JValueType::JList:
				*m_value = *std::get_if<list_t>(jo.m_value);
				break;
			case JValueType::JDict:
				*m_value = *std::get_if<dict_t>(jo.m_value);
				break;
			default:
				*m_value = *jo.m_value;
				break;
			}

			return *this;
		}

		JObject& operator =(JObject&& jo) noexcept
		{
			if (this == &jo)
				return *this;

			if (m_value != nullptr)
				delete m_value;
			m_type = jo.m_type;
			m_value = jo.m_value;
			jo.m_value = nullptr;

			return *this;
		}

		friend bool operator ==(const JObject& joa, const JObject& jo)
		{
			if (joa.m_type != jo.m_type)
				return false;
			switch (jo.m_type)
			{
			case JValueType::JNull:
				return true;
			case JValueType::JInt:
				if (joa.getInt() == jo.getInt())
					return true;
				return false;
			case JValueType::JDouble:
				if (joa.getDouble() == jo.getDouble())
					return true;
				return false;
			case JValueType::JBool:
				if (joa.getBool() == jo.getBool())
					return true;
				return false;
			case JValueType::JString:
				if (joa.getString() == jo.getString())
					return true;
				return false;
			case JValueType::JList:
			{
				const list_t& local = joa.getList();
				const list_t& jolist = jo.getList();
				if (local.empty() ^ jolist.empty())
					return false;
				if (local.size() != jolist.size())
					return false;
				for (size_t i = 0; i < local.size(); i++)
				{
					if (!(local[i] == jolist[i]))
					{
						return false;
					}
				}
				return true;
			}

			case JValueType::JDict:
			{
				const dict_t& local = joa.getDict();
				const dict_t& joDict = jo.getDict();
				if (local.empty() ^ joDict.empty())
					return false;
				if (local.size() != joDict.size())
					return false;
				for (auto i = local.begin(); i != local.end(); i++)
				{
					if (joDict.find(i->first) == joDict.end())
						return false;
					else if (!(i->second == joDict.find(i->first)->second))
						return false;
				}
				return true;
			}

			default:
				return false;
			}
		}

		friend bool operator ==(const JObject& jo, JValueType type)
		{
			if (jo.m_type == type)
				return true;
			return false;
		}

		const JObject& operator[](size_t itor) const
		{
			if (m_type == JValueType::JNull)
			{
				m_type = JValueType::JList;
				*m_value = list_t();
				std::get_if<list_t>(m_value)->resize(itor + 1);
				return (*std::get_if<list_t>(m_value))[itor];
			}
			else if (m_type == JValueType::JList)
			{
				list_t* local = std::get_if<list_t>(m_value);
				if (itor >= local->size())
					local->resize(itor + 1);
				return (*local)[itor];
			}
			else
			{
				throw std::logic_error("The type isn't JList, 类型不是JList.");
			}
		}

		JObject& operator[](size_t itor)
		{
			if (m_type == JValueType::JNull)
			{
				m_type = JValueType::JList;
				*m_value = list_t();
				std::get_if<list_t>(m_value)->resize(itor + 1);
				return (*std::get_if<list_t>(m_value))[itor];
			}
			else if (m_type == JValueType::JList)
			{
				list_t* local = std::get_if<list_t>(m_value);
				if (itor >= local->size())
					local->resize(itor + 1);
				return (*local)[itor];
			}
			else
			{
				throw std::logic_error("The type isn't JList, 类型不是JList.");
			}
		}

		const JObject& operator[](int itor) const
		{
			return operator[](size_t(itor));
		}

		JObject& operator[](int itor)
		{
			return operator[](size_t(itor));
		}

		const JObject& operator[](const char* str) const
		{
			if (m_type == JValueType::JNull)
			{
				m_type = JValueType::JDict;
				*m_value = dict_t();
				return (*std::get_if<dict_t>(m_value))[str];
			}
			else if (m_type == JValueType::JDict)
			{
				return (*std::get_if<dict_t>(m_value))[str];
			}
			else
			{
				throw std::logic_error("The type isn't JDict, 类型不是JDict.");
			}
		}

		JObject& operator[](const char* str)
		{
			if (m_type == JValueType::JNull)
			{
				m_type = JValueType::JDict;
				*m_value = dict_t();
				return (*std::get_if<dict_t>(m_value))[str];
			}
			else if (m_type == JValueType::JDict)
			{
				return (*std::get_if<dict_t>(m_value))[str];
			}
			else
			{
				throw std::logic_error("The type isn't JDict, 类型不是JDict.");
			}
		}

		void push_back(const JObject& jo)
		{
			if (m_type == JValueType::JNull)
			{
				m_type = JValueType::JList;
				*m_value = list_t();
				std::get_if<list_t>(m_value)->push_back(jo);
			}
			else if (m_type == JValueType::JList)
			{
				std::get_if<list_t>(m_value)->push_back(jo);
			}
			else
			{
				throw std::logic_error("The type isn't JList, 类型不是JList.");
			}
		}

		void push_back(JObject&& jo)
		{
			if (m_type == JValueType::JNull)
			{
				m_type = JValueType::JList;
				*m_value = list_t();
				std::get_if<list_t>(m_value)->push_back(std::move(jo));
			}
			else if (m_type == JValueType::JList)
			{
				std::get_if<list_t>(m_value)->push_back(std::move(jo));
			}
			else
			{
				throw std::logic_error("The type isn't JList, 类型不是JList.");
			}
		}

		void pop_back()
		{
			if (m_type == JValueType::JNull)
			{
				throw std::logic_error("The type isn't JList, 类型不是JList.");
			}
			else if (m_type == JValueType::JList)
			{
				list_t* local = std::get_if<list_t>(m_value);
				if (local->empty())
					throw std::logic_error("The JList is empty, JList为空.");
				local->pop_back();
			}
			else
			{
				throw std::logic_error("The type isn't JList, 类型不是JList.");
			}
		}

		bool hasMember(const std::string& str) const
		{
			if (m_type == JValueType::JDict)
			{
				const dict_t* local = std::get_if<dict_t>(m_value);
				if (local->find(str) != local->end())
					return true;
				return false;
			}
			throw std::logic_error("The type isn't JDict, 类型不是JDict.");
		}

		JValueType getType() const
		{
			return m_type;
		}

		const list_t& getList() const
		{
			if (m_type == JValueType::JList)
			{
				return *std::get_if<list_t>(m_value);
			}
			else
				throw std::logic_error("The type isn't JList, 类型不是JList.");
		}

		list_t& getList()
		{
			if (m_type == JValueType::JList)
			{
				return *std::get_if<list_t>(m_value);
			}
			else
				throw std::logic_error("The type isn't JList, 类型不是JList.");
		}

		const dict_t& getDict() const
		{
			if (m_type == JValueType::JDict)
			{
				return *std::get_if<dict_t>(m_value);
			}
			else
				throw std::logic_error("The type isn't JDict, 类型不是JDict.");
		}

		dict_t& getDict()
		{
			if (m_type == JValueType::JDict)
			{
				return *std::get_if<dict_t>(m_value);
			}
			else
				throw std::logic_error("The type isn't JDict, 类型不是JDict.");
		}

		const long long& getInt() const
		{
			if (m_type == JValueType::JInt)
			{
				return *std::get_if<int_t>(m_value);
			}
			else
			{
				throw std::logic_error("This JObject isn't int, 此JObject不是整形");
			}
		}

		long long& getInt()
		{
			if (m_type == JValueType::JInt)
			{
				return *std::get_if<int_t>(m_value);
			}
			else
			{
				throw std::logic_error("This JObject isn't int, 此JObject不是整形");
			}
		}

		const long double& getDouble() const
		{
			if (m_type == JValueType::JDouble)
			{
				return *std::get_if<double_t>(m_value);
			}
			else
			{
				throw std::logic_error("This JObject isn't double, 此JObject不是浮点数");
			}
		}

		long double& getDouble()
		{
			if (m_type == JValueType::JDouble)
			{
				return *std::get_if<double_t>(m_value);
			}
			else
			{
				throw std::logic_error("This JObject isn't double, 此JObject不是浮点数");
			}
		}

		const bool& getBool() const
		{
			if (m_type == JValueType::JBool)
			{
				return *std::get_if<bool_t>(m_value);
			}
			else
			{
				throw std::logic_error("This JObject isn't bool, 此JObject不是布尔值");
			}
		}

		bool& getBool()
		{
			if (m_type == JValueType::JBool)
			{
				return *std::get_if<bool_t>(m_value);
			}
			else
			{
				throw std::logic_error("This JObject isn't bool, 此JObject不是布尔值");
			}
		}

		const std::string& getString() const
		{
			if (m_type == JValueType::JString)
			{
				return *std::get_if<string_t>(m_value);
			}
			else
			{
				throw std::logic_error("This JObject isn't string, 此JObject不是字符串");
			}
		}

		std::string& getString()
		{
			if (m_type == JValueType::JString)
			{
				return *std::get_if<string_t>(m_value);
			}
			else
			{
				throw std::logic_error("This JObject isn't string, 此JObject不是字符串");
			}
		}

	public:
		using value_t = std::variant<int_t, bool_t, double_t, string_t, list_t, dict_t>;
	private:
		mutable value_t* m_value;
		mutable JValueType m_type;
	};

	class JParser
	{
	public:
		JParser() = default;

		JObject parse(std::string_view data)
		{
			size_t itor = 0;
			return std::move(_parse(data, itor));
		}

		static JObject fastParse(std::ifstream& infile)
		{
			if (!infile)
				throw std::runtime_error("can't open the file");

			infile.seekg(0, std::ios_base::end);
			size_t size = infile.tellg();
			infile.seekg(0, std::ios_base::beg);
			std::string buffer;
			buffer.resize(size);
			infile.read(buffer.data(), size);
			infile.close();

			return std::move(JParser::fastParse(buffer));
		}

		static JObject fastParse(const std::string_view data)
		{
			static JParser jp;
			size_t itor = 0;
			return std::move(jp._parse(data, itor));
		}

	protected:
		JObject _parse(std::string_view data, size_t& itor)
		{
			if (data.empty())
				throw std::logic_error("Lnvalid input");
			skipSpace(data, itor);
			if (data.size() <= itor)
				throw std::logic_error("Lnvalid input");
			if (data[itor] == '{')
			{
				JObject localJO(JValueType::JDict);
				itor++;
				while (itor < data.size() && data[itor] != '}')
				{
					skipSpace(data, itor);
					if (data[itor] == '}')
						return localJO;
					std::string key(getString(data, itor));
					skipSpace(data, itor);
					if (data[itor] == ':')
						itor++;
					else
						throw std::logic_error("Lnvalid input");
					skipSpace(data, itor);
					localJO[key.c_str()] = _parse(data, itor);
					skipSpace(data, itor);
					if (data[itor] != ',' && data[itor] != '}')
						throw std::logic_error("Lnvalid input");
					else if (data[itor] == '}')
					{
						itor++;
						return localJO;
					}
					itor++;
					skipSpace(data, itor);
				}
				if (data[itor] == '}')
					return localJO;
				else
					throw std::logic_error("Lnvalid input");
			}
			else if (data[itor] == '[')
			{
				JObject localJO(JValueType::JList);
				itor++;
				while (itor < data.size() && data[itor] != ']')
				{
					skipSpace(data, itor);
					if (data[itor] == ']')
						return localJO;
					localJO.push_back(_parse(data, itor));
					skipSpace(data, itor);
					if (data[itor] != ',' && data[itor] != ']')
						throw std::logic_error("Lnvalid input");
					else if (data[itor] == ']')
					{
						itor++;
						return localJO;
					}
					itor++;
					skipSpace(data, itor);
				}
				if (data[itor] == ']')
					return localJO;
				else
					throw std::logic_error("Lnvalid input");
			}
			else if (data[itor] == '\"')
			{
				return std::move(getString(data, itor));
			}
			else if (data[itor] == 'n')
			{
				return std::move(getNull(data, itor));
			}
			else if (data[itor] == 't' || data[itor] == 'f')
			{
				return std::move(getBool(data, itor));
			}
			else if ((data[itor] >= '0' && data[itor] <= '9') || data[itor] == '-')
			{
				return std::move(getNumber(data, itor));
			}
			else
				throw std::logic_error("Lnvalid input");
		}

		void skipSpace(std::string_view data, size_t& itor)
		{
			auto size = data.size();
			while (itor < size && (data[itor] == ' ' || data[itor] == '\t' || data[itor] == '\n'))
			{
				itor++;
			}
		}

		std::string getString(std::string_view data, size_t& itor)
		{
			if (data[itor] == '\"')
			{
				std::string str;
				itor++;
				while (itor < data.size() && data[itor] != '\"')
				{
					if (data[itor] == '\\')
					{
						itor++;
						switch (data[itor])
						{
						case 'n':
							str += '\n';
							break;
						case 'b':
							str += "\b";
							break;
						case 'f':
							str += "\f";
							break;
						case 'r':
							str += "\r";
							break;
						case 't':
							str += "\t";
							break;
						case '\\':
							str += "\\";
							break;
						case '\"':
							str += "\"";
							break;
						case '/':
							str += "/";
							break;
						case 'u':
							if (itor + 4 < data.size())
							{
								std::stringstream ss;
								std::string strNum = "0x" + std::string(data.substr(itor + 1, 4));
								int nValude = 0;
#ifdef _MSVC_LANG
								//msvc下使用这个
								sscanf_s(strNum.c_str(), "%x", &nValude);
#else
								sscanf(strNum.c_str(), "%x", &nValude);
#endif // _WIN32
								str += static_cast<wchar_t>(nValude);
								itor += 4;
							}
							break;
						default:
							throw std::logic_error("Lnvalid string");
							break;
						}
					}
					else
					{
						str += data[itor];
					}
					itor++;
				}
				if (itor >= data.size())
					throw std::logic_error("Lnvalid input");
				itor++;
				return std::move(str);
			}
			else
				throw std::logic_error("Lnvalid input");
		}

		JObject getNumber(std::string_view data, size_t& itor)
		{
			bool isDouble = false;
			bool firstNum = false;
			bool isNegative = false;
			if (data[itor] == '-')
			{
				isNegative = true;
				itor++;
			}
			size_t count = 0;
			size_t start = itor;

			while (itor < data.size() && ((data[itor] >= '0' && data[itor] <= '9') || data[itor] == '.'))
			{
				if (!firstNum && data[itor] >= '0' && data[itor] <= '9')
				{
					firstNum = true;
				}
				else if (isDouble)
				{
					count++;
				}
				else if (data[itor] == '.')
				{
					if (!firstNum)
						throw std::logic_error("Lnvalid input");
					isDouble = true;
					itor++;
					continue;
				}
				itor++;
			}

			if (isDouble)
			{
				long double number = data[itor - 1] - '0';
				size_t single = 10;
				for (long long i = itor - 2; i >= static_cast<long long>(start); --i, single *= 10)
				{
					if (data[i] == '.')
						continue;
					number += single * (data[i] - '0');
				}
				if (isNegative)
					number *= -1;
				return number / std::pow(10, count);
			}
			else
			{
				long long number = data[itor - 1] - '0';
				size_t single = 10;
				for (long long i = itor - 2; i >= static_cast<long long>(start); --i, single *= 10)
				{
					number += single * (data[i] - '0');
				}
				if (isNegative)
					number *= -1;
				return number;
			}
		}

		JObject getBool(std::string_view data, size_t& itor)
		{
			if (data.size() >= itor + 4 && data[itor] == 't' && data[itor + 1] == 'r' && data[itor + 2] == 'u' && data[itor + 3] == 'e')
			{
				itor += 4;
				return true;
			}
			else if (data.size() >= itor + 5 && data[itor] == 'f' && data[itor + 1] == 'a' && data[itor + 2] == 'l' && data[itor + 3] == 's' && data[itor + 4] == 'e')
			{
				itor += 5;
				return false;
			}
			throw std::logic_error("Lnvalid input");
		}

		JObject getNull(std::string_view data, size_t& itor)
		{
			if (data.size() >= itor + 4 && data[itor] == 'n' && data[itor + 1] == 'u' && data[itor + 2] == 'l' && data[itor + 3] == 'l')
			{
				itor += 4;
				return JObject();
			}
			throw std::logic_error("Lnvalid input");
		}
	};

	class JWriter
	{
	public:
		JWriter() = default;
		~JWriter() = default;

		std::string write(const JObject& jo)
		{
			std::string str;

			switch (jo.getType())
			{
			case JValueType::JNull:
				str += "null";
				break;

			case JValueType::JInt:
				str += std::to_string(jo.getInt());
				break;

			case JValueType::JDouble:
				str += std::to_string(jo.getDouble());
				break;

			case JValueType::JBool:
				if (jo.getBool())
				{
					str += "true";
				}
				else
				{
					str += "false";
				}
				break;

			case JValueType::JString:
			{
				std::string localString(jo.getString());
				if (localString.empty())
				{
					str += "\"\"";
				}
				else
				{
					str += '\"';
					for (auto i = localString.begin(); i != localString.end(); ++i)
					{
						switch (*i)
						{
						case 0:
							throw std::logic_error("Lnvalid string");
						case '\n':
							str += "\\n";
							break;
						case '\b':
							str += "\\b";
							break;
						case '\f':
							str += "\\f";
							break;
						case '\r':
							str += "\\r";
							break;
						case '\t':
							str += "\\t";
							break;
						case '\\':
							str += "\\\\";
							break;
						case '\"':
							str += "\\\"";
							break;
						default:
							str += *i;
							break;
						}
					}
					str += '\"';
				}
			}
			break;

			case JValueType::JList:
			{
				const list_t& list = jo.getList();
				if (list.empty())
				{
					str += "[]";
				}
				else
				{
					str += '[';
					for (auto itor = list.begin(); itor != list.end(); itor++)
					{
						str += write(*itor);
						if (itor + 1 != list.end())
						{
							str += ',';
						}
					}
					str += ']';
				}
				break;
			}

			case JValueType::JDict:
			{
				const dict_t& dict = jo.getDict();
				if (dict.empty())
				{
					str += "{}";
				}
				else
				{
					str += '{';
					for (auto itor = dict.begin(), itor2 = dict.begin(); itor != dict.end(); itor++)
					{
						str += '\"' + itor->first + "\":" + write(itor->second);
						itor2 = itor;
						if (++itor2 != dict.end())
						{
							str += ',';
						}
					}
					str += '}';
				}
				break;
			}

			default:
				break;
			}

			return std::move(str);
		}

		std::string formatWrite(const JObject& jo, size_t n = 1)
		{
			std::string str;

			switch (jo.getType())
			{
			case JValueType::JNull:
				str += "null";
				break;

			case JValueType::JInt:
				str += std::to_string(jo.getInt());
				break;

			case JValueType::JDouble:
				str += std::to_string(jo.getDouble());
				break;

			case JValueType::JBool:
				if (jo.getBool())
				{
					str += "true";
					break;
				}
				str += "false";
				break;

			case JValueType::JString:
			{
				std::string localString(jo.getString());
				str += '\"';
				for (auto i = localString.begin(); i != localString.end(); ++i)
				{
					switch (*i)
					{
					case 0:
						throw std::logic_error("Lnvalid string");
					case '\n':
						str += "\\n";
						break;
					case '\b':
						str += "\\b";
						break;
					case '\f':
						str += "\\f";
						break;
					case '\r':
						str += "\\r";
						break;
					case '\t':
						str += "\\t";
						break;
					case '\\':
						str += "\\\\";
						break;
					case '\"':
						str += "\\\"";
						break;
					default:
						str += *i;
						break;
					}
				}
				str += '\"';
			}
			break;

			case JValueType::JList:
			{
				const list_t& list = jo.getList();
				str += "[\n";
				for (auto itor = list.begin(); itor != list.end(); itor++)
				{
					for (size_t i = 0; i < n; i++)
					{
						str += "    ";
					}
					str += formatWrite(*itor, n + 1);
					if (itor + 1 != list.end())
					{
						str += ",\n";
					}
				}
				str += '\n';
				for (size_t i = 0; i < n - 1; i++)
				{
					str += "    ";
				}
				str += "]";
				break;
			}

			case JValueType::JDict:
			{
				const dict_t& dict = jo.getDict();
				str += "{\n";
				for (auto itor = dict.begin(), itor2 = dict.begin(); itor != dict.end(); itor++)
				{
					for (size_t i = 0; i < n; i++)
					{
						str += "    ";
					}
					str += '\"' + itor->first + "\": " + formatWrite(itor->second, n + 1);
					itor2 = itor;
					if (++itor2 != dict.end())
					{
						str += ",\n";
					}
				}
				str += '\n';
				for (size_t i = 0; i < n - 1; i++)
				{
					str += "    ";
				}
				str += "}";
				break;
			}

			default:
				break;
			}

			return std::move(str);
		}

		static std::string fastWrite(const JObject& jo)
		{
			static JWriter jw;
			return std::move(jw.write(jo) + '\n');
		}

		static std::string fastFormatWrite(const JObject& jo)
		{
			static JWriter jw;
			return std::move(jw.formatWrite(jo) + '\n');
		}
	};
}

#endif // !JSON_HPP
