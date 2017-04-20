#include "GlobalData.h"
#include "utils/Log.h"
#include "utils/String.h"
#include "utils/Profiler.h"
#include "network/GameServer.h"

using namespace std::placeholders;

#define SAVE_INTERVAL 300
#define CACHE_TIME 86400000ULL
#define CACHE_LOOP_MAX 200

/************************************************************************/
/* GameDataCache implements                                             */
/************************************************************************/
uint64_t GameDataCache::nextPlayerID(0);
std::map<uint64_t, PlayerData*> GameDataCache::playerCache;
std::map<uint64_t, PlayerData*>::iterator GameDataCache::lastPlayerIter = GameDataCache::playerCache.begin();
std::map<std::string, UserAccount*> GameDataCache::accountCache;
std::map<std::string, UserAccount*>::iterator GameDataCache::lastAccountIter = GameDataCache::accountCache.begin();
std::map<uint64_t, std::list<GameDataCache::PlayerCallback>> GameDataCache::playerCallbacks;

void GameDataCache::init()
{
	PlayerQuery* query = new PlayerQuery;
	query->getMaxPlayerID(std::bind(&GameDataCache::onPlayerIDGet, _1));
	GameServer::getDBQueue()->addQueueMsg(PtrDBRequest(query));

	GameServer::getTimer()->addTimeCall(seconds(SAVE_INTERVAL), std::bind(&GameDataCache::update));
}

void GameDataCache::onPlayerIDGet(uint64_t id)
{
	nextPlayerID = id;
}

//玩家数据缓存策略：
//每次update时遍历CACHE_LOOP_MAX个账号和CACHE_LOOP_MAX个玩家数据
//保存这些玩家数据
//将离线24小时以上的缓存进行清除
//循环遍历，记录上一次遍历的位置
void GameDataCache::update()
{
	//遍历账号
	if (lastAccountIter == accountCache.end())
	{
		lastAccountIter = accountCache.begin();
	}
	uint64_t now = TimeTool::getUnixtime<system_clock>();
	for (uint32_t i = 0; i < CACHE_LOOP_MAX && lastAccountIter != accountCache.end(); ++i)
	{
		UserAccount* account = lastAccountIter->second;
		if (account->offlineTime && now - account->offlineTime >= CACHE_TIME)	// 离线超过24小时
		{
			WebsocketClient* client = (WebsocketClient*)GameServer::getLogicServer()->getClient(account->clientID);
			if (client)
			{
				client->kick();
			}
			lastAccountIter = accountCache.erase(lastAccountIter);
			delete account;
		}
		else
		{
			++lastAccountIter;
		}
	}
	//遍历玩家
	if (lastPlayerIter == playerCache.end())
	{
		lastPlayerIter = playerCache.begin();
	}
	for (uint32_t i = 0; i < CACHE_LOOP_MAX && lastPlayerIter != playerCache.end(); ++i)
	{
		PlayerData* player = lastPlayerIter->second;
		savePlayerData(player);
		if (player->offlineTime && now - player->offlineTime >= CACHE_TIME)
		{
			WebsocketClient* client = (WebsocketClient*)GameServer::getLogicServer()->getClient(player->clientID);
			if (client)
			{
				client->kick();
			}
			lastPlayerIter = playerCache.erase(lastPlayerIter);
			Log::d("delete player cache, id=%llu, client=%lld", player->id, player->clientID);
			delete player;
		}
		else
		{
			++lastPlayerIter;
		}
	}
	Log::d("current memory: %dKB", Profiler::getCurrentRSS() / 1024);
}

void GameDataCache::cleanup()
{

}

void GameDataCache::addPlayerCache(PlayerData* data)
{
	auto iter = playerCache.find(data->id);
	if (iter != playerCache.end())
	{
		delete iter->second;
		iter->second = data;
	}
	else
	{
		playerCache.insert(std::make_pair(data->id, data));
	}
}

bool GameDataCache::delPlayerCache(uint64_t id)
{
	std::map<uint64_t, PlayerData*>::iterator iter = playerCache.find(id);
	if (iter != playerCache.end())
	{
		PlayerData* player = iter->second;
		if (iter == lastPlayerIter)
		{
			lastPlayerIter = playerCache.erase(iter);
		}
		else
		{
			playerCache.erase(iter);
		}
		WebsocketClient* client = (WebsocketClient*)GameServer::getLogicServer()->getClient(player->clientID);
		if (client)
		{
			client->kick();
		}
		savePlayerData(player);
		delete player;
		return true;
	}
	return false;
}

void GameDataCache::getPlayerByID(uint64_t id, PlayerCallback callback)
{
	if (!callback)
	{
		return;
	}
	auto iter = playerCache.find(id);
	if (iter != playerCache.end())
	{
		PlayerData* player = iter->second;
		if (player->offlineTime)
		{
			player->offlineTime = TimeTool::getUnixtime<system_clock>();
		}
		callback(iter->second);
		return;
	}
	auto &list = playerCallbacks[id];
	list.push_back(callback);
	if (list.size() == 1)	// 相同的id在查询中则不发起新DB请求
	{
		PlayerQuery* query = new PlayerQuery;
		query->getPlayerByID(id, std::bind(&GameDataCache::onPlayerGet, id, _1));
		GameServer::getDBQueue()->addQueueMsg(PtrDBRequest(query));
	}
}

void GameDataCache::onPlayerGet(uint64_t id, PlayerData* data)
{
	if (data)	// 玩家存在，继续查询玩家其他数据
	{
		// 添加玩家数据到缓存并回调
		addPlayerCache(data);
		data->offlineTime = TimeTool::getUnixtime<system_clock>();
		auto &list = playerCallbacks[data->id];
		for (auto &callback : list)
		{
			callback(data);
		}
		playerCallbacks.erase(data->id);
	}
	else //玩家不存在
	{
		auto &list = playerCallbacks[id];
		for (auto &callback : list)
		{
			callback(nullptr);
		}
		playerCallbacks.erase(id);
	}
}

void GameDataCache::addAccountCache(UserAccount* account)
{
	auto iter = accountCache.find(account->account);
	if (iter != accountCache.end())
	{
		// 替换旧账号缓存，移除与旧账号有关的数据
		UserAccount* oldAccount = iter->second;
		WebsocketClient* client = (WebsocketClient*)GameServer::getLogicServer()->getClient(oldAccount->clientID);
		if (client)
		{
			client->kick();
		}
		delete oldAccount;
		iter->second = account;
	}
	else
	{
		accountCache.insert(std::make_pair(account->account, account));
	}
}

bool GameDataCache::delAccountCache(const std::string& accName)
{
	auto iter = accountCache.find(accName);
	if (iter != accountCache.end())
	{
		UserAccount* account = iter->second;
		accountCache.erase(iter);
		WebsocketClient* client = (WebsocketClient*)GameServer::getLogicServer()->getClient(account->clientID);
		if (client)
		{
			client->kick();
		}
		delete account;
		return true;
	}
	return false;
}

UserAccount* GameDataCache::getAccount(const std::string& account)
{
	auto iter = accountCache.find(account);
	if (iter != accountCache.end())
	{
		return iter->second;
	}
	return nullptr;
}

void GameDataCache::savePlayerData(PlayerData* data)
{
	if (!data)
	{
		return;
	}
	// save player
	if (data->isChanged)
	{
		data->isChanged = false;
		PlayerQuery* query = new PlayerQuery;
		query->saveUserData(data);
		GameServer::getDBQueue()->addQueueMsg(PtrDBRequest(query));
	}
}
