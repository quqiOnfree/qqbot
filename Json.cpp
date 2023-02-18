#include "Json.h"

#include <exception>
#include <cmath>

namespace qjson
{
	JObject::JObject()
		:m_type(JValueType::JNull),
		m_value(new value_t)
	{
		*m_value = 0ll;
	}

	JObject::JObject(const JObject& jo)
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

	JObject::JObject(JObject&& jo) noexcept
		:m_type(jo.m_type)
	{
		m_value = jo.m_value;
		jo.m_value = nullptr;
	}

	JObject::JObject(JValueType jvt)
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

	JObject::JObject(long long value)
		:m_type(JValueType::JInt),
		m_value(new value_t)
	{
		*m_value = value;
	}

	JObject::JObject(long value)
		:m_type(JValueType::JInt),
		m_value(new value_t)
	{
		*m_value = static_cast<long long>(value);
	}

	JObject::JObject(int value)
		:m_type(JValueType::JInt),
		m_value(new value_t)
	{
		*m_value = static_cast<long long>(value);
	}

	JObject::JObject(short value)
		:m_type(JValueType::JInt),
		m_value(new value_t)
	{
		*m_value = static_cast<long long>(value);
	}

	JObject::JObject(bool value)
		:m_type(JValueType::JBool),
		m_value(new value_t)
	{
		*m_value = value;
	}

	JObject::JObject(long double value)
		:m_type(JValueType::JDouble),
		m_value(new value_t)
	{
		*m_value = value;
	}

	JObject::JObject(double value)
		:m_type(JValueType::JDouble),
		m_value(new value_t)
	{
		*m_value = static_cast<long double>(value);
	}

	JObject::JObject(float value)
		:m_type(JValueType::JDouble),
		m_value(new value_t)
	{
		*m_value = static_cast<long double>(value);
	}

	JObject::JObject(const char* data)
		:m_type(JValueType::JString),
		m_value(new value_t)
	{
		*m_value = std::string(data);
	}

	JObject::JObject(const std::string& data)
		:m_type(JValueType::JString),
		m_value(new value_t)
	{
		*m_value = data;
	}

	JObject::JObject(std::string&& data)
		:m_type(JValueType::JString),
		m_value(new value_t)
	{
		*m_value = std::move(data);
	}

	JObject::~JObject()
	{
		if (m_value != nullptr)
			delete m_value;
	}

	JObject::operator long double&() const
	{
		return getDouble();
	}

	JObject::operator double() const
	{
		return getDouble();
	}

	JObject::operator float() const
	{
		return getDouble();
	}

	JObject::operator long long&() const
	{
		return getInt();
	}

	JObject::operator long() const
	{
		return getInt();
	}

	JObject::operator int() const
	{
		return getInt();
	}

	JObject::operator short() const
	{
		return getInt();
	}

	JObject::operator bool&() const
	{
		return getBool();
	}

	JObject::operator std::string&() const
	{
		return getString();
	}

	JObject::operator list_t&() const
	{
		return getList();
	}

	JObject::operator dict_t&() const
	{
		return getDict();
	}

	JObject& JObject::operator =(const JObject& jo)
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

	JObject& JObject::operator =(JObject&& jo) noexcept
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

	bool JObject::operator ==(const JObject& jo)
	{
		if (m_type != jo.m_type)
			return false;
		switch (jo.m_type)
		{
		case JValueType::JNull:
			return true;
		case JValueType::JInt:
			if (getInt() == jo.getInt())
				return true;
			return false;
		case JValueType::JDouble:
			if (getDouble() == jo.getDouble())
				return true;
			return false;
		case JValueType::JBool:
			if (getBool() == jo.getBool())
				return true;
			return false;
		case JValueType::JString:
			if (getString() == jo.getString())
				return true;
			return false;
		case JValueType::JList:
		{
			list_t& local = getList();
			list_t& jolist = jo.getList();
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
			dict_t& local = getDict();
			dict_t& joDict = jo.getDict();
			if (local.empty() ^ joDict.empty())
				return false;
			if (local.size() != joDict.size())
				return false;
			for (auto i = local.begin(); i != local.end(); i++)
			{
				if (joDict.find(i->first) == joDict.end())
					return false;
				else if (!(i->second == joDict[i->first]))
					return false;
			}
			return true;
		}

		default:
			return false;
		}
	}

	bool JObject::operator ==(JValueType type)
	{
		if (m_type == type)
			return true;
		return false;
	}

	JObject& JObject::operator[](size_t itor)
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
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
	}

	JObject& JObject::operator[](int itor)
	{
		return operator[](size_t(itor));
	}

	JObject& JObject::operator[](const char* str)
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
			throw std::exception("The type isn't JDict, 类型不是JDict.");
		}
	}

	void JObject::push_back(const JObject& jo)
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
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
	}

	void JObject::push_back(JObject&& jo)
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
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
	}

	void JObject::pop_back()
	{
		if (m_type == JValueType::JNull)
		{
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
		else if (m_type == JValueType::JList)
		{
			list_t* local = std::get_if<list_t>(m_value);
			if (local->empty())
				throw std::exception("The JList is empty, JList为空.");
			local->pop_back();
		}
		else
		{
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
	}

	bool JObject::hasMember(const std::string& str)
	{
		if (m_type == JValueType::JDict)
		{
			dict_t* local = std::get_if<dict_t>(m_value);
			if (local->find(str) != local->end())
				return true;
			return false;
		}
		throw std::exception("The type isn't JDict, 类型不是JDict.");
	}

	JValueType JObject::getType() const
	{
		return m_type;
	}

	list_t& JObject::getList() const
	{
		if (m_type == JValueType::JList)
		{
			return *std::get_if<list_t>(m_value);
		}
		else
			throw std::exception("The type isn't JList, 类型不是JList.");
	}

	dict_t& JObject::getDict() const
	{
		if (m_type == JValueType::JDict)
		{
			return *std::get_if<dict_t>(m_value);
		}
		else
			throw std::exception("The type isn't JDict, 类型不是JDict.");
	}

	long long& JObject::getInt() const
	{
		if (m_type == JValueType::JInt)
		{
			return *std::get_if<int_t>(m_value);
		}
		else
		{
			throw std::exception("This JObject isn't int, 此JObject不是整形");
		}
	}

	long double& JObject::getDouble() const
	{
		if (m_type == JValueType::JDouble)
		{
			return *std::get_if<double_t>(m_value);
		}
		else
		{
			throw std::exception("This JObject isn't double, 此JObject不是浮点数");
		}
	}

	bool& JObject::getBool() const
	{
		if (m_type == JValueType::JBool)
		{
			return *std::get_if<bool_t>(m_value);
		}
		else
		{
			throw std::exception("This JObject isn't bool, 此JObject不是布尔值");
		}
	}

	std::string& JObject::getString() const
	{
		if (m_type == JValueType::JString)
		{
			return *std::get_if<string_t>(m_value);
		}
		else
		{
			throw std::exception("This JObject isn't string, 此JObject不是字符串");
		}
	}

	JObject JParser::parse(std::string_view data)
	{
		size_t itor = 0;
		return _parse(data, itor);
	}

	u8JObject JParser::u8parse(const std::u8string& data)
	{
		size_t itor = 0;
		return _u8parse(data, itor);
	}

	JObject JParser::fastParse(const std::string_view data)
	{
		static JParser jp;
		size_t itor = 0;
		return jp._parse(data, itor);
	}

	u8JObject JParser::u8fastParse(const std::u8string& data)
	{
		static JParser jp;
		size_t itor = 0;
		return jp._u8parse(data, itor);
	}

	JObject JParser::_parse(std::string_view data, size_t& itor)
	{
		if (data.empty())
			throw std::exception("Lnvalid input");
		skipSpace(data, itor);
		if (data.size() <= itor)
			throw std::exception("Lnvalid input");
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
					throw std::exception("Lnvalid input");
				skipSpace(data, itor);
				localJO[key.c_str()] = _parse(data, itor);
				skipSpace(data, itor);
				if (data[itor] != ',' && data[itor] != '}')
					throw std::exception("Lnvalid input");
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
				throw std::exception("Lnvalid input");
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
					throw std::exception("Lnvalid input");
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
				throw std::exception("Lnvalid input");
		}
		else if (data[itor] == '\"')
		{
			return getString(data, itor);
		}
		else if (data[itor] == 'n')
		{
			return getNull(data, itor);
		}
		else if (data[itor] == 't' || data[itor] == 'f')
		{
			return getBool(data, itor);
		}
		else if ((data[itor] >= '0' && data[itor] <= '9') || data[itor] == '-')
		{
			return getNumber(data, itor);
		}
		else
			throw std::exception("Lnvalid input");
	}

	u8JObject JParser::_u8parse(const std::u8string& data, size_t& itor)
	{
		if (data.empty())
			throw std::exception("Lnvalid input");
		skipSpace(data, itor);
		if (data.size() <= itor)
			throw std::exception("Lnvalid input");
		if (data[itor] == '{')
		{
			u8JObject localJO(JValueType::JDict);
			itor++;
			while (itor < data.size() && data[itor] != u8'}')
			{
				skipSpace(data, itor);
				if (data[itor] == u8'}')
					return localJO;
				std::u8string key(getString(data, itor));
				skipSpace(data, itor);
				if (data[itor] == ':')
					itor++;
				else
					throw std::exception("Lnvalid input");
				skipSpace(data, itor);
				localJO[key.c_str()] = _u8parse(data, itor);
				skipSpace(data, itor);
				if (data[itor] != u8',' && data[itor] != u8'}')
					throw std::exception("Lnvalid input");
				else if (data[itor] == u8'}')
				{
					itor++;
					return localJO;
				}
				itor++;
				skipSpace(data, itor);
			}
			if (data[itor] == u8'}')
				return localJO;
			else
				throw std::exception("Lnvalid input");
		}
		else if (data[itor] == u8'[')
		{
			u8JObject localJO(JValueType::JList);
			itor++;
			while (itor < data.size() && data[itor] != u8']')
			{
				skipSpace(data, itor);
				if (data[itor] == u8']')
					return localJO;
				localJO.push_back(_u8parse(data, itor));
				skipSpace(data, itor);
				if (data[itor] != u8',' && data[itor] != u8']')
					throw std::exception("Lnvalid input");
				else if (data[itor] == u8']')
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
				throw std::exception("Lnvalid input");
		}
		else if (data[itor] == u8'\"')
		{
			return getString(data, itor);
		}
		else if (data[itor] == u8'n')
		{
			return getNull(data, itor);
		}
		else if (data[itor] == u8't' || data[itor] == u8'f')
		{
			return getBool(data, itor);
		}
		else if ((data[itor] >= u8'0' && data[itor] <= u8'9') || data[itor] == u8'-')
		{
			return getNumber(data, itor);
		}
		else
			throw std::exception("Lnvalid input");
	}

	void JParser::skipSpace(const std::u8string& data, size_t& itor)
	{
		while (itor < data.size() && (data[itor] == u8' ' || data[itor] == u8'\t' || data[itor] == u8'\n'))
		{
			itor++;
		}
	}

	void JParser::skipSpace(std::string_view data, size_t& itor)
	{
		while (itor < data.size() && (data[itor] == ' ' || data[itor] == '\t' || data[itor] == '\n'))
		{
			itor++;
		}
	}

	std::string JParser::getString(std::string_view data, size_t& itor)
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
					default:
						throw std::exception("Lnvalid string");
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
				throw std::exception("Lnvalid input");
			itor++;
			return std::move(str);
		}
		else
			throw std::exception("Lnvalid input");
	}

	std::u8string JParser::getString(const std::u8string& data, size_t& itor)
	{
		if (data[itor] == u8'\"')
		{
			std::u8string str;
			itor++;
			while (itor < data.size() && data[itor] != u8'\"')
			{
				if (data[itor] == u8'\\')
				{
					itor++;
					switch (data[itor])
					{
					case u8'n':
						str += u8'\n';
						break;
					case u8'b':
						str += u8"\b";
						break;
					case u8'f':
						str += u8"\f";
						break;
					case u8'r':
						str += u8"\r";
						break;
					case u8't':
						str += u8"\t";
						break;
					case u8'\\':
						str += u8"\\";
						break;
					case u8'\"':
						str += u8"\"";
						break;
					default:
						throw std::exception("Lnvalid string");
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
				throw std::exception("Lnvalid input");
			itor++;
			return std::move(str);
		}
		else
			throw std::exception("Lnvalid input");
	}

	JObject JParser::getNumber(std::string_view data, size_t& itor)
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
					throw std::exception("Lnvalid input");
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

	u8JObject JParser::getNumber(const std::u8string& data, size_t& itor)
	{
		bool isDouble = false;
		bool firstNum = false;
		bool isNegative = false;
		if (data[itor] == u8'-')
		{
			isNegative = true;
			itor++;
		}
		size_t count = 0;
		size_t start = itor;

		while (itor < data.size() && ((data[itor] >= u8'0' && data[itor] <= u8'9') || data[itor] == u8'.'))
		{
			if (!firstNum && data[itor] >= u8'0' && data[itor] <= u8'9')
			{
				firstNum = true;
			}
			else if (isDouble)
			{
				count++;
			}
			else if (data[itor] == u8'.')
			{
				if (!firstNum)
					throw std::exception("Lnvalid input");
				isDouble = true;
				itor++;
				continue;
			}
			itor++;
		}

		if (isDouble)
		{
			long double number = data[itor - 1] - u8'0';
			size_t single = 10;
			for (long long i = itor - 2; i >= static_cast<long long>(start); --i, single *= 10)
			{
				if (data[i] == '.')
					continue;
				number += single * (data[i] - u8'0');
			}
			if (isNegative)
				number *= -1;
			return number / std::pow(10, count);
		}
		else
		{
			long long number = data[itor - 1] - u8'0';
			size_t single = 10;
			for (long long i = itor - 2; i >= static_cast<long long>(start); --i, single *= 10)
			{
				number += single * (data[i] - u8'0');
			}
			if (isNegative)
				number *= -1;
			return number;
		}
	}

	JObject JParser::getBool(std::string_view data, size_t& itor)
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
		throw std::exception("Lnvalid input");
	}

	u8JObject JParser::getBool(const std::u8string& data, size_t& itor)
	{
		if (data.size() >= itor + 4 && data[itor] == u8't' && data[itor + 1] == u8'r' && data[itor + 2] == 'u' && data[itor + 3] == u8'e')
		{
			itor += 4;
			return true;
		}
		else if (data.size() >= itor + 5 && data[itor] == u8'f' && data[itor + 1] == u8'a' && data[itor + 2] == u8'l' && data[itor + 3] == u8's' && data[itor + 4] == u8'e')
		{
			itor += 5;
			return false;
		}
		throw std::exception("Lnvalid input");
	}

	JObject JParser::getNull(std::string_view data, size_t& itor)
	{
		if (data.size() >= itor + 4 && data[itor] == 'n' && data[itor + 1] == 'u' && data[itor + 2] == 'l' && data[itor + 3] == 'l')
		{
			itor += 4;
			return JObject();
		}
		throw std::exception("Lnvalid input");
	}

	u8JObject JParser::getNull(const std::u8string& data, size_t& itor)
	{
		if (data.size() >= itor + 4 && data[itor] == u8'n' && data[itor + 1] == u8'u' && data[itor + 2] == u8'l' && data[itor + 3] == u8'l')
		{
			itor += 4;
			return u8JObject();
		}
		throw std::exception("Lnvalid input");
	}

	std::string JWriter::write(const JObject& jo)
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
					throw std::exception("Lnvalid string");
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
			list_t& list = jo.getList();
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
			break;
		}

		case JValueType::JDict:
		{
			dict_t& dict = jo.getDict();
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
			break;
		}

		default:
			break;
		}

		return std::move(str);
	}

	std::string  JWriter::formatWrite(const JObject& jo, size_t n)
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
					throw std::exception("Lnvalid string");
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
			list_t& list = jo.getList();
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
			dict_t& dict = jo.getDict();
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

	std::u8string JWriter::u8write(const u8JObject& jo)
	{
		std::u8string str;

		switch (jo.getType())
		{
		case JValueType::JNull:
			str += u8"null";
			break;

		case JValueType::JInt:
		{
			auto local = std::to_string(jo.getInt());
			str += std::u8string(local.begin(), local.end());
			break;
		}
		case JValueType::JDouble:
		{
			auto local = std::to_string(jo.getDouble());
			str += std::u8string(local.begin(), local.end());
			break;
		}
		case JValueType::JBool:
			if (jo.getBool())
			{
				str += u8"true";
				break;
			}
			str += u8"false";
			break;

		case JValueType::JString:
		{
			std::u8string localString(jo.getString());
			str += '\"';
			for (auto i = localString.begin(); i != localString.end(); ++i)
			{
				switch (*i)
				{
				case 0:
					throw std::exception("Lnvalid string");
				case u8'\n':
					str += u8"\\n";
					break;
				case u8'\b':
					str += u8"\\b";
					break;
				case u8'\f':
					str += u8"\\f";
					break;
				case u8'\r':
					str += u8"\\r";
					break;
				case u8'\t':
					str += u8"\\t";
					break;
				case u8'\\':
					str += u8"\\\\";
					break;
				case u8'\"':
					str += u8"\\\"";
					break;
				default:
					str += *i;
					break;
				}
			}
			str += u8'\"';
		}
		break;

		case JValueType::JList:
		{
			u8list_t& list = jo.getList();
			str += u8'[';
			for (auto itor = list.begin(); itor != list.end(); itor++)
			{
				str += u8write(*itor);
				if (itor + 1 != list.end())
				{
					str += u8',';
				}
			}
			str += u8']';
			break;
		}

		case JValueType::JDict:
		{
			u8dict_t& dict = jo.getDict();
			str += u8'{';
			for (auto itor = dict.begin(), itor2 = dict.begin(); itor != dict.end(); itor++)
			{
				str += u8'\"' + itor->first + u8"\":" + u8write(itor->second);
				itor2 = itor;
				if (++itor2 != dict.end())
				{
					str += u8',';
				}
			}
			str += u8'}';
			break;
		}

		default:
			break;
		}

		return std::move(str);
	}

	std::u8string JWriter::u8formatWrite(const u8JObject& jo, size_t n)
	{
		std::u8string str;

		switch (jo.getType())
		{
		case JValueType::JNull:
			str += u8"null";
			break;

		case JValueType::JInt:
		{
			auto local = std::to_string(jo.getInt());
			str += std::u8string(local.begin(), local.end());
			break;
		}
		case JValueType::JDouble:
		{
			auto local = std::to_string(jo.getDouble());
			str += std::u8string(local.begin(), local.end());
			break;
		}
		case JValueType::JBool:
			if (jo.getBool())
			{
				str += u8"true";
				break;
			}
			str += u8"false";
			break;

		case JValueType::JString:
		{
			std::u8string localString(jo.getString());
			str += '\"';
			for (auto i = localString.begin(); i != localString.end(); ++i)
			{
				switch (*i)
				{
				case 0:
					throw std::exception("Lnvalid string");
				case u8'\n':
					str += u8"\\n";
					break;
				case u8'\b':
					str += u8"\\b";
					break;
				case u8'\f':
					str += u8"\\f";
					break;
				case u8'\r':
					str += u8"\\r";
					break;
				case u8'\t':
					str += u8"\\t";
					break;
				case u8'\\':
					str += u8"\\\\";
					break;
				case u8'\"':
					str += u8"\\\"";
					break;
				default:
					str += *i;
					break;
				}
			}
			str += u8'\"';
		}
		break;

		case JValueType::JList:
		{
			u8list_t& list = jo.getList();
			str += u8"[\n";
			for (auto itor = list.begin(); itor != list.end(); itor++)
			{
				for (size_t i = 0; i < n; i++)
				{
					str += u8"    ";
				}
				str += u8formatWrite(*itor, n + 1);
				if (itor + 1 != list.end())
				{
					str += u8",\n";
				}
			}
			str += u8'\n';
			for (size_t i = 0; i < n - 1; i++)
			{
				str += u8"    ";
			}
			str += u8"]";
			break;
		}

		case JValueType::JDict:
		{
			u8dict_t& dict = jo.getDict();
			str += u8"{\n";
			for (auto itor = dict.begin(), itor2 = dict.begin(); itor != dict.end(); itor++)
			{
				for (size_t i = 0; i < n; i++)
				{
					str += u8"    ";
				}
				str += u8'\"' + itor->first + u8"\": " + u8formatWrite(itor->second, n + 1);
				itor2 = itor;
				if (++itor2 != dict.end())
				{
					str += u8",\n";
				}
			}
			str += u8'\n';
			for (size_t i = 0; i < n - 1; i++)
			{
				str += u8"    ";
			}
			str += u8"}";
			break;
		}

		default:
			break;
		}

		return std::move(str);
	}

	std::string  JWriter::fastWrite(const JObject& jo)
	{
		static JWriter jw;
		return std::move(jw.write(jo) + '\n');
	}

	std::string  JWriter::fastFormatWrite(const JObject& jo)
	{
		static JWriter jw;
		return std::move(jw.formatWrite(jo) + '\n');
	}

	std::u8string JWriter::u8fastWrite(const u8JObject& jo)
	{
		static JWriter jw;
		return std::move(jw.u8write(jo) + u8'\n');
	}

	std::u8string JWriter::u8fastFormatWrite(const u8JObject& jo)
	{
		static JWriter jw;
		return std::move(jw.u8formatWrite(jo) + u8'\n');
	}

	u8JObject::u8JObject()
		:m_type(JValueType::JNull),
		m_value(new u8value_t)
	{
		*m_value = 0ll;
	}

	u8JObject::u8JObject(const u8JObject& jo)
		:m_type(jo.m_type),
		m_value(new u8value_t)
	{
		switch (jo.m_type)
		{
		case JValueType::JString:
			*m_value = *std::get_if<u8string_t>(jo.m_value);
			break;
		case JValueType::JList:
			*m_value = *std::get_if<u8list_t>(jo.m_value);
			break;
		case JValueType::JDict:
			*m_value = *std::get_if<u8dict_t>(jo.m_value);
			break;
		default:
			*m_value = *jo.m_value;
			break;
		}
	}

	u8JObject::u8JObject(u8JObject&& jo) noexcept
		:m_type(jo.m_type)
	{
		m_value = jo.m_value;
		jo.m_value = nullptr;
	}

	u8JObject::u8JObject(JValueType jvt)
		:m_type(jvt),
		m_value(new u8value_t)
	{
		switch (jvt)
		{
		case qjson::JValueType::JNull:
			*m_value = u8null_t();
			break;
		case qjson::JValueType::JInt:
			*m_value = u8int_t();
			break;
		case qjson::JValueType::JDouble:
			*m_value = u8double_t();
			break;
		case qjson::JValueType::JBool:
			*m_value = u8bool_t();
			break;
		case qjson::JValueType::JString:
			*m_value = u8string_t();
			break;
		case qjson::JValueType::JList:
			*m_value = u8list_t();
			break;
		case qjson::JValueType::JDict:
			*m_value = u8dict_t();
			break;
		default:
			break;
		}
	}

	u8JObject::u8JObject(long long value)
		:m_type(JValueType::JInt),
		m_value(new u8value_t)
	{
		*m_value = value;
	}

	u8JObject::u8JObject(long value)
		:m_type(JValueType::JInt),
		m_value(new u8value_t)
	{
		*m_value = static_cast<long long>(value);
	}

	u8JObject::u8JObject(int value)
		:m_type(JValueType::JInt),
		m_value(new u8value_t)
	{
		*m_value = static_cast<long long>(value);
	}

	u8JObject::u8JObject(short value)
		:m_type(JValueType::JInt),
		m_value(new u8value_t)
	{
		*m_value = static_cast<long long>(value);
	}

	u8JObject::u8JObject(bool value)
		:m_type(JValueType::JBool),
		m_value(new u8value_t)
	{
		*m_value = value;
	}

	u8JObject::u8JObject(long double value)
		:m_type(JValueType::JDouble),
		m_value(new u8value_t)
	{
		*m_value = value;
	}

	u8JObject::u8JObject(double value)
		:m_type(JValueType::JDouble),
		m_value(new u8value_t)
	{
		*m_value = static_cast<long double>(value);
	}

	u8JObject::u8JObject(float value)
		:m_type(JValueType::JDouble),
		m_value(new u8value_t)
	{
		*m_value = static_cast<long double>(value);
	}

	u8JObject::u8JObject(const char8_t* data)
		:m_type(JValueType::JString),
		m_value(new u8value_t)
	{
		*m_value = std::u8string(data);
	}

	u8JObject::u8JObject(const std::u8string& data)
		:m_type(JValueType::JString),
		m_value(new u8value_t)
	{
		*m_value = data;
	}

	u8JObject::u8JObject(std::u8string&& data)
		:m_type(JValueType::JString),
		m_value(new u8value_t)
	{
		*m_value = std::move(data);
	}

	u8JObject::~u8JObject()
	{
		if (m_value != nullptr)
			delete m_value;
	}

	u8JObject::operator long double& () const
	{
		return getDouble();
	}

	u8JObject::operator double() const
	{
		return getDouble();
	}

	u8JObject::operator float() const
	{
		return getDouble();
	}

	u8JObject::operator long long& () const
	{
		return getInt();
	}

	u8JObject::operator long() const
	{
		return getInt();
	}

	u8JObject::operator int() const
	{
		return getInt();
	}

	u8JObject::operator short() const
	{
		return getInt();
	}

	u8JObject::operator bool& () const
	{
		return getBool();
	}

	u8JObject::operator std::u8string& () const
	{
		return getString();
	}

	u8JObject::operator u8list_t& () const
	{
		return getList();
	}

	u8JObject::operator u8dict_t& () const
	{
		return getDict();
	}

	u8JObject& u8JObject::operator =(const u8JObject& jo)
	{
		if (this == &jo)
			return *this;

		m_type = jo.m_type;
		switch (jo.m_type)
		{
		case JValueType::JString:
			*m_value = *std::get_if<u8string_t>(jo.m_value);
			break;
		case JValueType::JList:
			*m_value = *std::get_if<u8list_t>(jo.m_value);
			break;
		case JValueType::JDict:
			*m_value = *std::get_if<u8dict_t>(jo.m_value);
			break;
		default:
			*m_value = *jo.m_value;
			break;
		}

		return *this;
	}

	u8JObject& u8JObject::operator =(u8JObject&& jo) noexcept
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

	bool u8JObject::operator ==(const u8JObject& jo)
	{
		if (m_type != jo.m_type)
			return false;
		switch (jo.m_type)
		{
		case JValueType::JNull:
			return true;
		case JValueType::JInt:
			if (getInt() == jo.getInt())
				return true;
			return false;
		case JValueType::JDouble:
			if (getDouble() == jo.getDouble())
				return true;
			return false;
		case JValueType::JBool:
			if (getBool() == jo.getBool())
				return true;
			return false;
		case JValueType::JString:
			if (getString() == jo.getString())
				return true;
			return false;
		case JValueType::JList:
		{
			u8list_t& local = getList();
			u8list_t& jolist = jo.getList();
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
			u8dict_t& local = getDict();
			u8dict_t& joDict = jo.getDict();
			if (local.empty() ^ joDict.empty())
				return false;
			if (local.size() != joDict.size())
				return false;
			for (auto i = local.begin(); i != local.end(); i++)
			{
				if (joDict.find(i->first) == joDict.end())
					return false;
				else if (!(i->second == joDict[i->first]))
					return false;
			}
			return true;
		}

		default:
			return false;
		}
	}

	bool u8JObject::operator ==(JValueType type)
	{
		if (m_type == type)
			return true;
		return false;
	}

	u8JObject& u8JObject::operator[](size_t itor)
	{
		if (m_type == JValueType::JNull)
		{
			m_type = JValueType::JList;
			*m_value = u8list_t();
			std::get_if<u8list_t>(m_value)->resize(itor + 1);
			return (*std::get_if<u8list_t>(m_value))[itor];
		}
		else if (m_type == JValueType::JList)
		{
			u8list_t* local = std::get_if<u8list_t>(m_value);
			if (itor >= local->size())
				local->resize(itor + 1);
			return (*local)[itor];
		}
		else
		{
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
	}

	u8JObject& u8JObject::operator[](int itor)
	{
		return operator[](size_t(itor));
	}

	u8JObject& u8JObject::operator[](const char8_t* str)
	{
		if (m_type == JValueType::JNull)
		{
			m_type = JValueType::JDict;
			*m_value = u8dict_t();
			return (*std::get_if<u8dict_t>(m_value))[str];
		}
		else if (m_type == JValueType::JDict)
		{
			return (*std::get_if<u8dict_t>(m_value))[str];
		}
		else
		{
			throw std::exception("The type isn't JDict, 类型不是JDict.");
		}
	}

	void u8JObject::push_back(const u8JObject& jo)
	{
		if (m_type == JValueType::JNull)
		{
			m_type = JValueType::JList;
			*m_value = u8list_t();
			std::get_if<u8list_t>(m_value)->push_back(jo);
		}
		else if (m_type == JValueType::JList)
		{
			std::get_if<u8list_t>(m_value)->push_back(jo);
		}
		else
		{
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
	}

	void u8JObject::push_back(u8JObject&& jo)
	{
		if (m_type == JValueType::JNull)
		{
			m_type = JValueType::JList;
			*m_value = u8list_t();
			std::get_if<u8list_t>(m_value)->push_back(std::move(jo));
		}
		else if (m_type == JValueType::JList)
		{
			std::get_if<u8list_t>(m_value)->push_back(std::move(jo));
		}
		else
		{
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
	}

	void u8JObject::pop_back()
	{
		if (m_type == JValueType::JNull)
		{
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
		else if (m_type == JValueType::JList)
		{
			u8list_t* local = std::get_if<u8list_t>(m_value);
			if (local->empty())
				throw std::exception("The JList is empty, JList为空.");
			local->pop_back();
		}
		else
		{
			throw std::exception("The type isn't JList, 类型不是JList.");
		}
	}

	bool u8JObject::hasMember(const std::u8string& str)
	{
		if (m_type == JValueType::JDict)
		{
			u8dict_t* local = std::get_if<u8dict_t>(m_value);
			if (local->find(str) != local->end())
				return true;
			return false;
		}
		throw std::exception("The type isn't JDict, 类型不是JDict.");
	}

	JValueType u8JObject::getType() const
	{
		return m_type;
	}

	u8list_t& u8JObject::getList() const
	{
		if (m_type == JValueType::JList)
		{
			return *std::get_if<u8list_t>(m_value);
		}
		else
			throw std::exception("The type isn't JList, 类型不是JList.");
	}

	u8dict_t& u8JObject::getDict() const
	{
		if (m_type == JValueType::JDict)
		{
			return *std::get_if<u8dict_t>(m_value);
		}
		else
			throw std::exception("The type isn't JDict, 类型不是JDict.");
	}

	long long& u8JObject::getInt() const
	{
		if (m_type == JValueType::JInt)
		{
			return *std::get_if<u8int_t>(m_value);
		}
		else
		{
			throw std::exception("This JObject isn't int, 此JObject不是整形");
		}
	}

	long double& u8JObject::getDouble() const
	{
		if (m_type == JValueType::JDouble)
		{
			return *std::get_if<u8double_t>(m_value);
		}
		else
		{
			throw std::exception("This JObject isn't double, 此JObject不是浮点数");
		}
	}

	bool& u8JObject::getBool() const
	{
		if (m_type == JValueType::JBool)
		{
			return *std::get_if<u8bool_t>(m_value);
		}
		else
		{
			throw std::exception("This JObject isn't bool, 此JObject不是布尔值");
		}
	}

	std::u8string& u8JObject::getString() const
	{
		if (m_type == JValueType::JString)
		{
			return *std::get_if<u8string_t>(m_value);
		}
		else
		{
			throw std::exception("This JObject isn't string, 此JObject不是字符串");
		}
	}
}
