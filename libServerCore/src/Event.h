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

	class EventDispatcher
	{
	public:
		typedef void(*EventCallback)(const Event&);
		//typedef std::function<void(const Event&)> EventCallback;

		EventDispatcher();
		~EventDispatcher();

		void	addEventListener(int type, EventCallback callback);
		void	removeEventListener(int type, EventCallback callback);
		void	dispatchEvent(const Event& event);
	protected:
		std::map<int, std::set<EventCallback>>		listeners;
	};
}

#endif