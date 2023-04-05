# QQ机器人
## 介绍
- 这个QQ机器人拥有插件系统，可以轻松扩展
- 语言：C++

## 依赖
- [JsonParser](https://github.com/quqiOnfree/JsonParser)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [go-cqhttp](https://github.com/Mrs4s/go-cqhttp)

## 功能
### QQ机器人插件系统
- 此机器人只有一个框架，用于插件扩展，所有指令（除了基本的help等基础指令）都是通过插件加入  
- 插件类  
```cpp
#pragma once

#include <string>

#include "command.h"
#include "permission.h"

namespace qqbot
{
	struct PluginInfo
	{
		//插件名称
		std::string name;

		//插件作者名
		std::string author;

		//插件版本
		std::string version;
	};

	class CppPlugin
	{
	public:
		//禁止复制，只能移动
		CppPlugin() = default;
		CppPlugin(const CppPlugin&) = delete;
		~CppPlugin() = default;

		CppPlugin& operator =(const CppPlugin&) = delete;

		//插件加载
		virtual void onLoad()
		{
		}

		//插件启动
		virtual void onEnable()
		{
		}

		//插件关闭
		virtual void onDisable()
		{
		}

	public:
		//插件信息
		PluginInfo pluginInfo;

	private:

	};
}

```

- 插件例子[groupOperator.h](./plugin/groupOperator.h)  
```cpp
#include <httplib.h>
#include <Json.h>
#include <vector>
#include <string>

//最基本的三个库
#include "cppPlugin.h"
#include "pluginLibrary.h"
#include "network.h"

namespace GroupOperator
{
    class GroupOperatorPlugin : public qqbot::CppPlugin //继承插件类
    {
    public:
        GroupOperatorPlugin()
        {
            qqbot::CppPlugin::pluginInfo.author = "quqiOnfree";            //作者名
            qqbot::CppPlugin::pluginInfo.name = "GroupOperator群组管理插件";//插件名称
            qqbot::CppPlugin::pluginInfo.version = "0.0.1";                //插件版本
        }

        ~GroupOperatorPlugin() = default;

        virtual void onLoad()
        {
            //这里是插件加载之前执行
        }

        virtual void onEnable()
        {
            //这里是插件加载完成时启动执行

            //添加指令
            qqbot::ServerInfo::getCommander().addCommand("kick",//命令名称
                [this](long long groupID,
                    long long senderID,
                    const std::string& commandName,
                    std::vector<std::string> Args)              //lambda表达式或符合格式的函数
                {
                    //...
                },
                "kick userid true/false",       //命令格式
                "踢出群成员"                    //命令作用解释
                );
        }
    };
}

```
