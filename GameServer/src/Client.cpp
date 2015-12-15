#include "Client.h"
#include "utils/Log.h"
#include "gameLogic/ActivationCode.h"
#include "gameLogic/UIControl.h"
#include "gameLogic/SDKControl.h"

extern ServerSocket* gServer;

Client::Client() : pHead(nullptr)
{

}

Client::~Client()
{
	if (pHead)
	{
		delete pHead;
		pHead = nullptr;
	}
}

void Client::onRecv()
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
			pHead->command < CMD_C2S_VERSION  ||
			pHead->command >= CMD_C2S_MAX)
		{
			isClosing = true;
			return;
		}
		if (readBuffer->available() < pHead->packSize - headSize)	//不够消息体大小
		{
			return;
		}
		Message* msg = NULL;
		Log::v("recv command: %d", pHead->command);
		switch (pHead->command)
		{
		case CMD_C2S_VERSION:
			break;

		case CMD_C2S_ACTIVE_CODE:
			msg = new ActivationMessage;
			msg->unpack(*readBuffer);
			ActivationCode::getInstance()->processActivationCode(id, (ActivationMessage*)msg);
			break;

		case CMD_C2S_UI_CONFIG:
			msg = new UIConfigMessage;
			msg->unpack(*readBuffer);
			UIControl::getInstance()->processUIConfig(id, (UIConfigMessage*)msg);
			break;

		case CMD_C2S_SDK_CONFIG:
			msg = new SDKConfigMessage;
			msg->unpack(*readBuffer);
			SDKControl::getInstance()->processSDKConfig(id, (SDKConfigMessage*)msg);
			break;
		}	// end switch
		readBuffer->cutHead(pHead->packSize);
		delete msg;
		delete pHead;
		pHead = NULL;
	}
}

void Client::parseWebsocketHandshake()
{
	char buffer[1024] = {0};
	readBuffer->readObject(buffer, 1024);
	
}
