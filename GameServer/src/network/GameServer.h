#ifndef __GAME_SERVER_H__
#define __GAME_SERVER_H__

#include <chrono>
#include "ServerSocket.h"
#include "ClientSocket.h"
#include "Database.h"
#include "utils/Timer.h"
#include "utils/Math.h"
#include "utils/TimeTool.h"
#include "GameEvent.h"
#include "HttpClient.h"

using namespace std::chrono;
using namespace ws;
using namespace ws::utils;

class GameServer
{
public:
	static bool									init();
	static void									cleanup();

	static int									startListen();
	static void									update();

	inline static ServerSocket*					getLogicServer(){ return logicServer; }
	inline static DBQueue*						getDBQueue(){ return dbQueue; }
	inline static Timer<system_clock>*			getTimer(){ return timer; }
	
	static void									addEventListener(int type, EventCallback* callback);
	static void									removeEventListener(int type, EventCallback* callback);
	static void									dispatchEvent(const Event& evt);
	static void									broadcastToAllPlayers(ClientCommand cmd, Message& msg);
	static void									broadcastToAllPlayers(ClientCommand cmd, uint32_t bodySize, void* body);


private:
	static Client*								createClient();
	static void									destroyClient(Client* cs);
	static bool									loadConfig(std::map<std::string, std::string>& config);

	static ServerSocket*						logicServer;
	static DBQueue*								dbQueue;
	static Timer<system_clock>*					timer;
	static EventDispatcher						dispatcher;
};

#endif