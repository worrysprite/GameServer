#ifndef __GAME_EVENT_H__
#define __GAME_EVENT_H__

#include "Event.h"
#include "network/WebsocketClient.h"

using namespace ws;

enum EventType
{
	EVENT_USER_LOGIN
};

struct CommonEvent : public Event
{
	CommonEvent(int t = 0) :Event(t), player(nullptr) { }
	virtual ~CommonEvent(){}
	PlayerData*						player;
};

#endif