// qqbot.cpp: 定义应用程序的入口点。
//

#include "qqbot.h"

httplib::Server		server;
qqbot::Permission	permission;
qqbot::Command		command(&permission);
qqbot::Register		pluginRegister;

int main()
{
	qqbot::init(server, permission, pluginRegister);

	return 0;
}
