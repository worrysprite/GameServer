#include "Timer.h"

namespace ws
{
	namespace utils
	{
		//=======================Scedule implements========================
		Scedule::Scedule() :repeat(0), callback(nullptr)
		{

		}

		Scedule::~Scedule()
		{

		}

		//=======================Timer implements========================
		Timer::Timer() : timerThread(nullptr), isExit(false)
		{
			timerThread = new std::thread(std::bind(&Timer::timerProc, this));
		}

		Timer::~Timer()
		{
			this->isExit = true;
			timerThread->join();
			delete timerThread;
			timerThread = nullptr;
		}

		Scedule* Timer::addTimeCall(milliseconds time, std::function<void()> callback, unsigned int repeat /*= 0xFFFFFFFF*/)
		{
			Scedule* timer = new Scedule;
			timer->interval = time;
			timer->callback = callback;
			timer->repeat = repeat;
			timer->lastTime = steady_clock::now();
			sceduleMtx.lock();
			sceduleList.push_back(timer);
			sceduleMtx.unlock();
			return timer;
		}

		void Timer::removeTimeCall(Scedule* scedule)
		{
			sceduleMtx.lock();
			sceduleList.remove(scedule);
			sceduleMtx.unlock();
			delete scedule;
		}

		void Timer::update()
		{
			std::list<std::function<void()>> tmpList;
			callbackMtx.lock();
			callbackList.swap(tmpList);
			callbackMtx.unlock();
			for (auto callback : tmpList)
			{
				callback();
			}
		}

		void Timer::timerProc()
		{
			while (!isExit)
			{
				if (sceduleList.size() > 0)
				{
					sceduleMtx.lock();
					callbackMtx.lock();
					steady_clock::time_point now = steady_clock::now();
					for (auto iter = sceduleList.begin(); iter != sceduleList.end();)
					{
						Scedule* timer(*iter);
						bool remove(false);
						while (timer->lastTime + timer->interval <= now)
						{
							timer->lastTime += timer->interval;
							callbackList.push_back(timer->callback);
							if (timer->repeat > 0)
							{
								--timer->repeat;
							}
							else
							{
								remove = true;
								break;
							}
						}
						if (remove)
						{
							iter = sceduleList.erase(iter);
						}
						else
						{
							++iter;
						}
					}
					callbackMtx.unlock();
					sceduleMtx.unlock();
				}
				std::this_thread::sleep_for(milliseconds(1));
			}
		}
	}
}
