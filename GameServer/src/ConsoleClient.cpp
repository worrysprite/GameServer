#include "ConsoleClient.h"
#include "console/ConsoleLogin.h"
#include "console/ConsoleProfiler.h"

using ws::utils::Log;

ConsoleClient::ConsoleClient() :pHead(nullptr)
{

}

ConsoleClient::~ConsoleClient()
{
	if (pHead)
	{
		delete pHead;
		pHead = nullptr;
	}
}

void ConsoleClient::onRecv()
{
	const size_t headSize = 4;
	while (readBuffer->available() > 0)
	{
		if (pHead == NULL)	//先解析一个消息头
		{
			if (readBuffer->available() >= headSize)
			{
				pHead = new MessageHead;
				pHead->unpack(*readBuffer);
			}
			else
			{
				return;
			}
		}
		if (pHead->packSize > MESSAGE_MAX_SIZE ||
			pHead->command < CMD_CONSOLE_LOGIN ||
			pHead->command >= CMD_CONSOLE_MAX)
		{
			isClosing = true;
			return;
		}
		if (readBuffer->available() < pHead->packSize - headSize)	//不够消息体大小
		{
			return;
		}
		Message* msg = NULL;
		Log::v("console recv command: %d", pHead->command);
		switch (pHead->command)
		{
		case CMD_CONSOLE_LOGIN:
			msg = new ConsoleLoginMessage;
			msg->unpack(*readBuffer);
			ConsoleLogin::getInstance()->processLogin(id, (ConsoleLoginMessage*)msg);
			break;

		case CMD_CONSOLE_SUBSCRIBE:
			msg = new ConsoleSubscribeMessage;
			msg->unpack(*readBuffer);
			if (dynamic_cast<ConsoleSubscribeMessage*>(msg)->subscribed)
			{
				ConsoleProfiler::getInstance()->subscribeProfiler(id);
			}
			else
			{
				ConsoleProfiler::getInstance()->unsubscribeProfiler(id);
			}
			break;
		}	// end switch
		delete msg;
		delete pHead;
		pHead = NULL;
	}
}
