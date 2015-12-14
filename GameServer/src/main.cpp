#include <thread>
#include <functional>
#include <chrono>
#include "utils/Log.h"
#include "GameServer.h"

using namespace ws::utils;

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
#elif LINUX
#include <sys/signal.h>
#endif

int main(int argc, char *argv[])
{
	Log::level = LogLevel::_VERBOSE_;
#ifdef WIN32
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true))
	{
		Log::i("The Control Handler is installed.");
	}
#elif LINUX
	signal(SIGPIPE, SIG_IGN);
#endif

	GameServer* server = GameServer::getInstance();
	if (server->startListen() == -1)
	{
		return -1;
	}
	while (!isExit)
	{
		server->update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	GameServer::cleanup();
	Log::i("server will shutting down in 3 seconds");
	std::this_thread::sleep_for(std::chrono::seconds(3));
	return 0;
}
