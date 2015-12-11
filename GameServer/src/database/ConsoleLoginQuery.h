#ifndef __CONSOLE_LOGIN_QUERY_H__
#define __CONSOLE_LOGIN_QUERY_H__

#include "Database.h"
#include "Message.h"

class ConsoleLoginQuery : public ws::DBRequest
{
public:
	ConsoleLoginQuery();
	~ConsoleLoginQuery();
	typedef std::function<void(ConsoleLoginMessage*)> CallbackType;

	void queryUserinfo(const ConsoleLoginMessage& msg, CallbackType callback);
	virtual void onRequest(Database& db);
	virtual void onFinish();

private:
	ConsoleLoginMessage* loginMsg;
	CallbackType callback;
};

#endif