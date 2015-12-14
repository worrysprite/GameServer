#ifndef __WS_UTILS_TIMER_H__
#define __WS_UTILS_TIMER_H__
#include <chrono>
#include <list>
#include <thread>
#include <functional>
#include <mutex>

using namespace std::chrono;

namespace ws
{
	namespace utils
	{
		class Scedule
		{
		public:
			Scedule();
			virtual ~Scedule();

			steady_clock::time_point	lastTime;
			milliseconds				interval;
			unsigned int				repeat;
			std::function<void()>		callback;
		};

		class Timer
		{
		public:
			typedef std::function<void()> CallbackType;
			Timer();
			virtual ~Timer();

			Scedule*					addTimeCall(milliseconds time, CallbackType callback, unsigned int repeat = 0xFFFFFFFF);
			void						removeTimeCall(Scedule* scedule);
			void						update();

		protected:
			bool						isExit;
			std::thread*				timerThread;
			std::mutex					sceduleMtx;
			std::list<Scedule*>			sceduleList;
			std::mutex					callbackMtx;
			std::list<CallbackType>		callbackList;

			void						timerProc();
		};
	}
}

#endif