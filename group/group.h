#pragma once

#include <httplib.h>

namespace qqbot
{
	//群消息处理
	httplib::Response groupMethod(const httplib::Request& req);
}