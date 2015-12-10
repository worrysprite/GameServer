#include <assert.h>
#include "UIControl.h"
#include "Log.h"
#include "GameServer.h"

UIControl::UIControl()
{

}

UIControl* UIControl::_instance = NULL;

UIControl* UIControl::getInstance()
{
	if (!_instance)
	{
		_instance = new UIControl;
	}
	return _instance;
}

void UIControl::processUIConfig(long long clientID, UIConfigMessage* msg)
{
	assert(msg != NULL);
	UIConfigQuery* queryRequest = new UIConfigQuery;
	queryRequest->queryUIConfig(msg->version, msg->platform, std::bind(&UIControl::onUIConfigGet, this, clientID, std::placeholders::_1));
	GameServer::getInstance()->getDBQueue()->addQueueMsg(std::shared_ptr<UIConfigQuery>(queryRequest));
}

void UIControl::onUIConfigGet(long long clientID, UIConfigMessage* data)
{
	ClientSocket* client = GameServer::getInstance()->getLogicServer()->getClient(clientID);
	if (!client)
	{
		Log::w("client is disconnect before sql execute");
		return;
	}

	ByteArray packet;
	MessageHead head;
	head.command = CMD_S2C_UI_CONFIG;
	head.pack(packet);
	data->pack(packet);

	// rewrite packet size
	head.packSize = (unsigned short)packet.getSize();
	packet.position = 0;
	head.pack(packet);

	client->send(packet);
	client->flush();
	client = nullptr;
}


