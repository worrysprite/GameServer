#ifndef __ACTIVATION_CODE_QUERY_H__
#define __ACTIVATION_CODE_QUERY_H__

#include "Database.h"
#include "utils/ByteArray.h"
#include "DataDefinition.h"

using namespace ws;

class PlayerQuery : public DBRequest
{
public:
	PlayerQuery() : data(nullptr), names(nullptr), count(0), playerID(0) {}
	virtual ~PlayerQuery(){}

	typedef std::function<void(UserAccount*)>	AccountCallback;
	typedef std::function<void(PlayerData*)>	PlayerDataCallback;
	typedef std::function<void(uint64_t)>		PlayerIDCallback;

	void getMaxPlayerID(PlayerIDCallback callback);
	// 通过玩家账号查询玩家ID列表
	void getPlayersByAccount(const std::string& accName, AccountCallback callback);
	// 通过玩家ID查询玩家数据
	void getPlayerByID(uint64_t id, PlayerDataCallback callback);
	// 保存玩家数据
	void saveUserData(const PlayerData* player);
	// 保存基础信息
	void savePlayerBase(const PlayerData* player);

	virtual void onRequest(Database& db);
	virtual void onFinish();

private:
	enum QueryType
	{
		GET_MAX_PLAYER_ID,
		GET_PLAYERS_BY_ACCOUNT,
		GET_PLAYER_BY_ID,
		SAVE_PLAYER_DATA,
	};
	QueryType							type;
	// player data query and updates
	void*								data;
	std::string*						names;
	uint32_t							count;
	uint64_t							playerID;
	PlayerIDCallback					playerIDCallback;
	PlayerDataCallback					playerCallback;
	AccountCallback						accountCallback;
};

#endif