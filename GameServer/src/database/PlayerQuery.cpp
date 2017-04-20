#include "PlayerQuery.h"
#include "utils/Log.h"
#include "network/Message.h"
#include "GlobalData.h"

void PlayerQuery::getMaxPlayerID(PlayerIDCallback callback)
{
	type = GET_MAX_PLAYER_ID;
	playerIDCallback = callback;
}

void PlayerQuery::getPlayersByAccount(const std::string& accName, AccountCallback callback)
{
	type = GET_PLAYERS_BY_ACCOUNT;
	UserAccount* account = new UserAccount;
	account->account = accName;
	data = account;
	accountCallback = callback;
}

void PlayerQuery::getPlayerByID(uint64_t id, PlayerDataCallback callback)
{
	type = GET_PLAYER_BY_ID;
	playerID = id;
	playerCallback = callback;
}

void PlayerQuery::saveUserData(const PlayerData* player)
{
	type = SAVE_PLAYER_DATA;
	data = new PlayerData(*player);
}

void PlayerQuery::onRequest(Database& db)
{
	DBStatement* stmt(nullptr);
	switch (type)
	{
	case PlayerQuery::GET_MAX_PLAYER_ID:
	{
		PtrDBRecord record = db.query("SELECT MAX(`id`) FROM `player`");
		if (record->nextRow())
		{
			*record >> playerID;
		}
		break;
	}
	case PlayerQuery::GET_PLAYERS_BY_ACCOUNT:
	{
		stmt = db.prepare("SELECT `id`,`role_name`,`sex`,`level` FROM `player` WHERE `account`=?");
		UserAccount* account = (UserAccount*)data;
		*stmt << account->account;
		stmt->execute();
		count = (uint32_t)stmt->numRows();
		if (!count)
		{
			break;
		}
		for (uint32_t i = 0; i < count && stmt->nextRow(); ++i)
		{
			ChooseRoleInfo cri;
			*stmt >> cri.id >> cri.name >> cri.sex >> cri.level;
			account->roleList.push_back(cri);
		}
		break;
	}
	case PlayerQuery::GET_PLAYER_BY_ID:
	{
		// query player first
		stmt = db.prepare("SELECT * FROM `player` WHERE `id`=?");
		// input params
		*stmt << playerID;
		stmt->execute();
		if (stmt->nextRow())
		{
			PlayerData* player = new PlayerData;
			*stmt >> player->id >> player->account >> player->roleName >> player->sex >> player->level;
			data = player;
		}
		else
		{
			Log::d("Player not found, id=%llu", playerID);
		}
		break;
	}
	case PlayerQuery::SAVE_PLAYER_DATA:
	{
		stmt = db.prepare("INSERT INTO `player` VALUES(?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE "
			"`account`=VALUES(`account`), `role_name`=VALUES(`role_name`), `sex`=VALUES(`sex`), `level`=VALUES(`level`)");
		PlayerData* player = (PlayerData*)data;
		*stmt << player->id << player->account << player->roleName << player->sex << player->level;
		stmt->execute();
		if (!stmt->numRows())
		{
			Log::w("Save player data failed! id=%llu", player->id);
		}
		break;
	}
	}
	db.release(stmt);
}

void PlayerQuery::onFinish()
{
	switch (type)
	{
	case PlayerQuery::GET_MAX_PLAYER_ID:
		if (playerIDCallback)
		{
			playerIDCallback(playerID);
		}
		break;
	case PlayerQuery::GET_PLAYERS_BY_ACCOUNT:
		if (accountCallback)
		{
			accountCallback((UserAccount*)data);
		}
		delete (UserAccount*)data;
		break;
	case PlayerQuery::GET_PLAYER_BY_ID:
		if (playerCallback)
		{
			playerCallback((PlayerData*)data);
		}
		break;
	case PlayerQuery::SAVE_PLAYER_DATA:
		delete (PlayerData*)data;
		break;
	}
}
