#include <thread>
#include <functional>
#include <chrono>
#include "Database.h"
#include "Log.h"
#include "Timer.h"
#include "Client.h"

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
#elif LINUX
#include <sys/signal.h>
#endif

ClientSocket* createClient()
{
	return new Client;
}

void destroyClient(ClientSocket* cs)
{
	delete cs;
}

ServerSocket* gServer = nullptr;
DBQueue* gDBQueue = nullptr;

int main(int argc, char *argv[])
{
	Log::level = LogLevel::_DEBUG_;
#ifdef WIN32
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true))
	{
		Log::i("The Control Handler is installed.");
	}
#elif LINUX
	signal(SIGPIPE, SIG_IGN);
#endif
	// init server socket
	ServerConfig svrConfig;
	svrConfig.listenPort = 10001;
	svrConfig.maxConnection = 2000;
	svrConfig.kickTime = std::chrono::minutes(2);
	svrConfig.createClient = std::bind(&createClient);
	svrConfig.destroyClient = std::bind(&destroyClient, std::placeholders::_1);

	gServer = new ServerSocket(svrConfig);
	gServer->startListen();

	// init db
	MYSQL_CONFIG dbConfig;
#ifdef WIN32
	dbConfig.strHost = "192.168.11.151";
	dbConfig.strPassword = "";
#elif LINUX
	dbConfig.strHost = "localhost";
	dbConfig.strPassword = "";
	dbConfig.strUnixSock = "/tmp/mysql.sock";
#endif
	dbConfig.nPort = 3306;
	dbConfig.strUser = "root";
	dbConfig.strDB = "star2015";

	gDBQueue = new DBQueue;
	gDBQueue->init(5, dbConfig);

	while (!isExit)
	{
		gServer->update();
		gDBQueue->update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	delete gDBQueue;
	delete gServer;
	Log::i("server will shutting down in 3 seconds");
	std::this_thread::sleep_for(std::chrono::seconds(3));
	return 0;
}
