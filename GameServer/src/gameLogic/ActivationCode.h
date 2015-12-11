#ifndef __ACTIVATION_CODE_H__
#define __ACTIVATION_CODE_H__

#include "Message.h"
#include "Client.h"
#include "database/ActivationCodeQuery.h"
#include <map>

class ActivationCode
{
public:
	static ActivationCode* getInstance();
	void processActivationCode(long long clientID, ActivationMessage* msg);

private:
	ActivationCode();
	static ActivationCode* _instance;

	void onActivationInfoGet(long long clientID, ActivationMessage* data);
	void onActivationUpdated(long long clientID, ActivationMessage* data);
	void replyClient(ClientSocket* client, ActivationMessage* replyMsg);
};

#endif