#include "pluginLibrary.h"

extern qqbot::Permission	permission;
extern qqbot::Command		command;
extern qqbot::Register		pluginRegister;

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
