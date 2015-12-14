#ifndef __CONSOLE_PROFILER_H__
#define __CONSOLE_PROFILER_H__

#include <set>

class ConsoleProfiler
{
public:
	static ConsoleProfiler* getInstance();
	~ConsoleProfiler();

	void subscribeProfiler(long long clientID);
	void unsubscribeProfiler(long long clientID);

private:
	ConsoleProfiler();
	static ConsoleProfiler* _instance;
	std::set<long long> allSubscribe;

	void onTimer1000();
};

#endif