#pragma once

#include <exception>
#include <format>
#include <filesystem>

#define THROW_ERROR(errmsg) std::exception(std::format("in {} {}line, {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, errmsg).c_str())

namespace qqbot
{

}
