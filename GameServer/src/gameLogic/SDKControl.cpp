#include "SDKControl.h"
#include "ServerSocket.h"
#include <assert.h>
#include "Log.h"
#include "GameServer.h"

SDKControl::SDKControl()
{

}

SDKControl* SDKControl::_instance = nullptr;

SDKControl* SDKControl::getInstance()
{
	if (!_instance)
	{
		_instance = new SDKControl;
	}
	return _instance;
}

void SDKControl::processSDKConfig(long long clientID, SDKConfigMessage* msg)
{
	assert(msg != NULL);
	SDKConfigQuery* queryRequest(new SDKConfigQuery);
	queryRequest->querySDKConfig(msg->version, msg->platform, std::bind(&SDKControl::onSDKConfigGet, this, clientID, std::placeholders::_1));
	GameServer::getInstance()->getDBQueue()->addQueueMsg(std::shared_ptr<SDKConfigQuery>(queryRequest));
}

void SDKControl::onSDKConfigGet(long long clientID, SDKConfigMessage* data)
{
	auto client(GameServer::getInstance()->getLogicServer()->getClient(clientID));
	if (!client)
	{
		Log::w("client is disconnect before sql execute");
		return;
	}

	SDKConfigMessage* replyData = data;
	if (!data)
	{
		replyData = new SDKConfigMessage;
	}
	ByteArray packet;
	MessageHead head;
	head.command = CMD_S2C_SDK_CONFIG;
	head.pack(packet);
	replyData->pack(packet);

	// rewrite packet size
	head.packSize = (unsigned short)packet.getSize();
	packet.position = 0;
	head.pack(packet);

	client->send(packet);
	client->flush();
	client = nullptr;
	if (!data)
	{
		delete replyData;
	}
}

