#ifndef __WS_UTILS_TIME_TOOL_H__
#define __WS_UTILS_TIME_TOOL_H__

#include <stdint.h>
#include <chrono>
#include "Log.h"

using namespace std::chrono;

namespace ws
{
	namespace utils
	{
		class TimeTool
		{
		public:
			// get unix stamp in milliseconds
			template<class ClockType>
			inline static uint64_t getUnixtime(){ return (uint64_t)duration_cast<milliseconds>(ClockType::now().time_since_epoch()).count(); }

			static void LocalTime(uint64_t time, tm& date)
			{
				time_t t = time / 1000;
#ifdef _WIN32
				localtime_s(&date, &t);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
				localtime_r(&t, &date);
#endif
			}

			// get zero hour time of time
			static uint64_t getZeroHourOfTime(uint64_t time = 0)
			{
				if (!time)
				{
					time = getUnixtime<system_clock>();
				}
				tm date;
				LocalTime(time, date);
				date.tm_hour = 0;
				date.tm_min = 0;
				date.tm_sec = 0;
				return mktime(&date) * 1000ULL;
			}

			// get first day timestamp of month
			static uint64_t getFirstDayOfMonth(uint64_t time = 0)
			{
				if (!time)
				{
					time = getUnixtime<system_clock>();
				}
				tm date;
				LocalTime(time, date);
				date.tm_mday = 1;
				date.tm_hour = 0;
				date.tm_min = 0;
				date.tm_sec = 0;
				return mktime(&date) * 1000ULL;
			}

			// 判断一个时间是否是昨天或更早之前
			static bool isYesterdayBefore(uint64_t time, uint64_t offset = 0)
			{
				if (!time)
				{
					return true;
				}
				uint64_t now = getUnixtime<system_clock>();
				if (time >= now)
				{
					return false;
				}
				uint64_t diff = now - offset - getZeroHourOfTime(time - offset);
				return diff > 24 * 3600000;
			}

			// 获取当前时间在本月的第N天
			static uint8_t								getDayofMonth(uint64_t time)
			{
				tm date = {0};
				LocalTime(time, date);
				return date.tm_mday;
			}
		};
	}
}

#endif