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
			/************************************************************************/
			/* 按seperator分割str，并将结果存入output，将会修改str                    */
			/************************************************************************/
			static void split(char* str, const char* seperator, std::vector<char*>& output);
			/************************************************************************/
			/* 按seperator分割str，并将结果存入output，不会修改str                    */
			/************************************************************************/
			static void split(const char* str, const char* seperator, std::vector<std::string>& output);

			static void toLowercase(char* str);
			static void toUppercase(char* str);
			/************************************************************************/
			/* 判断字符串是否为可打印ASCII码                                          */
			/************************************************************************/
			static bool isPrintableString(const char* str);
			/************************************************************************/
			/* 将一个时间字符串格式化为unix timestamp，格式为yyyy/mm/dd hh:mm:ss      */
			/************************************************************************/
			static time_t formatTime(const std::string& time);
			/************************************************************************/
			/* 将一个unix timestamp转化为时间字符串，格式为yyyy/mm/dd hh:mm:ss        */
			/************************************************************************/
			static std::string formatTime(time_t time = 0);

			static std::string md5(const std::string input);

			static std::string URLEncode(const std::string& input);
			static std::string URLDecode(const std::string& input);
			/************************************************************************/
			/* 生成随机字符串，length指定长度，chars指定取值内容，默认为大小写字母和数字 */
			/************************************************************************/
			static std::string random(unsigned char length, const char* chars = nullptr);

		private:
			static std::string Bin2Hex(std::string input);
			static char Char2Hex(const char& input);
			static char Hex2Char(const char& input);
		};
	}
}

#endif