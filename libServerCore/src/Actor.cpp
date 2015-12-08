#include "Actor.h"

namespace ws
{
	Actor::Actor()
	{

	}

	Actor::~Actor()
	{

	}

	void Actor::sendEvent(std::shared_ptr<const ActorEvent> evt)
	{
		evtLock.lock();
		eventList.push_back(evt);
		evtLock.unlock();
	}

	void Actor::update()
	{
		std::list<std::shared_ptr<const ActorEvent>> tmpList;
		evtLock.lock();
		eventList.swap(tmpList);
		evtLock.unlock();
		for (auto evt : tmpList)
		{
			processEvent(evt.get());
		}
	}

	void Actor::processEvent(const ActorEvent* evt)
	{

	}

	Worker::Worker() : th(nullptr), isRunning(true)
	{
		th = new std::thread(std::bind(&Worker::work, this));
	}

	Worker::~Worker()
	{
		isRunning = false;
		th->join();
		delete th;
	}

	void Worker::processEvent(const ActorEvent* evt)
	{
		const WorkerEvent* workerEvt = dynamic_cast<const WorkerEvent*>(evt);
		if (workerEvt)
		{
			workerEvt->func();
		}
	}

	void Worker::work()
	{
		while (isRunning)
		{
			update();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}