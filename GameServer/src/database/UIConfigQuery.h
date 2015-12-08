#ifndef __UI_CONFIG_QUERY_H__
#define __UI_CONFIG_QUERY_H__

#include "Database.h"
#include "Message.h"

class UIConfigQuery : public ws::DBRequest
{
public:
	typedef std::function<void(UIConfigMessage*)> CallbackType;
	void queryUIConfig(unsigned char version, unsigned char platform, CallbackType callback);
	virtual void onRequest(Database& db);
	virtual void onFinish();

private:
	unsigned char version;
	unsigned char platform;
	UIConfigMessage* config;
	CallbackType callback;
};

#endif