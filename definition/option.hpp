#ifndef OPTION_HPP
#define OPTION_HPP

// namespace option
#define NAMESPACE_OPTION_START namespace option {
#define NAMESPACE_OPTION_END }

#include <unordered_map>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iostream>

NAMESPACE_OPTION_START

class Option
{
public:
    enum class OptionType {
        OPT_UNKNOWN = 0,
        OPT_NO,
        OPT_REQUIRED,
        OPT_OPTIONAL
    };

    Option() = default;
    ~Option() = default;

    /*
    * @brief 添加参数
    * @param opt_name 参数名称
    * @param type 参数类型
    */
    void add(const std::string& opt_name, OptionType type)
    {
        if (type == OptionType::OPT_UNKNOWN)
            throw std::logic_error("type cant be \"OPT_UNKNOWN\"");
        
        m_opt_map[opt_name] = type;
    }

    /*
    * @brief 删除参数
    * @param opt_name 参数名称
    */
    void remove(const std::string& opt_name)
    {
        auto itor = m_opt_map.find(opt_name);
        if (itor != m_opt_map.end())
        {
            m_opt_map.erase(itor);
        }
    }

    /*
    * @brief 解析字符串
    * @param argc 参数个数
    * @param argv 字符串参数
    */
    void parse(int argc, char* const argv[])
    {
        std::vector<std::string> args;

        for (int i = 0; i < argc; i++)
        {
            args.emplace_back(argv[i]);
        }

        parse(args);
    }

    /*
    * @brief 解析字符串
    * @param command 需要解析的完整字符串
    */
    void parse(const std::string& command)
    {
        const std::string& data = command;
        std::vector<std::string> dataList;

        long long begin = -1;
        long long i = 0;

        for (; size_t(i) < data.size(); i++)
        {
            if (data[i] == ' ')
            {
                if ((i - begin - 1) > 0)
                {
                    dataList.push_back(data.substr(begin + 1, i - begin - 1));
                }
                begin = i;
            }
        }
        dataList.push_back(data.substr(begin + 1, i - begin - 1));

        parse(dataList);
    }

    /*
    * @brief 解析字符串
    * @param args 字符串参数
    */
    void parse(const std::vector<std::string>& args)
    {
        for (auto i = args.begin(); i != args.end(); i++)
        {
            const std::string& arg = *i;

            if (arg.substr(0, 1) != "-")
            {
                // 忽略普通参数
                continue;
            }
            if (arg.substr(0, 2) == "--")
            {
                // 长参数解析

                std::string str = arg.substr(2);
                if (str.empty())
                    continue;

                auto pos = str.find('=');
                if (pos != std::string::npos)
                {
                    // 出现等号
                    std::string opt_name = str.substr(0, pos);
                    std::string value = str.substr(pos + 1);

                    switch (get_type(opt_name))
                    {
                    case OptionType::OPT_UNKNOWN:
                        throw std::logic_error("No option: " + opt_name);

                    case OptionType::OPT_NO:
                        throw std::logic_error("No argument option: " + opt_name);

                    case OptionType::OPT_OPTIONAL:
                    case OptionType::OPT_REQUIRED:
                        m_args_map[opt_name] = value;
                        break;

                    default:
                        break;
                    }
                }
                else
                {
                    std::string& opt_name = str;
                    switch (get_type(opt_name))
                    {
                    case OptionType::OPT_NO:
                        m_args_map[opt_name] = "";
                        break;
                        
                    case OptionType::OPT_OPTIONAL:
                        if ((i + 1) != args.end() && (i + 1)->substr(0, 1) != "-")
                        {
                            m_args_map[opt_name] = *(i + 1);
                            i++;
                        }
                        else
                            m_args_map[opt_name] = "";
                        break;
                    
                    case OptionType::OPT_REQUIRED:
                        if ((i + 1) != args.end() && (i + 1)->substr(0, 1) != "-")
                        {
                            m_args_map[opt_name] = *(i + 1);
                            i++;
                            break;
                        }
                        else
                            throw std::logic_error("Option require argument: " + opt_name);

                    default:
                        break;
                    }
                }
            }
            else
            {
                // 短参数解析

                std::string str = arg.substr(1);
                if (str.empty())
                    continue;
                if (str.find('=') != std::string::npos)
                    throw std::logic_error("Invalid argument option: " + str);
                
                std::string opt_name = str.substr(0, 1);

                switch (get_type(opt_name))
                {
                case OptionType::OPT_NO:
                    for (int i = 0; i < str.size(); i++)
                    {
                        std::string o(1, str[i]);
                        if (get_type(o) != OptionType::OPT_NO)
                            continue;
                        m_args_map[o] = "";
                    }
                    break;
                    
                case OptionType::OPT_OPTIONAL:
                    if (str.size() > 1)
                        m_args_map[opt_name] = str.substr(1);
                    else if ((i + 1) != args.end() && (i + 1)->substr(0, 1) != "-")
                    {
                        m_args_map[opt_name] = *(i + 1);
                        i++;
                    }
                    else
                        m_args_map[opt_name] = "";

                    break;
                
                case OptionType::OPT_REQUIRED:
                {
                    if (str.size() > 1)
                        m_args_map[opt_name] = str.substr(1);
                    else if ((i + 1) != args.end() && (i + 1)->substr(0, 1) != "-")
                    {
                        m_args_map[opt_name] = *(i + 1);
                        i++;
                        break;
                    }
                    else
                        throw std::logic_error("Option require argument: " + opt_name);
                }
                    break;

                default:
                    break;
                }
            }
        }
    }

    bool has_opt(const std::string& opt) const
    {
        return  m_opt_map.find(opt) != m_opt_map.end() &&
                m_args_map.find(opt) != m_args_map.end();
    }

    bool get_bool(const std::string& opt) const
    {
        if (!has_opt(opt))
            throw std::logic_error("No option: " + opt);

        if (m_args_map.find(opt)->second == "true")
            return true;
        else if (m_args_map.find(opt)->second == "false")
            return false;
        else
            throw std::logic_error("No bool option: " + opt);
    }

    std::string get_string(const std::string& opt) const
    {
        if (!has_opt(opt))
            throw std::logic_error("No option: " + opt);

        return m_args_map.find(opt)->second;
    }

    long long get_int(const std::string& opt) const
    {
        if (!has_opt(opt))
            throw std::logic_error("No option: " + opt);
        
        return std::stoll(m_args_map.find(opt)->second);
    }

    long double get_double(const std::string& opt) const
    {
        if (!has_opt(opt))
            throw std::logic_error("No option: " + opt);
        
        return std::stold(m_args_map.find(opt)->second);
    }

    void show() const
    {
        std::cout << "m_opt_map\n";
        for (auto i = m_opt_map.begin(); i != m_opt_map.end(); i++)
        {
            switch (i->second)
            {
            case OptionType::OPT_UNKNOWN:
                std::cout << i->first << " OPT_UNKNOWN\n";
                break;

            case OptionType::OPT_NO:
                std::cout << i->first << " OPT_NO\n";
                break;

            case OptionType::OPT_OPTIONAL:
                std::cout << i->first << " OPT_OPTIONAL\n";
                break;

            case OptionType::OPT_REQUIRED:
                std::cout << i->first << " OPT_REQUIRED\n";
                break;
            
            default:
                break;
            }
        }

        std::cout << "m_args_map\n";
        for (auto i = m_args_map.begin(); i != m_args_map.end(); i++)
        {
            std::cout << i->first << ' ' << i->second << '\n';
        }
    }

protected:
    /*
    * @brief 获取参数type
    * @param opt 参数
    */
    OptionType get_type(const std::string& opt) const
    {
        auto itor = m_opt_map.find(opt);

        if (itor == m_opt_map.end())
        {
            return OptionType::OPT_UNKNOWN;
        }
        return itor->second;
    }

private:
    std::unordered_map<std::string, OptionType>     m_opt_map;
    std::unordered_map<std::string, std::string>    m_args_map;
};

NAMESPACE_OPTION_END

#endif
