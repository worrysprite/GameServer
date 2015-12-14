#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <string>
#include <memory.h>
#include "utils/ByteArray.h"

enum RequestCommand
{
	CMD_C2S_VERSION = 10000,
	CMD_C2S_ACTIVE_CODE,
	CMD_C2S_UI_CONFIG,
	CMD_C2S_SDK_CONFIG,

	CMD_C2S_MAX
};

enum ReplyCommand
{
	CMD_S2C_VERSION = 20000,
	CMD_S2C_ACTIVE_CODE,
	CMD_S2C_UI_CONFIG,
	CMD_S2C_SDK_CONFIG,

	CMD_S2C_MAX
};

enum ConsoleCommand
{
	CMD_CONSOLE_LOGIN = 1000,
	CMD_CONSOLE_SUBSCRIBE,

	CMD_CONSOLE_MAX
};

using ws::utils::ByteArray;

#pragma pack(push, 1)
class Message
{
public:
	Message(){};
	virtual ~Message(){};
	virtual void unpack(ByteArray& input) = 0;
	virtual void pack(ByteArray& output) = 0;

protected:
	void readUTF8(ByteArray& input, std::string& output);
	void writeUTF8(ByteArray& output, std::string& input);
};

#define writeBlock(output,fromField,toField) {\
	void* ptr(&(this->fromField));\
	size_t size((intptr_t)(&(this->toField))-(intptr_t)ptr+sizeof(toField));\
	output.writeObject(ptr, size);\
}

#define readBlock(input,fromField,toField) {\
	void* ptr(&(this->fromField));\
	size_t size((intptr_t)(&(this->toField))-(intptr_t)ptr+sizeof(toField));\
	input.readObject(ptr, size);\
}

#define zeroInit(fromField,toField) {\
	void* ptr(&(this->fromField));\
	size_t size((intptr_t)(&(this->toField))-(intptr_t)ptr+sizeof(toField));\
	memset(ptr, 0, size);\
}

#define HEAD_SIZE 4
#define MESSAGE_MAX_SIZE 4096

//消息头（空消息）
typedef struct MessageHead : public Message
{
	MessageHead(void) :command(0), packSize(0){}
	unsigned short command;
	unsigned short packSize;

	virtual void unpack(ByteArray& input);
	virtual void pack(ByteArray& output);
} EmptyMessage;

//激活码消息
struct ActivationMessage : public Message
{
	ActivationMessage()
	{
		zeroInit(status, plane4)
	}
	std::string		code;
	unsigned char	status;
	unsigned int	reward;
	unsigned int	coin;
	unsigned int	bomb;
	unsigned int	shield;
	unsigned char	plane2;
	unsigned char	plane3;
	unsigned char	plane4;

	virtual void unpack(ByteArray& input);
	virtual void pack(ByteArray& output);
};

struct UIConfig
{
	UIConfig() : ui(0), open(0)
	{

	}
	unsigned short	ui;
	unsigned char	open;
};

struct UIConfigMessage : public Message
{
	UIConfigMessage()
	{
		zeroInit(version, configList)
	}
	unsigned char	version;
	unsigned char	platform;
	unsigned short	numConfig;
	UIConfig*		configList;

	virtual void unpack(ByteArray& input);
	virtual void pack(ByteArray& output);
};

struct SDKConfigMessage : public Message
{
	SDKConfigMessage()
	{
		zeroInit(version, opensdk)
	}
	unsigned char	version;
	unsigned char	platform;
	unsigned int	opensdk;

	virtual void unpack(ByteArray& input);
	virtual void pack(ByteArray& output);
};

struct ConsoleLoginMessage : public Message
{
	ConsoleLoginMessage() : id(0)
	{
		
	}

	unsigned int	id;
	std::string		username;
	std::string		password;

	virtual void unpack(ByteArray& input);
	virtual void pack(ByteArray& output);
};

struct ConsoleSubscribeMessage : public Message
{
	ConsoleSubscribeMessage()
	{
		zeroInit(subscribed, memoryPeak);
	}

	unsigned char	subscribed;
	unsigned short	numOnline;
	unsigned int	numDBRequests;
	unsigned int	ioDataPoolSize;
	unsigned int	ioDataPostedSize;
	unsigned int	memoryUsed;
	unsigned int	memoryPeak;

	virtual void unpack(ByteArray& input);
	virtual void pack(ByteArray& output);
};

#pragma pack(pop)
#endif //__MESSAGE_H__
