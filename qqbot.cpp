// qqbot.cpp: 定义应用程序的入口点。
//

#include "qqbot.h"

httplib::Server		server;
qqbot::Permission	permission;

int main()
{
	qqbot::init(server, permission);

	return 0;
}
