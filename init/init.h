#pragma once

#include <httplib.h>

#include "permission.h"
#include "command.h"
#include "register.h"

namespace qqbot
{
	int init(httplib::Server& server, qqbot::Permission& permission, qqbot::Register& pluginRegister);
}
