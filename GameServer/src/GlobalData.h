#ifndef __GLOBAL_DATA_H__
#define __GLOBAL_DATA_H__

#include "DataDefinition.h"
#include "database/PlayerQuery.h"

class GameDataCache
{
public:
	static void													init();
	static void													update();
	static void													cleanup();
	// global ids
	static uint64_t												getNextPlayerID(){ return ++nextPlayerID; }
	// 添加玩家账号缓存
	static void													addAccountCache(UserAccount* account);
	// 删除玩家账号缓存
	static bool													delAccountCache(const std::string& accName);
	// 获取玩家账号
	static UserAccount*											getAccount(const std::string& account);
	
	typedef std::function<void(PlayerData*)>	PlayerCallback;
	// 添加玩家角色缓存
	static void													addPlayerCache(PlayerData* data);
	// 删除玩家角色缓存
	static bool													delPlayerCache(uint64_t id);
	// 通过id获取玩家角色数据
	static void													getPlayerByID(uint64_t id, PlayerCallback callback);
	// 保存玩家数据
	static void													savePlayerData(PlayerData* data);

private:
	static uint64_t												nextPlayerID;
	static std::map<std::string, UserAccount*>					accountCache;		//账号缓存
	static std::map<std::string, UserAccount*>::iterator		lastAccountIter;	//缓存遍历迭代器

	static std::map<uint64_t, PlayerData*>						playerCache;			//角色缓存
	static std::map<uint64_t, PlayerData*>::iterator			lastPlayerIter;			//缓存遍历迭代器
	static std::map<uint64_t, std::list<PlayerCallback>>		playerCallbacks;		//查询角色回调

	// 初始化玩家id的回调
	static void													onPlayerIDGet(uint64_t id);
	// 查询角色的回调函数
	static void onPlayerGet(uint64_t id, PlayerData* data);
};

#endif