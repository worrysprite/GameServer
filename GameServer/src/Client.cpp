#include "Client.h"
#include "gameLogic/ActivationCode.h"
#include "gameLogic/UIControl.h"
#include "gameLogic/SDKControl.h"

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
		if (pHead->packSize > MESSAGE_MAX_SIZE)
		{
			manager->closeClient(this);
			return;
		}
		if (readBuffer->available() < pHead->packSize - headSize)	//不够消息体大小
		{
			return;
		}
		Message* msg = NULL;
		switch (pHead->command)
		{
		case CMD_C2S_VERSION:
			break;

		case CMD_C2S_ACTIVE_CODE:
			msg = new ActivationMessage;
			msg->unpack(*readBuffer);
			ActivationCode::getInstance()->processActivationCode(this, (ActivationMessage*)msg);
			break;

		case CMD_C2S_UI_CONFIG:
			msg = new UIConfigMessage;
			msg->unpack(*readBuffer);
			UIControl::getInstance()->processUIConfig(this, (UIConfigMessage*)msg);
			break;

		case CMD_C2S_SDK_CONFIG:
			msg = new SDKConfigMessage;
			msg->unpack(*readBuffer);
			SDKControl::getInstance()->processSDKConfig(this, (SDKConfigMessage*)msg);
			break;
		}	// end switch
		delete msg;
		delete pHead;
		pHead = NULL;
	}
}
