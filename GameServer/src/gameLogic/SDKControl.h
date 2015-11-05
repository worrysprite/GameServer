#ifndef __SDK_CONTROL_H__
#define __SDK_CONTROL_H__

#include "Message.h"
#include "Client.h"
#include "database/SDKConfigQuery.h"

class SDKControl
{
public:
	static SDKControl* getInstance();
	void processSDKConfig(Client* client, SDKConfigMessage* msg);

private:
	SDKControl();
	static SDKControl* _instance;

	Client* client;
	SDKConfigQuery* queryRequest;

	void onSDKConfigGet(void* data);
};

#endif