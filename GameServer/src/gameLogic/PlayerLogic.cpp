#include "PlayerLogic.h"
#include "GlobalData.h"
#include "network/GameServer.h"

void PlayerLogic::notifyPlayerData(WebsocketClient* client, bool force)
{
	if (force || client->dataChangedFlag & DATA_CHANGE_FLAG_PLAYER_BASE)
	{
		PlayerMessage msg;
		msg.player = client->playerData;
		client->sendMessage(CMD_PLAYER_DATA, msg);
		client->dataChangedFlag &= ~DATA_CHANGE_FLAG_PLAYER_BASE;
	}
}