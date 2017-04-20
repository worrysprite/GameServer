#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "utils/ByteArray.h"
#include "DataDefinition.h"

enum ClientCommand
{
	CMD_LOGIN = 10001,					//登陆
	CMD_RESULT,							//通用结果
	CMD_TIME_TICK,						//心跳包
	CMD_CREATE_ROLE,					//创建角色
	CMD_ENTER_GAME,						//进入游戏
	CMD_PLAYER_DATA,					//玩家数据
	CMD_MAX
};

enum DataChangeFlag
{
	DATA_CHANGE_FLAG_PLAYER_BASE = 0x01
};

using ws::utils::ByteArray;

#pragma pack(push, 1)

class Message
{
public:
	Message(){}
	virtual ~Message(){}
	virtual void unpack(ByteArray& input){}
	virtual void pack(ByteArray& output){}

	static void readUTF8(ByteArray& input, std::string& output)
	{
		unsigned short length(input.readUnsignedShort());
		if (length > 0)
		{
			char* str = new char[length];
			input.readObject(str, length);
			output = std::string(str, length);
			delete[] str;
		}
		else
		{
			output = "";
		}
	}
	static void writeUTF8(ByteArray& output, const std::string& input)
	{
		unsigned short length((unsigned short)input.size());
		output.writeUnsignedShort(length);
		if (length > 0)
		{
			output.writeObject(input.c_str(), length);
		}
	}
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

#define HEAD_SIZE 6
#define REQUEST_MAX_SIZE 0x1000
#define CONSOLE_REQUEST_MAX_SIZE 0xFFFF
#define CROSS_PACK_MAX_SIZE 0xFFFF

//消息头（空消息）
typedef struct MessageHead : public Message
{
	MessageHead(void) :command(0), packSize(0){}
	uint16_t command;
	uint32_t packSize;

	inline virtual void unpack(ByteArray& input)
	{
		input >> command >> packSize;
	}
	inline virtual void pack(ByteArray& output)
	{
		output << command << packSize;
	}
} EmptyMessage;

struct UINT8Message : public Message
{
	UINT8Message() :value(0){}
	uint8_t							value;
	inline virtual void unpack(ByteArray& input)
	{
		input >> value;
	}
	inline virtual void pack(ByteArray& output)
	{
		output << value;
	}
};

struct UINT16Message : public Message
{
	UINT16Message() :value(0){}
	uint16_t						value;
	inline virtual void unpack(ByteArray& input)
	{
		input >> value;
	}
	inline virtual void pack(ByteArray& output)
	{
		output << value;
	}
};

struct UINT32Message : public Message
{
	UINT32Message() :value(0){}
	uint32_t						value;
	inline virtual void unpack(ByteArray& input)
	{
		input >> value;
	}
	inline virtual void pack(ByteArray& output)
	{
		output << value;
	}
};

typedef struct UINT64Message : public Message
{
	UINT64Message() :value(0){}
	uint64_t						value;
	inline virtual void unpack(ByteArray& input)
	{
		input >> value;
	}
	inline virtual void pack(ByteArray& output)
	{
		output << value;
	}
} EnterGameMessage;

typedef struct DOUBLEMessage : public Message
{
	DOUBLEMessage() :value(0.0){}
	double							value;
	inline virtual void unpack(ByteArray& input)
	{
		input >> value;
	}
	inline virtual void pack(ByteArray& output)
	{
		output << value;
	}
} TimeTickMessage;

struct StringMessage : public Message
{
	std::string						value;
	inline virtual void unpack(ByteArray& input)
	{
		readUTF8(input, value);
	}
	inline virtual void pack(ByteArray& output)
	{
		writeUTF8(output, value);
	}
};

//登陆消息
struct LoginMessage : public Message
{
	LoginMessage() :roleList(nullptr){ }
	std::string						account;
	std::vector<ChooseRoleInfo>*	roleList;
	virtual void unpack(ByteArray& input)
	{
		readUTF8(input, account);
	}
	virtual void pack(ByteArray& output)
	{
		output << (uint8_t)roleList->size();
		for (ChooseRoleInfo& cri : *roleList)
		{
			output << cri.id << cri.sex << cri.level;
			writeUTF8(output, cri.name);
		}
	}
};

//创建角色消息
struct CreateRoleMessage : public Message
{
	CreateRoleMessage() :sex(0){}
	uint8_t							sex;
	std::string						name;
	virtual void unpack(ByteArray& input)
	{
		input >> sex;
		readUTF8(input, name);
	}
};

//通用结果消息
struct ResultMessage : public Message
{
	ResultMessage(){ zeroInit(code, reserve2) }
	uint16_t						code;
	uint16_t						cmd;
	uint32_t						reserve1;
	uint32_t						reserve2;
	virtual void pack(ByteArray& output)
	{
		writeBlock(output, code, reserve2)
	}
	virtual void unpack(ByteArray& input)
	{
		readBlock(input, code, reserve2)
	}
};

// 玩家消息
struct PlayerMessage : public Message
{
	PlayerMessage() :player(nullptr){}
	const PlayerData*				player;
	virtual void pack(ByteArray& output)
	{
		output << player->id << player->sex << player->level;
		writeUTF8(output, player->roleName);
	}
};

#pragma pack(pop)
#endif //__MESSAGE_H__
