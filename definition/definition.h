#pragma once

#include <exception>
#include <format>
#include <filesystem>

#if _HAS_CXX23
// c++23 stacktrace
#include <stacktrace>
#endif

//报错宏 throw THROW_ERROR("message");
#define THROW_ERROR(errmsg) std::exception(std::format("in file \"{}\" line {}, {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, errmsg).c_str())

#if _HAS_CXX23
#define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\nstack trace: \n{}\n", errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__, std::to_string(std::stacktrace::current()))
#else
#define ERROR_WITH_STACKTRACE(errmsg) THROW_ERROR(errmsg)
#endif

namespace qqbot
{
	// Error 暂时用不到
	/*
	class Error
	{
	public:
		Error() = default;
		~Error() = default;

		static std::string outErrorMessage(const std::string& errorMessage)
		{
			return std::move(
				std::format(
					"error: {}\nstack trace: \n{}\n",
					errorMessage,
					std::to_string(std::stacktrace::current()
					)
				)
			);
		}

		static std::string outErrorMessage(const std::logic_error& errorMessage)
		{
			return std::move(
				std::format(
					"error: {}\nstack trace: \n{}\n",
					errorMessage.what(),
					std::to_string(std::stacktrace::current()
					)
				)
			);
		}
	};
	*/
}
