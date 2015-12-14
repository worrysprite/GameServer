#ifndef __WS_UTILS_LOG_H__
#define __WS_UTILS_LOG_H__

#include <stdarg.h>

namespace ws
{
	namespace utils
	{
#define CTIME_SIZE 25

		enum LogLevel
		{
			_VERBOSE_,
			_DEBUG_,
			_INFO_,
			_WARN_,
			_ERROR_
		};

		class Log
		{
		public:
			static LogLevel level;

			static void v(const char* format, ...);
			static void d(const char* format, ...);
			static void i(const char* format, ...);
			static void w(const char* format, ...);
			static void e(const char* format, ...);

			static void getTime(char pout[CTIME_SIZE]);

		private:
			static void printOut(const char* format, const char* level, va_list valist);
		};
	}
}
#endif