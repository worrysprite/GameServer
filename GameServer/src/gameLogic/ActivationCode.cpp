#include <assert.h>
#include "ActivationCode.h"
#include "Log.h"

extern ServerSocket* gServer;
extern DBQueue* gDBQueue;

ActivationCode* ActivationCode::_instance = nullptr;

ActivationCode::ActivationCode()
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

void ActivationCode::processActivationCode(long long clientID, ActivationMessage* msg)
{
	assert(msg != NULL);
	ActivationCodeQuery* queryRequest = new ActivationCodeQuery;
	queryRequest->queryActivationCode(msg->code, std::bind(&ActivationCode::onActivationInfoGet, this, clientID, std::placeholders::_1));
	gDBQueue->addQueueMsg(std::shared_ptr<ActivationCodeQuery>(queryRequest));
}

void ActivationCode::onActivationInfoGet(long long clientID, ActivationMessage* data)
{
	auto client(gServer->getClient(clientID));
	if (!client)
	{
		Log::w("client is disconnect before sql execute");
		return;
	}
	if (data->status == 0)
	{
		ActivationCodeQuery* updateRequest = new ActivationCodeQuery;
		updateRequest->updateActivationStatus(*data, ActivationStatus::USED_CODE,
							std::bind(&ActivationCode::onActivationUpdated, this, clientID, std::placeholders::_1));
		gDBQueue->addQueueMsg(std::shared_ptr<ActivationCodeQuery>(updateRequest));
	}
	else
	{
		replyClient(client, data);
	}
}

void ActivationCode::replyClient(ClientSocket* client, ActivationMessage* replyMsg)
{
	ByteArray packet;
	MessageHead head;
	head.command = CMD_S2C_ACTIVE_CODE;
	head.pack(packet);
	replyMsg->pack(packet);

	// rewrite packet size
	head.packSize = (unsigned short)packet.getSize();
	packet.position = 0;
	head.pack(packet);
	client->send(packet);
	client->flush();
}

void ActivationCode::onActivationUpdated(long long clientID, ActivationMessage* data)
{
	auto client(gServer->getClient(clientID));
	if (client)
	{
		replyClient(client, data);
	}
	else
	{
		Log::w("client is disconnect before sql execute");
	}
}
