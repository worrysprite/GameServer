#ifndef __SERVER_H__
#define __SERVER_H__

#include "ClientManager.h"
#include "ServerSocket.h"
#include "Database.h"

using namespace ws;

class Server : public ClientManager
{
public:
	Server();
	virtual ~Server();

	int startListen(int port);

protected:
	virtual ClientSocket* createClient();
	virtual void destroyClient(ClientSocket* cs);
};

#endif