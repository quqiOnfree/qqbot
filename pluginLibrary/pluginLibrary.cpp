#include "pluginLibrary.h"

extern qqbot::Permission	permission;
extern qqbot::Register		pluginRegister;
extern qqbot::Command		command;

namespace qqbot::ServerInfo
{
	qqbot::Command& getCommander()
	{
		return command;
	}

	qqbot::Permission& getPermission()
	{
		return permission;
	}

	qqbot::Register& getPluginRegister()
	{
		return pluginRegister;
	}
}
