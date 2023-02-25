#pragma once

#include "command.h"
#include "permission.h"
#include "register.h"

namespace qqbot::ServerInfo
{
	qqbot::Command& getCommander();
	qqbot::Permission& getPermission();
	qqbot::Register& getPluginRegister();
}
