#ifndef __UI_CONTROL_H__
#define __UI_CONTROL_H__

#include "Message.h"
#include "Client.h"
#include "database/UIConfigQuery.h"

class UIControl
{
public:
	static UIControl* getInstance();
	void processUIConfig(long long clientID, UIConfigMessage* msg);

private:
	UIControl();
	static UIControl* _instance;

	void onUIConfigGet(long long clientID, UIConfigMessage* data);
};

#endif