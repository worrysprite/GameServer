#ifndef __WS_ACTOR_H__
#define __WS_ACTOR_H__

#include <list>
#include <mutex>
#include <thread>
#include <memory>

namespace ws
{
	class ActorEvent
	{
	public:
		ActorEvent() {}
		virtual ~ActorEvent() {}
	};

	class Actor
	{
	public:
		Actor();
		virtual ~Actor();

		virtual void sendEvent(std::shared_ptr<const ActorEvent> evt);
		virtual void update();

	protected:
		std::mutex evtLock;
		std::list<std::shared_ptr<const ActorEvent>> eventList;

		virtual void processEvent(const ActorEvent* evt);
	};

	class WorkerEvent : public ActorEvent
	{
	public:
		std::function<void()> func;
	};

	class Worker : public Actor
	{
	public:
		Worker();
		virtual ~Worker();

	protected:
		virtual void processEvent(const ActorEvent* evt);

	private:
		std::thread* th;
		bool isRunning;

		void work();
	};
}

#endif
