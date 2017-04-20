#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <openssl/evp.h>
#include "WebsocketClient.h"
#include "utils/Log.h"
#include "utils/String.h"
#include "network/GameServer.h"
#include "GlobalData.h"
#include "gameLogic/UserLogin.h"
#include "gameLogic/PlayerLogic.h"

const char* WEBSOCKET_REPLY = "HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade: websocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: %s\r\n\r\n";

#define HAND_SHAKE_HEADER_MAX_SIZE 1024

#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#define WINSOCK_SWAP_LONGLONG(l)            \
	( ( ((l) >> 56) & 0x00000000000000FFLL ) |       \
	( ((l) >> 40) & 0x000000000000FF00LL ) |       \
	( ((l) >> 24) & 0x0000000000FF0000LL ) |       \
	( ((l) >>  8) & 0x00000000FF000000LL ) |       \
	( ((l) <<  8) & 0x000000FF00000000LL ) |       \
	( ((l) << 24) & 0x0000FF0000000000LL ) |       \
	( ((l) << 40) & 0x00FF000000000000LL ) |       \
	( ((l) << 56) & 0xFF00000000000000LL ) )

inline unsigned long long htonll(unsigned long long Value)
{
	const unsigned long long Retval = WINSOCK_SWAP_LONGLONG(Value);
	return Retval;
}
#endif

WebsocketClient::WebsocketClient() : pHead(nullptr), isWebsocketConnected(false), playerData(nullptr),
lastFrame(nullptr), dataChangedFlag(0), account(nullptr)
{
	
}

WebsocketClient::~WebsocketClient()
{
	if (pHead)
	{
		delete pHead;
		pHead = nullptr;
	}
	if (lastFrame)
	{
		delete lastFrame;
		lastFrame = nullptr;
	}
	for (auto frame : fragmentFrames)
	{
		delete frame;
	}
	fragmentFrames.clear();
}

void WebsocketClient::onRecv()
{
	if (readBuffer->getSize() >= 10 * 1024 * 1024)
	{
		Log::w("client read buffer size is over 10M!");
		kick();
		return;
	}
	if (!isWebsocketConnected && !parseWebsocketHandshake())
	{
		return;
	}
	while (true)
	{
		if (!lastFrame)
		{
			lastFrame = new WebsocketFrame;
		}
		readBuffer->lock();
		lastFrame->unpack(*readBuffer);
		readBuffer->cutHead(readBuffer->position);
		readBuffer->unlock();
		if (lastFrame->unpackStep != WebsocketFrame::UNPACK_COMPLETE)
		{
			break;
		}
		if (lastFrame->type == WebsocketFrame::ERROR_FRAME ||
			!lastFrame->isMasked)	// client data must be masked!
		{
			Log::d("websocket client data not masked, kick out...");
			kick();
			return;
		}
		// unmask payload data
		unsigned char* mask = (unsigned char*)&lastFrame->maskKey;
		unsigned char* encoded = (unsigned char*)lastFrame->payloadData.getBytes();
		for (size_t i = 0; i < lastFrame->payloadLength; ++i)
		{
			encoded[i] = encoded[i] ^ mask[i % 4];
		}
		// process frames
		if (lastFrame->isFinalFragment)
		{
			processFrames();
			for (auto frame : fragmentFrames)
			{
				delete frame;
			}
			fragmentFrames.clear();
			delete lastFrame;
		}
		else
		{
			fragmentFrames.push_back(lastFrame);
		}
		lastFrame = nullptr;
	}
}

void WebsocketClient::onDisconnected()
{
	uint64_t now = TimeTool::getUnixtime<system_clock>();
	if (playerData)
	{
		if (playerData->clientID == id)
		{
			playerData->offlineTime = now;
		}
		GameDataCache::savePlayerData(playerData);
	}
	if (account)
	{
		account->offlineTime = now;
	}
}

void WebsocketClient::update()
{
	PlayerLogic::notifyPlayerData(this);
}

bool WebsocketClient::parseWebsocketHandshake()
{
	if (readBuffer->getSize() >= HAND_SHAKE_HEADER_MAX_SIZE)
	{
		Log::d("websocket hand shake header too long, size=%d", readBuffer->getSize());
		kick();
		return false;
	}
	char buffer[HAND_SHAKE_HEADER_MAX_SIZE] = {0};
	readBuffer->lock();
	readBuffer->readObject(buffer, HAND_SHAKE_HEADER_MAX_SIZE - 1);

	std::vector<char*> httpRequest;
	utils::String::split(buffer, "\r\n\r\n", httpRequest);
	if (httpRequest.size() < 2)
	{
		readBuffer->position = 0;
		readBuffer->unlock();
		kick();
		return false;
	}
	readBuffer->cutHead(readBuffer->position);
	readBuffer->unlock();
	// httpRequest[0] is http headers, httpRequest[1] is http body
	std::vector<char*> allLines;
	utils::String::split(httpRequest[0], "\r\n", allLines);

	std::map<std::string, std::string> httpHeaders;
	for (auto line : allLines)
	{
		std::vector<char*> headerTexts;
		utils::String::split(line, ": ", headerTexts);
		if (headerTexts.size() == 2)
		{
			std::string key(headerTexts[0]);
			if (key == "Upgrade" || key == "Connection")
			{
				utils::String::toLowercase(headerTexts[1]);
			}
			std::string value(headerTexts[1]);
			httpHeaders.insert(std::make_pair(key, value));
		}
		else
		{
			if (!strstr(line, "HTTP/1.1"))
			{
				Log::d("websocket handshake error! The first line is not HTTP/1.1, %s", line);
				kick();
				return false;
			}
		}
	}
	// RFC6455 requirement
	if (httpHeaders["Upgrade"].find("websocket") == std::string::npos ||
		httpHeaders["Connection"].find("upgrade") == std::string::npos ||
		httpHeaders["Sec-WebSocket-Version"] != "13")
	{
		Log::d("websocket handshake error! Header does not contains \"Upgrade: websocket, Connection: upgrade, Sec-WebSocket-Version: 13\"");
		kick();
		return false;
	}
	std::string serverKey = httpHeaders["Sec-WebSocket-Key"];
	if (serverKey.length() == 0)
	{
		Log::d("websocket handshake error! Header does not contains \"Sec-WebSocket-Key\"");
		kick();
		return false;
	}
	//RFC6544_MAGIC_KEY
	serverKey += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	// sha1
	unsigned int md_len(0);
	unsigned char md_value[EVP_MAX_MD_SIZE] = {0};
	EVP_MD_CTX* mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);
	EVP_DigestUpdate(mdctx, serverKey.c_str(), serverKey.length());
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	EVP_MD_CTX_destroy(mdctx);

	// base64
	BIO* b64, *bmem;
	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	BIO_push(b64, bmem);
	BIO_write(b64, md_value, md_len);
	BIO_flush(b64);
	char b64result[128] = {0};
	int b64resultLength(0);
	b64resultLength = BIO_read(bmem, b64result, 128);
	b64result[b64resultLength - 1] = '\0';
	BIO_free_all(b64);

	char reply[HAND_SHAKE_HEADER_MAX_SIZE] = {0};
	sprintf(reply, WEBSOCKET_REPLY, b64result);
	send(reply, strlen(reply));
	isWebsocketConnected = true;
	return true;
}

void WebsocketClient::processFrames()
{
	WebsocketFrame* firstFrame = lastFrame;
	if (fragmentFrames.size() > 0)
	{
		firstFrame = fragmentFrames.front();
	}
	switch (firstFrame->type)
	{
	case WebsocketFrame::CLOSE_FRAME:	// stop read and send a close frame before close
	{
		sendCloseFrame();
		kick();
		return;
	}
	case WebsocketFrame::PING_FRAME:	// read ping and reply a pong
	{
		ByteArray fullData;
		for (auto frame : fragmentFrames)
		{
			fullData.writeBytes(frame->payloadData, 0, frame->payloadLength);
		}
		fullData.writeBytes(lastFrame->payloadData, 0, lastFrame->payloadLength);
		// reply a pong frame with same data;
		WebsocketFrame pongFrame;
		pongFrame.type = WebsocketFrame::PONG_FRAME;
		pongFrame.payloadLength = fullData.getSize();
		pongFrame.payloadData.writeBytes(fullData);
		writeBuffer->lock();
		pongFrame.pack(*writeBuffer);
		writeBuffer->unlock();
		break;
	}
	case WebsocketFrame::PONG_FRAME:	// ignore pong frame
	{
		break;
	}
	case WebsocketFrame::BINARY_FRAME:
	{
		messageData.position = messageData.getSize();
		for (auto frame : fragmentFrames)
		{
			messageData.writeBytes(frame->payloadData, 0, frame->payloadLength);
		}
		messageData.writeBytes(lastFrame->payloadData, 0, lastFrame->payloadLength);
		messageData.position = 0;
		handleMessage();
		break;
	}
	}
}

void WebsocketClient::sendCloseFrame()
{
	WebsocketFrame closeFrame;
	closeFrame.type = WebsocketFrame::CLOSE_FRAME;
	writeBuffer->lock();
	closeFrame.pack(*writeBuffer);
	writeBuffer->unlock();
}

void WebsocketClient::handleMessage()
{
	while (messageData.available() > 0)
	{
		if (pHead == NULL)	//先解析一个消息头
		{
			if (messageData.available() >= HEAD_SIZE)
			{
				pHead = new MessageHead;
				pHead->unpack(messageData);
			}
			else
			{
				return;
			}
		}
		if (pHead->packSize > REQUEST_MAX_SIZE ||
			pHead->command < CMD_LOGIN ||
			pHead->command >= CMD_MAX)
		{
			if (account)
			{
				Log::d("receive invalid command: %d, size=%d, account=%s, kick out...", pHead->command,
					pHead->packSize, account->account.c_str());
			}
			else
			{
				Log::d("receive invalid command: %d, size=%d, kick out...", pHead->command, pHead->packSize);
			}
			sendCloseFrame();
			kick();
			return;
		}
		if (messageData.available() < pHead->packSize - HEAD_SIZE)	//不够消息体大小
		{
			return;
		}
		// 计算客户端发包频率
		bool mustLogin(false);
		if (account)
		{
			mustLogin = !playerData && pHead->command != CMD_ENTER_GAME && pHead->command != CMD_CREATE_ROLE;
		}
		else
		{
			mustLogin = pHead->command != CMD_LOGIN;
		}
		if (mustLogin)
		{
			sendCommonResult(ErrorCodeEnum::MUST_LOGIN_FIRST);
			messageData.cutHead(pHead->packSize);
			delete pHead;
			pHead = nullptr;
			continue;
		}
		Message* msg = NULL;
		switch (pHead->command)
		{
		case CMD_LOGIN:
			msg = new LoginMessage;
			msg->unpack(messageData);
			UserLogin::processLogin(this, (LoginMessage*)msg);
			break;
		case CMD_TIME_TICK:
		{
			TimeTickMessage tickMsg;
			tickMsg.value = (double)TimeTool::getUnixtime<system_clock>();
			sendMessage(CMD_TIME_TICK, tickMsg);
			break;
		}
		case CMD_ENTER_GAME:
			msg = new EnterGameMessage;
			msg->unpack(messageData);
			UserLogin::processEnterGame(this, (EnterGameMessage*)msg);
			break;
		case CMD_CREATE_ROLE:
			msg = new CreateRoleMessage;
			msg->unpack(messageData);
			UserLogin::processCreateRole(this, (CreateRoleMessage*)msg);
			break;
		}	// end switch
		messageData.cutHead(pHead->packSize);
		delete msg;
		delete pHead;
		pHead = nullptr;
	}
}

void WebsocketClient::sendBinaryFrame(WebsocketFrame& frame)
{
	frame.type = WebsocketFrame::BINARY_FRAME;
	writeBuffer->lock();
	frame.pack(*writeBuffer);
	writeBuffer->unlock();
}

void WebsocketClient::sendCommonResult(uint16_t code, uint32_t reserve1, uint32_t reserve2, uint16_t cmd)
{
	ResultMessage msg;
	msg.code = code;
	if (cmd)
	{
		msg.cmd = cmd;
	}
	else if (pHead)
	{
		msg.cmd = pHead->command;
	}
	msg.reserve1 = reserve1;
	msg.reserve2 = reserve2;

	WebsocketFrame frame;
	MessageHead head;
	head.command = CMD_RESULT;
	head.pack(frame.payloadData);
	msg.pack(frame.payloadData);

	head.packSize = (uint32_t)frame.payloadData.getSize();
	frame.payloadData.position = 0;
	head.pack(frame.payloadData);
	sendBinaryFrame(frame);
}

void WebsocketClient::sendMessage(ClientCommand cmd, Message& msg)
{
	WebsocketFrame frame;
	MessageHead head;
	head.command = (uint16_t)cmd;
	head.pack(frame.payloadData);
	msg.pack(frame.payloadData);
	// rewrite packet size
	head.packSize = (uint32_t)frame.payloadData.getSize();
	frame.payloadData.position = 0;
	head.pack(frame.payloadData);
	sendBinaryFrame(frame);
}

void WebsocketClient::sendMessage(ClientCommand cmd, uint32_t bodySize, void* body)
{
	WebsocketFrame frame;
	MessageHead head;
	head.command = (uint16_t)cmd;
	head.packSize = HEAD_SIZE + bodySize;
	head.pack(frame.payloadData);
	frame.payloadData.writeObject(body,(size_t)bodySize);
	sendBinaryFrame(frame);
}

void WebsocketClient::sendEmptyMessage(ClientCommand cmd)
{
	WebsocketFrame frame;
	MessageHead head;
	head.command = (uint16_t)cmd;
	head.packSize = HEAD_SIZE;
	head.pack(frame.payloadData);
	sendBinaryFrame(frame);
}

WebsocketFrame::WebsocketFrame() :isFinalFragment(true), unpackStep(UNPACK_START),
			type(FRAGMENT_FRAME), isMasked(false), maskKey(0), payloadLength(0)
{

}

WebsocketFrame::~WebsocketFrame()
{

}

void WebsocketFrame::unpack(ByteArray& input)
{
	/*
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-------+-+-------------+-------------------------------+
	|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	|N|V|V|V|       |S|             |   (if payload len==126/127)   |
	| |1|2|3|       |K|             |                               |
	+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	|     Extended payload length continued, if payload len == 127  |
	+ - - - - - - - - - - - - - - - +-------------------------------+
	|                               |Masking-key, if MASK set to 1  |
	+-------------------------------+-------------------------------+
	| Masking-key (continued)       |          Payload Data         |
	+-------------------------------- - - - - - - - - - - - - - - - +
	:                     Payload Data continued ...                :
	+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	|                     Payload Data continued ...                |
	+---------------------------------------------------------------+
	*/
	if (unpackStep == UNPACK_START)	// read first 2 bytes
	{
		// 不足2字节，需要继续等待
		if (input.available() < 2)
		{
			return;
		}

		// 第一个字节, 最高位用于描述消息是否结束,
		unsigned char bytedata = input.readUnsignedByte();
		isFinalFragment = (bytedata >> 7) & 0x01;

		//最低4位用于描述消息类型
		unsigned char msg_opcode = bytedata & 0x0F;
		switch (msg_opcode)
		{
		case 0x0:
			type = FRAGMENT_FRAME;
			break;
		case 0x1:
			type = TEXT_FRAME;
			break;
		case 0x2:
			type = BINARY_FRAME;
			break;
		case 0x8:
			type = CLOSE_FRAME;
			break;
		case 0x9:
			type = PING_FRAME;
			break;
		case 0xA:
			type = PONG_FRAME;
			break;
		default:
			type = ERROR_FRAME;
			unpackStep = UNPACK_COMPLETE;
			return;
		}
		
		// 消息的第二个字节，最高位用0或1来描述是否有掩码处理
		bytedata = input.readUnsignedByte();
		isMasked = (bytedata >> 7) & 0x01;
		// 低7位用于描述消息长度
		payloadLength = bytedata & (~0x80);
		unpackStep = PAYLOAD_LENGTH_STEP;
	}
	if (unpackStep == PAYLOAD_LENGTH_STEP)	// read remaining head
	{
		// 若7位描述的长度为0-125，则实际长度为该值
		// 若7位描述的长度为126，则接下来的2个字节为实际长度
		// 若7位描述的长度为127，则接下来的8个字节为实际长度
		if (payloadLength == 126)
		{
			if (input.available() < 2)
			{
				return;
			}
			payloadLength = htons(input.readUnsignedShort());
		}
		else if (payloadLength == 127)
		{
			if (input.available() < 8)
			{
				return;
			}
			payloadLength = htonll(input.readUnsignedInt64());
		}
		unpackStep = MASK_STEP;
	}
	if (unpackStep == MASK_STEP)
	{
		// 如果存在掩码的情况下获取4字节掩码值
		if (isMasked)
		{
			if (input.available() < 4)
			{
				return;
			}
			input >> maskKey;
		}
		unpackStep = PAYLOAD_DATA_STEP;
	}
	if (unpackStep == PAYLOAD_DATA_STEP)
	{
		if (input.available() < payloadLength)
		{
			return;
		}
		input.readBytes(payloadData, 0, payloadLength);
		unpackStep = UNPACK_COMPLETE;
	}
}

void WebsocketFrame::pack(ByteArray& output)
{
	output.position = output.getSize();
	unsigned char opcode(0);
	switch (type)
	{
	case WebsocketFrame::BINARY_FRAME:
		opcode = 0x02;
		break;
	case WebsocketFrame::TEXT_FRAME:
		opcode = 0x01;
		break;
	case WebsocketFrame::CLOSE_FRAME:
		opcode = 0x08;
		break;
	case WebsocketFrame::PING_FRAME:
		opcode = 0x09;
		break;
	case WebsocketFrame::PONG_FRAME:
		opcode = 0x0A;
		break;
	}
	// first byte
	unsigned char bytedata = (isFinalFragment << 7) | opcode;
	output << bytedata;
	// second byte
	bytedata = isMasked ? 0x80 : 0;
	payloadLength = payloadData.getSize();
	if (payloadLength <= 125)
	{
		bytedata |= payloadLength & (~0x80);
		output << bytedata;
	}
	else if (payloadLength <= 0xFFFF)
	{
		bytedata |= 126;
		output << bytedata;
		output.writeUnsignedShort(htons((uint16_t)payloadLength));
	}
	else
	{
		bytedata |= 127;
		output << bytedata;
		output.writeUnsignedInt64(htonll(payloadLength));
	}
	if (isMasked)
	{
		output << maskKey;
	}
	output.writeBytes(payloadData);
}
