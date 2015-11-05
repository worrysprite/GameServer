#ifndef __WS_TIMER_H__
#define __WS_TIMER_H__
#include <chrono>
#include <list>
#include <thread>
#include <functional>
#include <mutex>

using namespace std::chrono;

namespace ws
{
	class Scedule
	{
	public:
		Scedule();
		virtual ~Scedule();

		steady_clock::time_point lastTime;
		milliseconds interval;
		unsigned int repeat;
		std::function<void()> callback;
	};

	class Timer
	{
	public:
		Timer();
		virtual ~Timer();

		Scedule*	addTimeCall(milliseconds time, std::function<void()> callback, unsigned int repeat = 0xFFFFFFFF);
		void		removeTimeCall(Scedule* scedule);
		void		update();

	protected:
		bool isExit;
		std::thread* timerThread;
		std::mutex sceduleMtx;
		std::list<Scedule*> sceduleList;
		std::mutex callbackMtx;
		std::list<std::function<void()>> callbackList;

		void	timerProc();
	};
}

#endif