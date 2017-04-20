#ifndef __ACTIVATION_CODE_H__
#define __ACTIVATION_CODE_H__

#include "network/Message.h"
#include "network/WebsocketClient.h"

class UserLogin
{
public:
	static void processLogin(WebsocketClient* client, LoginMessage* msg);
	static void processEnterGame(WebsocketClient* client, EnterGameMessage* msg);
	static void processCreateRole(WebsocketClient* client, CreateRoleMessage* msg);

private:
	static void onRoleListGet(int64_t clientID, UserAccount* account);
	static void replyRoleList(WebsocketClient* client, UserAccount* account);
	static void replyUserData(int64_t clientID, PlayerData* data);
};

#endif