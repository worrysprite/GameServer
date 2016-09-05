#include "Event.h"

namespace ws
{
	EventDispatcher::EventDispatcher()
	{

	}

	EventDispatcher::~EventDispatcher()
	{

	}

	void EventDispatcher::addEventListener(int type, EventCallback callback)
	{
		listeners[type].insert(callback);
	}

	void EventDispatcher::removeEventListener(int type, EventCallback callback)
	{
		auto iter = listeners.find(type);
		if (iter != listeners.end())
		{
			iter->second.erase(callback);
		}
	}

	void EventDispatcher::dispatchEvent(const Event& event)
	{
		auto iter = listeners.find(event.type);
		if (iter != listeners.end())
		{
			std::set<EventCallback>& cbList = iter->second;
			for (const EventCallback& callback : cbList)
			{
				callback(event);
			}
		}
	}
}
