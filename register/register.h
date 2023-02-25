#pragma once

#include <unordered_map>
#include <memory>

#include "cppPlugin.h"

namespace qqbot
{
	class Register
	{
	public:
		//禁止复制，只能移动
		Register() = default;
		Register(const Register&) = delete;

		Register& operator=(const Register&) = delete;

		//初始化，添加插件在这里
		void init();
		//所有插件加载完毕开始运作
		void run();
		//添加插件
		void addPlugin(const std::shared_ptr<CppPlugin>& plugin);

	private:
		//存储插件的哈希表
		std::unordered_map<std::string, std::shared_ptr<CppPlugin>> m_plugins;
	};
}
