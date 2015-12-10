#ifndef __GAME_SERVER_H__
#define __GAME_SERVER_H__

#include "ServerSocket.h"
#include "Database.h"
#include "Client.h"

using namespace ws;

class GameServer
{
public:
	static GameServer* getInstance();
	static void cleanup();

	~GameServer();

	int startListen();
	void update();

	inline ServerSocket* getLogicServer(){ return logicServer; }
	inline ServerSocket* getConsoleServer(){ return consoleServer; }
	inline DBQueue* getDBQueue(){ return dbQueue; }

private:
	GameServer();

	ClientSocket* createClient();
	void destroyClient(ClientSocket* cs);

	static GameServer* _instance;
	ServerSocket* logicServer;
	ServerSocket* consoleServer;
	DBQueue* dbQueue;
};

#endif