#include "GameServer.h"
#include <openssl/evp.h>
#include "utils/String.h"
#include "WebsocketClient.h"
#include "GlobalData.h"

#ifdef _WIN32
#include <DbgHelp.h>
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <execinfo.h>
#endif // WIN32

utils::Timer<system_clock>* GameServer::timer = nullptr;
DBQueue* GameServer::dbQueue = nullptr;
ServerSocket* GameServer::logicServer = nullptr;
ws::EventDispatcher GameServer::dispatcher;

bool GameServer::init()
{
	std::map<std::string, std::string> config;
	if (!loadConfig(config))
	{
		return false;
	}
	// game server
	ServerConfig svrConfig;
	svrConfig.listenPort = atoi(config["game_port"].c_str());
	svrConfig.maxConnection = atoi(config["game_max_conn"].c_str());
	svrConfig.kickTime = atoi(config["game_kick_time"].c_str());
	svrConfig.createClient = &GameServer::createClient;
	svrConfig.destroyClient = &GameServer::destroyClient;
	logicServer = new ServerSocket(svrConfig);

	// init game db
	MYSQL_CONFIG dbConfig;
	dbConfig.strHost = config["db_host"].c_str();
	dbConfig.nPort = atoi(config["db_port"].c_str());
	dbConfig.strUser = config["db_user"].c_str();
	dbConfig.strPassword = config["db_pass"].c_str();
	dbConfig.strDB = config["db_database"].c_str();
#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
	dbConfig.strUnixSock = config["db_sock"].c_str();
#endif // defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
	dbQueue = new DBQueue;
	dbQueue->init(5, dbConfig);
	
	// init timer
	timer = new utils::Timer<system_clock>;

	// init data cache and logic
	GameDataCache::init();

	// init libopenssl
	OpenSSL_add_all_digests();
	return true;
}

void GameServer::cleanup()
{
	delete logicServer;
	GameDataCache::cleanup();
	delete dbQueue;
	delete timer;
	EVP_cleanup();
}

int GameServer::startListen()
{
	if (logicServer->startListen() == -1)
	{
		return -1;
	}
	return 0;
}

void GameServer::update()
{
	logicServer->update();
	dbQueue->update();
	timer->update();
}

Client* GameServer::createClient()
{
	return new WebsocketClient;
}

void GameServer::destroyClient(Client* cs)
{
	delete cs;
}

/************************************************************************/
/* Delegate of EventDispatcher                                          */
/************************************************************************/

void GameServer::addEventListener(int type, EventCallback* callback)
{
	dispatcher.addEventListener(type, callback);
}

void GameServer::removeEventListener(int type, EventCallback* callback)
{
	dispatcher.removeEventListener(type, callback);
}

void GameServer::dispatchEvent(const Event& evt)
{
	dispatcher.dispatchEvent(evt);
}

void GameServer::broadcastToAllPlayers(ClientCommand cmd, Message& msg)
{
	auto allClients = getLogicServer()->getAllClients();
	for (auto &item : allClients)
	{
		WebsocketClient* client = (WebsocketClient*)item.second;
		client->sendMessage(cmd, msg);
	}
}

void GameServer::broadcastToAllPlayers(ClientCommand cmd, uint32_t bodySize, void* body)
{
	auto allClients = getLogicServer()->getAllClients();
	for (auto &item : allClients)
	{
		WebsocketClient* client = (WebsocketClient*)item.second;
		client->sendMessage(cmd, bodySize, body);
	}
}

bool GameServer::loadConfig(std::map<std::string, std::string>& config)
{
	FILE* file = fopen("./config.ini", "r");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		auto size = ftell(file);
		fseek(file, 0, SEEK_SET);
		char* data = new char[size + 1];
		memset(data, 0, size + 1);
		fread(data, 1, size, file);
		std::vector<char*> allRows;
		String::split(data, "\n", allRows);
		for (auto row : allRows)
		{
			std::vector<char*> kv_pair;
			String::split(row, "=", kv_pair);
			if (kv_pair.size() == 2)
			{
				config[kv_pair[0]] = kv_pair[1];
			}
		}
		delete[] data;
		fclose(file);
		return true;
	}
	else
	{
		Log::e("open config.ini error: %s", strerror(errno));
		return false;
	}
}
