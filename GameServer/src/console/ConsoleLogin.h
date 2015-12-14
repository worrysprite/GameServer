#ifndef __CONSOLE_LOGIN_H__
#define __CONSOLE_LOGIN_H__

#include "Message.h"

class ConsoleLogin
{
public:
	static ConsoleLogin* getInstance();

	void processLogin(long long clientID, ConsoleLoginMessage* msg);

private:
	ConsoleLogin() { }
	static ConsoleLogin* _instance;
	
	void onLoginGet(long long clientID, ConsoleLoginMessage* msg);
};

#endif