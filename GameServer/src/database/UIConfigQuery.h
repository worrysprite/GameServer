#ifndef __UI_CONFIG_QUERY_H__
#define __UI_CONFIG_QUERY_H__

#include "SQLRequest.h"

class UIConfigQuery : public SQLRequest
{
public:
	void queryUIConfig(unsigned char version, unsigned char platform);
	virtual void onRequest(Database& db);

private:
	unsigned char version;
	unsigned char platform;
};

#endif