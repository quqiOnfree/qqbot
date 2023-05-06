#pragma once

#include <exception>
#include <format>
#include <filesystem>

//报错宏 throw THROW_ERROR("message");
#define THROW_ERROR(errmsg) std::exception(std::format("in file \"{}\" line {}, {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, errmsg).c_str())

namespace qqbot
{

}
