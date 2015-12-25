#ifndef __WEBSOCKET_CLIENT_H__
#define __WEBSOCKET_CLIENT_H__

#include "ServerSocket.h"
#include "Message.h"
#include <list>

using namespace ws;

class WebsocketFrame
{
public:
	enum FrameType
	{
		FRAGMENT_FRAME,
		TEXT_FRAME,
		BINARY_FRAME,
		CLOSE_FRAME,
		PING_FRAME,
		PONG_FRAME,
		ERROR_FRAME
	};

	enum FrameStep
	{
		NEW_FRAME,
		PAYLOAD_LENGTH_STEP,
		MASK_STEP,
		PAYLOAD_DATA_STEP,
		FULL_FRAME
	};

	WebsocketFrame();
	~WebsocketFrame();

	void unpack(ByteArray& input);
	void pack(ByteArray& output);

	FrameStep			unpackStep;
	bool				isFinalFragment;
	FrameType			type;
	bool				isMasked;
	unsigned int		maskKey;
	unsigned long long	payloadLength;
	ByteArray			payloadData;
};

class WebsocketClient : public ClientSocket
{
public:

	WebsocketClient();
	~WebsocketClient();
	void onRecv();

private:
	WebsocketFrame* lastFrame;
	std::list<WebsocketFrame*> fragmentFrames;
	MessageHead* pHead;
	bool isWebsocketConnected;

	bool parseWebsocketHandshake();
	void processFrames();
	void handleMessage(ByteArray& data);
	void sendCloseFrame();

};

#endif