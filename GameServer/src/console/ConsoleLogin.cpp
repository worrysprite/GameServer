#include <assert.h>
#include "ConsoleLogin.h"
#include "GameServer.h"
#include "database/ConsoleLoginQuery.h"

ConsoleLogin* ConsoleLogin::_instance = nullptr;

ConsoleLogin* ConsoleLogin::getInstance()
{
	if (_instance == nullptr)
	{
		_instance = new ConsoleLogin;
	}
	return _instance;
}

void ConsoleLogin::processLogin(long long clientID, ConsoleLoginMessage* msg)
{
	assert(msg != nullptr);
	ConsoleLoginQuery* query = new ConsoleLoginQuery;
	query->queryUserinfo(*msg, std::bind(&ConsoleLogin::onLoginGet, this, clientID, std::placeholders::_1));
	GameServer::getInstance()->getDBQueue()->addQueueMsg(std::shared_ptr<DBRequest>(query));
}

void ConsoleLogin::onLoginGet(long long clientID, ConsoleLoginMessage* msg)
{
	if (msg->id)
	{
		auto client(GameServer::getInstance()->getConsoleServer()->getClient(clientID));
		if (!client)
		{
			Log::w("client is disconnect before sql execute");
			return;
		}

		ByteArray packet;
		MessageHead head;
		head.command = CMD_CONSOLE_LOGIN;
		head.pack(packet);
		msg->pack(packet);

		// rewrite packet size
		head.packSize = (unsigned short)packet.getSize();
		packet.position = 0;
		head.pack(packet);

		client->send(packet);
		client->flush();
	}
	else
	{
		GameServer::getInstance()->getConsoleServer()->kickClient(clientID);
	}
}
