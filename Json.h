#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <string_view>

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

		operator long double&() const;
		operator double() const;
		operator float() const;
		operator long long&() const;
		operator long() const;
		operator int() const;
		operator short() const;
		operator bool&() const;
		operator std::string&() const;
		operator list_t&() const;
		operator dict_t&() const;

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
	protected:
		using value_t = std::variant<int_t, bool_t, double_t, string_t, list_t, dict_t>;
	private:
		value_t* m_value;
		JValueType m_type;
	};

	class u8JObject;

	class JParser
	{
	public:
		JParser() = default;

		JObject parse(std::string_view data);
		u8JObject u8parse(const std::u8string& data);
		static JObject fastParse(const std::string_view data);
		static u8JObject u8fastParse(const std::u8string& data);
	protected:
		JObject _parse(std::string_view data, size_t& itor);
		u8JObject _u8parse(const std::u8string& data, size_t& itor);
		void skipSpace(std::string_view data, size_t& itor);
		void skipSpace(const std::u8string& data, size_t& itor);
		std::string getString(std::string_view data, size_t& itor);
		std::u8string getString(const std::u8string& data, size_t& itor);
		JObject getNumber(std::string_view data, size_t& itor);
		u8JObject getNumber(const std::u8string& data, size_t& itor);
		JObject getBool(std::string_view data, size_t& itor);
		u8JObject getBool(const std::u8string& data, size_t& itor);
		JObject getNull(std::string_view data, size_t& itor);
		u8JObject getNull(const std::u8string& data, size_t& itor);
	};

	class JWriter
	{
	public:
		JWriter() = default;
		~JWriter() = default;

		std::string write(const JObject& jo);
		std::u8string u8write(const u8JObject& jo);
		std::string formatWrite(const JObject& jo, size_t n = 1);
		std::u8string u8formatWrite(const u8JObject& jo, size_t n = 1);

		static std::string fastWrite(const JObject& jo);
		static std::string fastFormatWrite(const JObject& jo);
		static std::u8string u8fastWrite(const u8JObject& jo);
		static std::u8string u8fastFormatWrite(const u8JObject& jo);
	};

	using u8null_t = bool;
	using u8int_t = long long;
	using u8bool_t = bool;
	using u8double_t = long double;
	using u8string_t = std::u8string;
	using u8list_t = std::vector<u8JObject>;
	using u8dict_t = std::unordered_map<std::u8string, u8JObject>;

	class u8JObject
	{
	public:
		u8JObject();
		u8JObject(const u8JObject& jo);
		u8JObject(u8JObject&& jo) noexcept;
		u8JObject(JValueType jvt);
		u8JObject(long long value);
		u8JObject(long value);
		u8JObject(int value);
		u8JObject(short value);
		u8JObject(bool value);
		u8JObject(long double value);
		u8JObject(double value);
		u8JObject(float value);
		u8JObject(const char8_t* data);
		u8JObject(const std::u8string& data);
		u8JObject(std::u8string&& data);
		~u8JObject();

		operator long double& () const;
		operator double() const;
		operator float() const;
		operator long long& () const;
		operator long() const;
		operator int() const;
		operator short() const;
		operator bool& () const;
		operator std::u8string& () const;
		operator u8list_t& () const;
		operator u8dict_t& () const;

		u8JObject& operator =(const u8JObject& jo);
		u8JObject& operator =(u8JObject&& jo) noexcept;
		bool operator ==(const u8JObject& jo);
		bool operator ==(JValueType type);
		u8JObject& operator[](size_t itor);
		u8JObject& operator[](int itor);
		u8JObject& operator[](const char8_t* str);

		void push_back(const u8JObject& jo);
		void push_back(u8JObject&& jo);
		void pop_back();
		bool hasMember(const std::u8string& str);
		JValueType getType() const;
		u8list_t& getList() const;
		u8dict_t& getDict() const;
		long long& getInt() const;
		long double& getDouble() const;
		bool& getBool() const;
		std::u8string& getString() const;
	protected:
		using u8value_t = std::variant<u8int_t, u8bool_t, u8double_t, u8string_t, u8list_t, u8dict_t>;
	private:
		u8value_t* m_value;
		JValueType m_type;
	};
}

