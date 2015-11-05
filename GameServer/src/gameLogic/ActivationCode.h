#ifndef __ACTIVATION_CODE_H__
#define __ACTIVATION_CODE_H__

#include "Message.h"
#include "Client.h"
#include "database/ActivationCodeQuery.h"

class ActivationCode
{
public:
	static ActivationCode* getInstance();
	void processActivationCode(Client* client, ActivationMessage* msg);

private:
	ActivationCode();
	static ActivationCode* _instance;

	Client* client;
	ActivationCodeQuery* queryRequest;
	ActivationCodeQuery* updateRequest;
	ActivationMessage replyMsg;

	void onActivationInfoGet(void* data);
	void onActivationUpdated(void* data);
	void replyClient();
};



#endif