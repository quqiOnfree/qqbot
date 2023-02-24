#pragma once

#include <httplib.h>

#include "permission.h"

namespace qqbot
{
	int init(httplib::Server& server, qqbot::Permission& permission);
}
