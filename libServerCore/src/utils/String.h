#ifndef __WS_UTILS_STRING_H__
#define __WS_UTILS_STRING_H__

#include <vector>
#include <string>

namespace ws
{
	namespace utils
	{
		class String
		{
		public:
			static void split(char* str, const char* seperator, std::vector<char*>& output);
			static void split(const char* str, const char* seperator, std::vector<std::string>& output);

		private:
		};
	}
}

#endif