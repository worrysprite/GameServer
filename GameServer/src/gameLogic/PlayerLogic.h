#ifndef __PLAYER_LOGIC_H__
#define __PLAYER_LOGIC_H__

#include "network/WebsocketClient.h"

class PlayerLogic
{
public:
	static void				notifyPlayerData(WebsocketClient* client, bool force = false);
};
#endif