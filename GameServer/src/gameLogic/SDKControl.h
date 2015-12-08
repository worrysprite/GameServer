#ifndef __SDK_CONTROL_H__
#define __SDK_CONTROL_H__

#include "Message.h"
#include "Client.h"
#include "database/SDKConfigQuery.h"

class SDKControl
{
public:
	static SDKControl* getInstance();
	void processSDKConfig(long long clientID, SDKConfigMessage* msg);

private:
	SDKControl();
	static SDKControl* _instance;

	void onSDKConfigGet(long long clientID, SDKConfigMessage* data);
};

#endif