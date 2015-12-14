#include "ConsoleProfiler.h"
#include "GameServer.h"
#include "Message.h"
#include "utils/Profiler.h"

ConsoleProfiler* ConsoleProfiler::_instance = nullptr;

ConsoleProfiler* ConsoleProfiler::getInstance()
{
	if (_instance == nullptr)
	{
		_instance = new ConsoleProfiler;
		GameServer::getInstance()->getTimer()->addTimeCall(std::chrono::seconds(1), std::bind(&ConsoleProfiler::onTimer1000, _instance));
	}
	return _instance;
}

ConsoleProfiler::ConsoleProfiler()
{

}

ConsoleProfiler::~ConsoleProfiler()
{

}

void ConsoleProfiler::subscribeProfiler(long long clientID)
{
	auto iter = allSubscribe.find(clientID);
	if (iter == allSubscribe.end())
	{
		allSubscribe.insert(clientID);
	}
}

void ConsoleProfiler::unsubscribeProfiler(long long clientID)
{
	auto iter = allSubscribe.find(clientID);
	if (iter != allSubscribe.end())
	{
		allSubscribe.erase(iter);
	}
}

void ConsoleProfiler::onTimer1000()
{
	Log::v("on timer 1000");
	ServerSocket* logicServer = GameServer::getInstance()->getLogicServer();
	ServerSocket* consoleServer = GameServer::getInstance()->getConsoleServer();
	DBQueue* dbQueue = GameServer::getInstance()->getDBQueue();

	ByteArray packet;
	MessageHead head;
	head.command = CMD_CONSOLE_SUBSCRIBE;
	head.pack(packet);

	ConsoleSubscribeMessage msg;
	msg.memoryPeak = Profiler::getPeakRSS();
	msg.memoryUsed = Profiler::getCurrentRSS();
	msg.numDBRequests = dbQueue->getQueueLength();
	msg.numOnline = logicServer->numOnlines();
#ifdef _WIN32
	msg.ioDataPoolSize = logicServer->getIODataPoolSize();
	msg.ioDataPostedSize = logicServer->getIODataPostedSize();
#endif
	msg.pack(packet);
	head.packSize = packet.getSize();
	packet.position = 0;
	head.pack(packet);

	auto iter = allSubscribe.begin();
	while (iter != allSubscribe.end())
	{
		long long clientID = *iter;
		auto client = consoleServer->getClient(clientID);
		if (!client)
		{
			iter = allSubscribe.erase(iter);
		}
		else
		{
			client->send(packet);
			client->flush();
			++iter;
		}
	}
}
