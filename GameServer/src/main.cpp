#include <thread>
#include <functional>
#include <chrono>
#include "utils/Log.h"
#include "network/GameServer.h"
#include "upgrade/Upgrader.h"

using namespace ws::utils;

bool isExit = false;

#ifdef _WIN32
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp")

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

LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	HANDLE lhDumpFile = CreateFile("DumpFile.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), lhDumpFile, MiniDumpWithFullMemory, &loExceptionInfo, NULL, NULL);
	CloseHandle(lhDumpFile);
	return EXCEPTION_EXECUTE_HANDLER;
}

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <sys/signal.h>

void CtrlHandler(int sig)
{
	Log::d("signal received: %d", sig);
	isExit = true;
}
#endif

int start()
{
	if (!GameServer::init())
	{
		return -1;
	}
	if (GameServer::startListen() == -1)
	{
		return -1;
	}
	while (!isExit)
	{
		GameServer::update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	GameServer::cleanup();
	Log::i("server will shutting down in 3 seconds");
	std::this_thread::sleep_for(std::chrono::seconds(3));
	return 0;
}

int showVersion()
{
	Log::i("GameServer 1.0.0");
#ifdef _WIN32
	system("pause");
#endif
	return 0;
}

int upgradeDatabase()
{
	Log::i("GameServer upgrading database structure");
	if (!Upgrader::init())
	{
		return -1;
	}
	while (!isExit)
	{
		Upgrader::update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	if (Upgrader::isFinished())
	{
		Log::i("upgrade finished.");
	}
	else
	{
		Log::i("stopping upgrade.");
	}
	Upgrader::cleanup();
#ifdef _WIN32
	system("pause");
#endif
	return 0;
}

int main(int argc, char *argv[])
{
	Log::level = LogLevel::_DEBUG_;
	// init ctrl+c signal
#ifdef _WIN32
	system("chcp 65001");
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true))
	{
		Log::i("The Control Handler is installed.");
	}
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, CtrlHandler);
	signal(SIGKILL, CtrlHandler);
	signal(SIGTERM, CtrlHandler);
	signal(SIGQUIT, CtrlHandler);
#endif

	if (argc > 1)
	{
		for (int i = 1; i < argc; ++i)
		{
			if (argv[i] == std::string("--version") || argv[i] == std::string("-v"))
			{
				return showVersion();
			}
			else if (argv[i] == std::string("--upgrade"))
			{
				return upgradeDatabase();
			}
		}
	}
	else
	{
		return start();
	}
}