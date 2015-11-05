#include <assert.h>
#include "ActivationCode.h"

ActivationCode* ActivationCode::_instance = nullptr;

ActivationCode::ActivationCode() :queryRequest(nullptr), updateRequest(nullptr), client(nullptr)
{

}

ActivationCode* ActivationCode::getInstance()
{
	if (!_instance)
	{
		_instance = new ActivationCode;
	}
	return _instance;
}

void ActivationCode::processActivationCode(Client* client, ActivationMessage* msg)
{
	this->client = client;
	assert(msg != NULL);
	if (!queryRequest)
	{
		queryRequest = new ActivationCodeQuery;
		queryRequest->callback = std::bind(&ActivationCode::onActivationInfoGet, this, std::placeholders::_1);
	}
	queryRequest->queryActivationCode(msg->code);
	client->manager->addDBRequest(queryRequest);
}

void ActivationCode::onActivationInfoGet(void* data)
{
	replyMsg = *(ActivationMessage*)data;
	if (replyMsg.status == 0)
	{
		if (!updateRequest)
		{
			updateRequest = new ActivationCodeQuery;
			updateRequest->callback = std::bind(&ActivationCode::onActivationUpdated, this, std::placeholders::_1);
		}
		updateRequest->updateActivationStatus(replyMsg.code, ActivationStatus::USED_CODE);
		client->manager->addDBRequest(updateRequest);
	}
	else
	{
		replyClient();
	}
}

void ActivationCode::replyClient()
{
	ByteArray packet;
	MessageHead head;
	head.command = CMD_S2C_ACTIVE_CODE;
	head.pack(packet);
	replyMsg.pack(packet);

	// rewrite packet size
	head.packSize = (unsigned short)packet.getSize();
	packet.position = 0;
	head.pack(packet);
	client->send(packet);
	client->flush();
	client = nullptr;
}

void ActivationCode::onActivationUpdated(void* data)
{
	int result = *((int*)data);
	if (result <= 0)
	{
		replyMsg.status = INVALID_CODE;
	}
	replyClient();
}
