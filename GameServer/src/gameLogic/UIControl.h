#ifndef __UI_CONTROL_H__
#define __UI_CONTROL_H__

#include "Message.h"
#include "Client.h"
#include "database/UIConfigQuery.h"

class UIControl
{
public:
	static UIControl* getInstance();
	void processUIConfig(Client* client, UIConfigMessage* msg);

private:
	UIControl();
	static UIControl* _instance;

	Client* client;
	UIConfigQuery* queryRequest;

	void onUIConfigGet(void* data);
};

#endif