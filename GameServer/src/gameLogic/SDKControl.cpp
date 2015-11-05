#include "SDKControl.h"
#include <assert.h>

SDKControl::SDKControl() : client(nullptr), queryRequest(nullptr)
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

void SDKControl::processSDKConfig(Client* client, SDKConfigMessage* msg)
{
	assert(msg != NULL);
	this->client = client;
	if (!queryRequest)
	{
		queryRequest = new SDKConfigQuery;
		queryRequest->callback = std::bind(&SDKControl::onSDKConfigGet, this, std::placeholders::_1);
	}
	queryRequest->querySDKConfig(msg->version, msg->platform);
	client->manager->addDBRequest(queryRequest);
}

void SDKControl::onSDKConfigGet(void* data)
{
	SDKConfigMessage* replyData = nullptr;
	if (data)
	{
		replyData = (SDKConfigMessage*)data;
	}
	else
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

