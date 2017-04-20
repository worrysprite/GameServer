#include "String.h"
#include "Math.h"
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <openssl/md5.h>

namespace ws
{
	namespace utils
	{
		void String::split(char* str, const char* seperator, std::vector<char*>& output)
		{
			size_t strLen = strlen(str);
			if (!strLen)
			{
				return;
			}
			size_t sepLength = strlen(seperator);
			output.push_back(str);
			while (*str != '\0' && strLen >= sepLength)
			{
				if (strncmp(str, seperator, sepLength) == 0)
				{
					*str = '\0';
					str += sepLength;
					strLen -= sepLength;
					output.push_back(str);
				}
				else
				{
					++str;
					--strLen;
				}
			}
		}

		void String::split(const char* str, const char* seperator, std::vector<std::string>& output)
		{
			size_t strLen = strlen(str);
			if (!strLen)
			{
				return;
			}
			size_t sepLength = strlen(seperator);
			const char* ptr = str;
			while (*str != '\0' && strLen >= sepLength)
			{
				if (strncmp(str, seperator, sepLength) == 0)
				{
					output.push_back(std::string(ptr, str - ptr));
					str += sepLength;
					ptr = str;
					strLen -= sepLength;
				}
				else
				{
					++str;
					--strLen;
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

		bool String::isPrintableString(const char* str)
		{
			while (*str != '\0')
			{
				if (*str < ' ' || *str > '~')
				{
					return false;
				}
				++str;
			}
			return true;
		}

		time_t String::formatTime(const std::string& time)
		{
			std::vector<std::string> tmp;
			split(time.c_str(), " ", tmp);
			std::vector<std::string> dates, times;
			split(tmp[0].c_str(), "/", dates);
			if (dates.size() != 3)
			{
				return 0;
			}
			if (tmp.size() > 1)
			{
				split(tmp[1].c_str(), ":", times);
				if (times.size() != 3)
				{
					return 0;
				}
			}
			tm result = {0};
			result.tm_year = atoi(dates[0].c_str()) - 1900;
			result.tm_mon = atoi(dates[1].c_str()) - 1;
			result.tm_mday = atoi(dates[2].c_str());
			if (!times.empty())
			{
				result.tm_hour = atoi(times[0].c_str());
				result.tm_min = atoi(times[1].c_str());
				result.tm_sec = atoi(times[2].c_str());
			}
			return mktime(&result);
		}

		std::string String::formatTime(time_t time)
		{
			if (!time)
			{
				time = ::time(nullptr);
			}
			char buff[30] = {0};
			tm date = {0};
#ifdef _WIN32
			localtime_s(&date, &time);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
			localtime_r(&time, &date);
#endif
			strftime(buff, 30, "%Y/%m/%d %H:%M:%S", &date);
			return std::string(buff);
		}

		std::string String::md5(const std::string input)
		{
			std::string result;
			if (input.empty())
			{
				return result;
			}
			const int digestLen = 16;
			unsigned char digest[digestLen] = { 0 };
			memset(digest, 0x00, sizeof(digest));

			MD5_CTX ctx;
			MD5_Init(&ctx);
			MD5_Update(&ctx, input.c_str(), input.size());
			MD5_Final(digest, &ctx);

			result.clear();
			result.append((char *)digest, digestLen);
			return Bin2Hex(result);
		}

		// 二进制字符串转十六进制字符串(只用于String::md5)
		std::string String::Bin2Hex(std::string input)
		{
			std::string result;
			const char hexdig[] = "0123456789ABCDEF";

			if (input.empty())
			{
				return result;
			}

			result.clear();
			for (std::string::iterator i = input.begin(); i != input.end(); i++)
			{
				result.append(1, hexdig[(*i >> 4) & 0xf]);  //留下高四位
				result.append(1, hexdig[(*i & 0xf)]);  //留下低四位
			}

			return result;
		}

		// 只用于String::URLEncode
		char String::Char2Hex(const char& input)
		{
			return input > 9 ? input - 10 + 'A' : input + '0';
		}

		// 只用于String::URLDecode
		char String::Hex2Char(const char& input)
		{
			return isdigit(input) ? input - '0' : input - 'A' + 10;
		}

		std::string String::URLEncode(const std::string& input)
		{
			std::string result;
			for (size_t ix = 0; ix < input.size(); ix++)
			{
				char buf[4];
				memset(buf, 0, 4);
				if (isalnum((char)input[ix]))
				{
					buf[0] = input[ix];
				}
				else
				{
					buf[0] = '%';
					buf[1] = String::Char2Hex((char)input[ix] >> 4);
					buf[2] = String::Char2Hex((char)input[ix] % 16);
				}
				result += (char *)buf;
			}
			return result;
		}

		std::string String::URLDecode(const std::string& input)
		{
			std::string result;
			for (size_t ix = 0; ix < input.size(); ix++)
			{
				char ch = 0;
				if (input[ix] == '%')
				{
					ch = (String::Hex2Char(input[ix + 1]) << 4);
					ch |= String::Hex2Char(input[ix + 2]);
					ix += 2;
				}
				else if (input[ix] == '+')
				{
					ch = ' ';
				}
				else
				{
					ch = input[ix];
				}
				result += (char)ch;
			}
			return result;
		}

		const char rndChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
		std::string String::random(unsigned char length, const char* chars)
		{
			if (!length)
			{
				return std::string();
			}
			if (!chars)
			{
				chars = rndChars;
			}
			unsigned int maxLen = (unsigned int)strlen(chars);
			std::string result;
			while (result.length() < length)
			{
				unsigned int index = Math::random(maxLen);
				result += chars[index];
			}
			return result;
		}
	}
}
