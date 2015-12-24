#ifndef __WEBSOCKET_CLIENT_H__
#define __WEBSOCKET_CLIENT_H__

#include "ServerSocket.h"
#include "Message.h"

using namespace ws;

class WebsocketFrame
{
public:
	enum FrameType
	{
		ERROR_FRAME,
		INCOMPLETE_FRAME,
		BINARY_FRAME,
		TEXT_FRAME,
		CLOSE_FRAME,
		PING_FRAME,
		PONG_FRAME
	};

	WebsocketFrame();
	~WebsocketFrame();

	static WebsocketFrame* unpack(ByteArray& input);
	static WebsocketFrame* pack(ByteArray& output);

	bool				isFinalFragment;
	FrameType			type;
	bool				isMasked;
	unsigned int		maskKey;
	unsigned long long	payloadLength;
};

class WebsocketClient : public ClientSocket
{
public:

	WebsocketClient();
	~WebsocketClient();
	void onRecv();

private:
	MessageHead* pHead;
	bool isWebsocketConnected;

	bool parseWebsocketHandshake();
};

#endif