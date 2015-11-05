#ifndef __SDK_CONFIG_QUERY_H__
#define __SDK_CONFIG_QUERY_H__

#include "SQLRequest.h"

class SDKConfigQuery : public SQLRequest
{
public:
	void querySDKConfig(unsigned char version, unsigned char platform);
	virtual void onRequest(Database& db);

private:
	unsigned char version;
	unsigned char platform;
};

#endif