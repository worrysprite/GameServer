#ifndef __SDK_CONFIG_QUERY_H__
#define __SDK_CONFIG_QUERY_H__

#include "Database.h"
#include "Message.h"

class SDKConfigQuery : public ws::DBRequest
{
public:
	void querySDKConfig(unsigned char version, unsigned char platform, std::function<void(SDKConfigMessage*)> callback);
	virtual void onRequest(Database& db);
	virtual void onFinish();

private:
	unsigned char version;
	unsigned char platform;
	SDKConfigMessage* config;
	std::function<void(SDKConfigMessage*)> callback;
};

#endif