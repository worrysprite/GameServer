#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "ServerSocket.h"
#include "Message.h"

using namespace ws;

class Client : public ClientSocket
{
public:
	Client();
	~Client();
	void onRecv();

private:
	MessageHead* pHead;

	void parseWebsocketHandshake();
};

#endif