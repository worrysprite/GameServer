#ifndef __WS_EVENT_H__
#define __WS_EVENT_H__

#include <functional>
#include <map>
#include <set>

namespace ws
{
	struct Event
	{
		Event(int t = 0) :type(t){}
		virtual ~Event(){}
		int				type;
	};

	typedef std::function<void(const Event&)> EventCallback;

	class EventDispatcher
	{
	public:
		EventDispatcher();
		virtual ~EventDispatcher();

		virtual void	addEventListener(int type, EventCallback* callback);
		virtual void	removeEventListener(int type, EventCallback* callback);
		virtual void	dispatchEvent(const Event& event);
	protected:
		std::map<int, std::set<EventCallback*>>			listeners;
	};
}

#endif