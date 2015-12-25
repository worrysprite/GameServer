#ifndef __GAME_SERVER_H__
#define __GAME_SERVER_H__

#include "ServerSocket.h"
#include "Database.h"
#include "utils/Timer.h"

using namespace ws;

class GameServer
{
public:
	~GameServer();
	static GameServer*			getInstance();
	static void					cleanup();

	int							startListen();
	void						update();

	inline ServerSocket*		getLogicServer(){ return logicServer; }
	inline ServerSocket*		getConsoleServer(){ return consoleServer; }
	inline DBQueue*				getDBQueue(){ return dbQueue; }
	inline utils::Timer*		getTimer(){ return timer; }

private:
	GameServer();

	ClientSocket*				createClient();
	void						destroyClient(ClientSocket* cs);
	ClientSocket*				createConsole();
	void						destroyConsole(ClientSocket* cs);

	static GameServer*			_instance;
	ServerSocket*				logicServer;
	ServerSocket*				consoleServer;
	DBQueue*					dbQueue;
	utils::Timer*				timer;
};

#endif