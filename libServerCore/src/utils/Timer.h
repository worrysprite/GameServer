#ifndef __WS_UTILS_TIMER_H__
#define __WS_UTILS_TIMER_H__
#include <chrono>
#include <thread>
#include <functional>
#include <mutex>
#include <set>
#include <list>

using namespace std::chrono;

namespace ws
{
	namespace utils
	{
		//=======================Schedule implements========================
		typedef std::function<void()>	CallbackType;

		template<class ClockType>
		class Schedule
		{
		public:
			Schedule() :repeat(0) {}
			virtual ~Schedule(){}

			time_point<ClockType>		lastTime;
			milliseconds				interval;
			unsigned int				repeat;
			CallbackType				callback;
		};

		//=======================Timer implements========================
		template<class ClockType>
		class Timer
		{
		public:
			typedef Schedule<ClockType>		ScheduleType;

			Timer(milliseconds mid = seconds(60), milliseconds big = seconds(600)) : mediumInterval(mid),
						bigInterval(big), timerThread(nullptr), isExit(false)
			{
				mediumLastCheck = bigLastCheck = ClockType::now();
				timerThread = new std::thread(std::bind(&Timer<ClockType>::timerProc, this));
			}

			virtual ~Timer()
			{
				this->isExit = true;
				timerThread->join();
				delete timerThread;
				timerThread = nullptr;
				for (ScheduleType* schedule : scheduleList)
				{
					delete schedule;
				}
				for (ScheduleType* schedule : mediumScheduleList)
				{
					delete schedule;
				}
				for (ScheduleType* schedule : bigScheduleList)
				{
					delete schedule;
				}
			}

			ScheduleType*	addTimeCall(milliseconds time, CallbackType callback,
					unsigned int repeat = 0xFFFFFFFF, milliseconds delay = milliseconds(0))
			{
				if (!callback)
				{
					return nullptr;
				}
				ScheduleType* schedule = new ScheduleType;
				schedule->interval = time;
				schedule->callback = callback;
				schedule->repeat = repeat;
				schedule->lastTime = ClockType::now() + delay;
				scheduleMtx.lock();
				if (time >= bigInterval)
				{
					bigScheduleList.insert(schedule);
				}
				else if (time >= mediumInterval)
				{
					mediumScheduleList.insert(schedule);
				}
				else
				{
					scheduleList.insert(schedule);
				}
				scheduleMtx.unlock();
				return schedule;
			}

			void removeTimeCall(ScheduleType* schedule)
			{
				if (!schedule)
				{
					return;
				}
				size_t count(0);
				scheduleMtx.lock();
				count += scheduleList.erase(schedule);
				count += mediumScheduleList.erase(schedule);
				count += bigScheduleList.erase(schedule);
				if (count == 1)
				{
					removeList.insert(schedule);
				}
				else if (count == 0)
				{
					Log::e("fatal error! removeTimeCall got a removed schedule");
				}
				else
				{
					Log::e("fatal error! schedule list in timer has overlapped object");
				}
				scheduleMtx.unlock();
			}

			void update()
			{
				std::list<CallbackType> tmpList;
				callbackMtx.lock();
				callbackList.swap(tmpList);
				callbackMtx.unlock();
				for (CallbackType& callback : tmpList)
				{
					callback();
				}
				std::set<ScheduleType*> tmpRemoveList;
				scheduleMtx.lock();
				removeList.swap(tmpRemoveList);
				scheduleMtx.unlock();
				for (ScheduleType* schedule : tmpRemoveList)
				{
					delete schedule;
				}
			}

		protected:
			bool							isExit;
			std::thread*					timerThread;
			std::mutex						scheduleMtx;
			std::set<ScheduleType*>			scheduleList;
			std::set<ScheduleType*>			mediumScheduleList;
			std::set<ScheduleType*>			bigScheduleList;
			std::set<ScheduleType*>			removeList;
			std::mutex						callbackMtx;
			std::list<CallbackType>			callbackList;

			milliseconds					mediumInterval;
			milliseconds					bigInterval;
			time_point<ClockType>			mediumLastCheck;		//中间隔计时上次触发时间
			time_point<ClockType>			bigLastCheck;			//大间隔计时上次触发时间

			void timerProc()
			{
				while (!isExit)
				{
					scheduleMtx.lock();
					callbackMtx.lock();
					time_point<ClockType> now = ClockType::now();
					// check big list
					if (now - bigLastCheck >= bigInterval)
					{
						bigLastCheck = now;
						//Log::d("check big list");
						auto iter = bigScheduleList.begin();
						while (iter != bigScheduleList.end())
						{
							ScheduleType* schedule(*iter);
							auto remain = schedule->lastTime + schedule->interval - now;
							if (remain < bigInterval)
							{
								iter = bigScheduleList.erase(iter);
								if (remain < mediumInterval)
								{
									scheduleList.insert(schedule);
									//Log::d("big move to small list");
								}
								else
								{
									mediumScheduleList.insert(schedule);
									//Log::d("big move to medium list");
								}
							}
							else
							{
								++iter;
							}
						}
					}
					// check medium list
					if (now - mediumLastCheck >= mediumInterval)
					{
						mediumLastCheck = now;
						//Log::d("check medium list");
						auto iter = mediumScheduleList.begin();
						while (iter != mediumScheduleList.end())
						{
							ScheduleType* schedule(*iter);
							if (schedule->lastTime + schedule->interval - now < mediumInterval)
							{
								iter = mediumScheduleList.erase(iter);
								scheduleList.insert(schedule);
								//Log::d("medium move to small list");
							}
							else
							{
								++iter;
							}
						}
					}
					// check small list
					for (auto iter = scheduleList.begin(); iter != scheduleList.end();)
					{
						ScheduleType* schedule(*iter);
						bool isFinished(false);
						bool isTrigger(false);
						while (schedule->lastTime + schedule->interval <= now)
						{
							isTrigger = true;
							schedule->lastTime += schedule->interval;
							callbackList.push_back(schedule->callback);
							if (schedule->repeat > 0)
							{
								--schedule->repeat;
							}
							else
							{
								isFinished = true;
								break;
							}
						}
						if (isFinished)
						{
							Log::d("time's up! pointer=%16X, interval=%d, repeat=%d", (intptr_t)schedule, schedule->interval.count(), schedule->repeat);
							iter = scheduleList.erase(iter);
							removeList.insert(schedule);
						}
						else
						{
							if (isTrigger)
							{
								if (schedule->interval > bigInterval)
								{
									iter = scheduleList.erase(iter);
									bigScheduleList.insert(schedule);
									//Log::d("small move to big list");
									continue;
								}
								else if (schedule->interval > mediumInterval)
								{
									iter = scheduleList.erase(iter);
									mediumScheduleList.insert(schedule);
									//Log::d("small move to medium list");
									continue;
								}
							}
							++iter;
						}
					}
					callbackMtx.unlock();
					scheduleMtx.unlock();
					std::this_thread::sleep_for(milliseconds(1));
				}
			}
		};
	}
}

#endif