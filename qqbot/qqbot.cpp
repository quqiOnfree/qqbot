// qqbot.cpp: 定义应用程序的入口点。
//

#include "qqbot.h"

httplib::Server		server;
qqbot::Permission	permission;
qqbot::Register		pluginRegister;
qqbot::Command		command(&permission);

int main()
{
	if (qqbot::init(server, permission, pluginRegister))
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
