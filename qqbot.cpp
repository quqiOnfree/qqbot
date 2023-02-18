// qqbot.cpp: 定义应用程序的入口点。
//

#include "qqbot.h"

httplib::Server server;

int main()
{
	qqbot::init(server);

	return 0;
}
