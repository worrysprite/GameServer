#ifndef __CONSOLE_CLIENT_H__
#define __CONSOLE_CLIENT_H__

#include "ServerSocket.h"
#include "Message.h"

class ConsoleClient : public ws::ClientSocket
{
public:
	ConsoleClient();
	~ConsoleClient();
	void onRecv();

private:
	MessageHead* pHead;
};

#endif