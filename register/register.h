#pragma once

#include <unordered_map>
#include <memory>

#include "cppPlugin.h"

namespace qqbot
{
	class Register
	{
	public:
		Register() = default;
		Register(const Register&) = delete;

		Register& operator=(const Register&) = delete;

		void init();
		void run();
		void addPlugin(const std::shared_ptr<CppPlugin>& plugin);

	private:
		std::unordered_map<std::string, std::shared_ptr<CppPlugin>> m_plugins;
	};
}
