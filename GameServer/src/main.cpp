#include <thread>
#include <chrono>
#include <iostream>
#include "Server.h"
#include "Database.h"
#include "Log.h"
#include "Timer.h"

#include <iostream>

using namespace ws;

bool isExit = false;

#ifdef WIN32
#include <windows.h>
BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		isExit = true;
		return true;

	case CTRL_BREAK_EVENT:
		return false;

	case CTRL_LOGOFF_EVENT:
		return false;

	default:
		return false;
	}
}
#endif

int main(int argc, char *argv[])
{
	Log::level = LogLevel::_VERBOSE_;
#ifdef WIN32
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true))
	{
		Log::i("The Control Handler is installed.");
	}
#endif
	Server* server = new Server;
	if (server->startListen(10001) == -1)
	{
		return -1;
	}
	while (!isExit)
	{
		server->update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	delete server;
	Log::i("server will shutting down in 3 seconds");
	std::this_thread::sleep_for(std::chrono::seconds(3));
	return 0;
}
