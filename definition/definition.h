#pragma once

#include <exception>
#include <format>
#include <filesystem>
#include <stacktrace>

//报错宏 throw THROW_ERROR("message");
#define THROW_ERROR(errmsg) std::exception(std::format("in file \"{}\" line {}, {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, errmsg).c_str())

namespace qqbot
{
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
}
