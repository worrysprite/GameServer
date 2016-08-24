#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <openssl/evp.h>
#include "WebsocketClient.h"
#include "utils/Log.h"
#include "utils/String.h"
#include "gameLogic/ActivationCode.h"
#include "gameLogic/UIControl.h"
#include "gameLogic/SDKControl.h"

const char* WEBSOCKET_REPLY = "HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade: websocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: %s\r\n\r\n";

WebsocketClient::WebsocketClient() : pHead(nullptr), isWebsocketConnected(false)
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
}

void WebsocketClient::onRecv()
{
	if (!isWebsocketConnected && !parseWebsocketHandshake())
	{
		return;
	}
	while (true)
	{
		// unpack frame
		if (!lastFrame)
		{
			lastFrame = new WebsocketFrame;
		}
		readBuffer->lock();
		lastFrame->unpack(*readBuffer);
		readBuffer->cutHead(readBuffer->position);
		readBuffer->unlock();
		if (lastFrame->unpackStep != WebsocketFrame::FULL_FRAME)
		{
			break;
		}
		if (lastFrame->type == WebsocketFrame::ERROR_FRAME ||
			!lastFrame->isMasked)	// client data must be masked!
		{
			isClosing = true;
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
			delete lastFrame;
		}
		else
		{
			fragmentFrames.push_back(lastFrame);
		}
		lastFrame = nullptr;
	}
}

bool WebsocketClient::parseWebsocketHandshake()
{
	readBuffer->lock();
	char buffer[1024] = {0};
	readBuffer->readObject(buffer, 1024);
	Log::d(buffer);

	std::vector<char*> httpRequest;
	utils::String::split(buffer, "\r\n\r\n", httpRequest);
	if (httpRequest.size() < 2)
	{
		readBuffer->position = 0;
		readBuffer->unlock();
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
			auto key = std::string(headerTexts[0]);
			if (key == "Upgrade" || key == "Connection")
			{
				utils::String::toLowercase(headerTexts[1]);
			}
			auto value = std::string(headerTexts[1]);
			httpHeaders.insert(std::make_pair(key, value));
		}
		else
		{
			if (!strstr(line, "HTTP/1.1"))
			{
				isClosing = true;
				return false;
			}
		}
	}
	// RFC6455 requirement
	if (httpHeaders["Upgrade"] != "websocket" ||
		httpHeaders["Connection"] != "upgrade" ||
		httpHeaders["Sec-WebSocket-Version"] != "13")
	{
		isClosing = true;
		return false;
	}
	std::string serverKey = httpHeaders["Sec-WebSocket-Key"];
	if (serverKey.length() == 0)
	{
		isClosing = true;
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

	char reply[1024] = {0};
	sprintf(reply, WEBSOCKET_REPLY, b64result);
	send(reply, strlen(reply));
	flush();
	isWebsocketConnected = true;
	return true;
}

void WebsocketClient::processFrames()
{
	ByteArray fullData;
	for (auto frame : fragmentFrames)
	{
		fullData.writeBytes(frame->payloadData, 0, frame->payloadLength);
	}
	fullData.writeBytes(lastFrame->payloadData, 0, lastFrame->payloadLength);

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
		isClosing = true;
		return;
	}
	case WebsocketFrame::PING_FRAME:	// read ping and reply a pong
	{
		WebsocketFrame pongFrame;
		pongFrame.type = WebsocketFrame::PONG_FRAME;
		pongFrame.payloadLength = fullData.getSize();
		pongFrame.payloadData.writeBytes(fullData);
		writeBuffer->lock();
		pongFrame.pack(*writeBuffer);
		writeBuffer->unlock();
		flush();
		break;
	}
	case WebsocketFrame::PONG_FRAME:	// ignore pong frame
	{
		break;
	}
	default:
	{
		fullData.position = 0;
		handleMessage(fullData);
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
	flush();
}

void WebsocketClient::handleMessage(ByteArray& data)
{
	size_t len = data.getSize() + 1;
	char* str = new char[len];
	memset(str, 0, len);
	data.readObject(str);
	Log::d(str);
	WebsocketFrame frame;
	frame.type = WebsocketFrame::TEXT_FRAME;
	frame.payloadData.writeObject(str, len);
	writeBuffer->lock();
	frame.pack(*writeBuffer);
	writeBuffer->unlock();
	flush();
	delete[] str;
}

WebsocketFrame::WebsocketFrame() :isFinalFragment(true), unpackStep(NEW_FRAME), type(FRAGMENT_FRAME), isMasked(false), maskKey(0), payloadLength(0)
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
	if (unpackStep == NEW_FRAME)	// read first 2 bytes
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
			unpackStep = FULL_FRAME;
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
			payloadLength = input.readUnsignedShort();
		}
		else if (payloadLength == 127)
		{
			if (input.available() < 8)
			{
				return;
			}
			payloadLength = input.readUnsignedInt64();
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
		unpackStep = FULL_FRAME;
	}
}

void WebsocketFrame::pack(ByteArray& output)
{
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
		output.writeUnsignedShort(payloadLength);
	}
	else
	{
		bytedata |= 127;
		output << bytedata;
		output.writeUnsignedInt64(payloadLength);
	}
	if (isMasked)
	{
		output << maskKey;
	}
	output.writeBytes(payloadData);
}
