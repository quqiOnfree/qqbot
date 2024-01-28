// qqbot.cpp: 定义应用程序的入口点。
//

#include "qqbot.h"

httplib::Server			server;
qqbot::Permission		permission;
qqbot::Register			pluginRegister;
qqbot::Command			command(&permission, &pluginRegister);
qqbot::EventRegister	eventRegister;

int main()
{
	return qqbot::init(server, permission, pluginRegister);
}
