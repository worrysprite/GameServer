#ifndef __WS_UTILS_PROFILER_H__
#define __WS_UTILS_PROFILER_H__

namespace ws
{
	namespace utils
	{
		/*
		* Author:  David Robert Nadeau
		* Site:    http://NadeauSoftware.com/
		* License: Creative Commons Attribution 3.0 Unported License
		*          http://creativecommons.org/licenses/by/3.0/deed.en_US
		*/
		class Profiler
		{
		public:
			/**
			* Returns the peak (maximum so far) resident set size (physical
			* memory use) measured in bytes, or zero if the value cannot be
			* determined on this OS.
			*/
			static size_t getPeakRSS();

			/**
			* Returns the current resident set size (physical memory use) measured
			* in bytes, or zero if the value cannot be determined on this OS.
			*/
			static size_t getCurrentRSS();

		private:
		};
	}
}

#endif