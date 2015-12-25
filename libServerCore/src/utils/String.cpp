#include "String.h"
#include <string.h>
#include <ctype.h>

namespace ws
{
	namespace utils
	{
		void String::split(char* str, const char* seperator, std::vector<char*>& output)
		{
			size_t sepLength = strlen(seperator);
			output.push_back(str);
			while (*str++ != '\0')
			{
				if (strncmp(str, seperator, sepLength) == 0)
				{
					*str = '\0';
					str += sepLength;
					output.push_back(str);
				}
			}
		}

		void String::split(const char* str, const char* seperator, std::vector<std::string>& output)
		{
			size_t sepLength = strlen(seperator);

			const char* ptr = str;
			while (*str++ != '\0')
			{
				if (strncmp(str, seperator, sepLength) == 0)
				{
					output.push_back(std::string(ptr, str - ptr));
					str += sepLength;
					ptr = str;
				}
			}
			output.push_back(std::string(ptr));
		}

		void String::toLowercase(char* str)
		{
			while (*str != '\0')
			{
				*str = tolower(*str);
				++str;
			}
		}

		void String::toUppercase(char* str)
		{
			while (*str != '\0')
			{
				*str = toupper(*str);
				++str;
			}
		}

	}
}
