#pragma once

#include <httplib.h>

#include "permission.h"
#include "command.h"

namespace qqbot
{
	int init(httplib::Server& server, qqbot::Permission& permission);
}
