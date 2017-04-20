#ifndef __WEBSOCKET_CLIENT_H__
#define __WEBSOCKET_CLIENT_H__

#include <list>
#include "ServerSocket.h"
#include "Event.h"
#include "utils/Timer.h"
#include "network/Message.h"

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
		UNPACK_START,
		PAYLOAD_LENGTH_STEP,
		MASK_STEP,
		PAYLOAD_DATA_STEP,
		UNPACK_COMPLETE
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

class WebsocketClient : public Client, public EventDispatcher
{
public:
	WebsocketClient();
	virtual ~WebsocketClient();
	
	//覆盖基类接口
	virtual void onRecv();
	virtual void onDisconnected();
	virtual void update();
	//通讯接口相关
	virtual void sendCloseFrame();
	virtual void sendBinaryFrame(WebsocketFrame& frame);
	virtual void sendCommonResult(uint16_t code, uint32_t reserve1 = 0, uint32_t reserve2 = 0, uint16_t cmd = 0);
	virtual void sendMessage(ClientCommand cmd, Message& msg);
	virtual void sendMessage(ClientCommand cmd, uint32_t bodySize, void* body);
	virtual void sendEmptyMessage(ClientCommand cmd);

	UserAccount*							account;
	PlayerData*								playerData;
	uint32_t								dataChangedFlag;

private:
	//协议相关
	bool									isWebsocketConnected;
	WebsocketFrame*							lastFrame;
	std::list<WebsocketFrame*>				fragmentFrames;
	MessageHead*							pHead;
	ByteArray								messageData;

	bool parseWebsocketHandshake();
	void processFrames();
	void handleMessage();
};

#endif