#include "Log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

namespace ws
{
	ws::LogLevel Log::level = _ERROR_;

	void Log::v(const char* format, ...)
	{
		if (level <= _VERBOSE_)
		{
			va_list args;
			va_start(args, format);
			printOut("V", format, args);
			va_end(args);
		}
	}

	void Log::d(const char* format, ...)
	{
		if (level <= _DEBUG_)
		{
			va_list args;
			va_start(args, format);
			printOut("D", format, args);
			va_end(args);
		}
	}

	void Log::i(const char* format, ...)
	{
		if (level <= _INFO_)
		{
			va_list args;
			va_start(args, format);
			printOut("I", format, args);
			va_end(args);
		}
	}

	void Log::w(const char* format, ...)
	{
		if (level <= _WARN_)
		{
			va_list args;
			va_start(args, format);
			printOut("W", format, args);
			va_end(args);
		}
	}

	void Log::e(const char* format, ...)
	{
		if (level <= _ERROR_)
		{
			va_list args;
			va_start(args, format);
			printOut("E", format, args);
			va_end(args);
		}
	}

	void Log::getTime(char pout[CTIME_SIZE])
	{
		time_t t(time(NULL));
		char* time = ctime(&t);
		memcpy(pout, time, CTIME_SIZE - 1);
		pout[CTIME_SIZE - 1] = '\0';
	}

	void Log::printOut(const char* level, const char* format, va_list valist)
	{
		char time[CTIME_SIZE];
		getTime(time);
		char buffer[1024] = {0};
		sprintf(buffer, "%s %s %s\n", time, level, format);
		vprintf(buffer, valist);
		fflush(stdout);
	}
}
