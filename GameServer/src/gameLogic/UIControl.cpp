#include <assert.h>
#include "UIControl.h"

UIControl::UIControl() : client(nullptr), queryRequest(nullptr)
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

void UIControl::processUIConfig(Client* client, UIConfigMessage* msg)
{
	assert(msg != NULL);
	this->client = client;
	if (!queryRequest)
	{
		queryRequest = new UIConfigQuery;
		queryRequest->callback = std::bind(&UIControl::onUIConfigGet, this, std::placeholders::_1);
	}
	queryRequest->queryUIConfig(msg->version, msg->platform);
	client->manager->addDBRequest(queryRequest);
}

void UIControl::onUIConfigGet(void* data)
{
	UIConfigMessage* replyData = nullptr;
	if (data)
	{
		replyData = (UIConfigMessage*)data;
	}
	else
	{
		replyData = new UIConfigMessage;
	}
	ByteArray packet;
	MessageHead head;
	head.command = CMD_S2C_UI_CONFIG;
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


