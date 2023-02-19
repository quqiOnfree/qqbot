#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <string_view>
#include <fstream>

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
		JObject();
		JObject(const JObject& jo);
		JObject(JObject&& jo) noexcept;
		JObject(JValueType jvt);
		JObject(long long value);
		JObject(long value);
		JObject(int value);
		JObject(short value);
		JObject(bool value);
		JObject(long double value);
		JObject(double value);
		JObject(float value);
		JObject(const char* data);
		JObject(const std::string& data);
		JObject(std::string&& data);
		~JObject();

		JObject& operator =(const JObject& jo);
		JObject& operator =(JObject&& jo) noexcept;
		bool operator ==(const JObject& jo);
		bool operator ==(JValueType type);
		JObject& operator[](size_t itor);
		JObject& operator[](int itor);
		JObject& operator[](const char* str);

		void push_back(const JObject& jo);
		void push_back(JObject&& jo);
		void pop_back();
		bool hasMember(const std::string& str);
		JValueType getType() const;
		list_t& getList() const;
		dict_t& getDict() const;
		long long& getInt() const;
		long double& getDouble() const;
		bool& getBool() const;
		std::string& getString() const;

	public:
		using value_t = std::variant<int_t, bool_t, double_t, string_t, list_t, dict_t>;
	private:
		value_t* m_value;
		JValueType m_type;
	};

	class JParser
	{
	public:
		JParser() = default;

		JObject parse(std::string_view data);
		static JObject fastParse(std::ifstream& infile);
		static JObject fastParse(const std::string_view data);
	protected:
		JObject _parse(std::string_view data, size_t& itor);
		void skipSpace(std::string_view data, size_t& itor);
		std::string getString(std::string_view data, size_t& itor);
		JObject getNumber(std::string_view data, size_t& itor);
		JObject getBool(std::string_view data, size_t& itor);
		JObject getNull(std::string_view data, size_t& itor);
	};

	class JWriter
	{
	public:
		JWriter() = default;
		~JWriter() = default;

		std::string write(const JObject& jo);
		std::string formatWrite(const JObject& jo, size_t n = 1);

		static std::string fastWrite(const JObject& jo);
		static std::string fastFormatWrite(const JObject& jo);
	};
}
