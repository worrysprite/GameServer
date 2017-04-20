#ifndef __GAME_DATA_H__
#define __GAME_DATA_H__

#include <map>
#include <vector>
#include <list>
#include <memory.h>
#include "utils/Timer.h"

using namespace ws::utils;

#define getFieldSize(fromField,toField) {\
	(intptr_t)&this->toField-(intptr_t)&this->fromField+sizeof(toField)\
}

#define zeroInit(fromField,toField) {\
	void* ptr(&(this->fromField));\
	size_t size((intptr_t)(&(this->toField))-(intptr_t)ptr+sizeof(toField));\
	memset(ptr, 0, size);\
}

enum ErrorCodeEnum
{
	SUCCESS,							//成功
	MUST_LOGIN_FIRST,					//必须先登录
	ACCOUNT_NOT_EXIST,					//账号不存在
	PLAYER_NOT_EXIST,					//玩家不存在
	LOGIN_REPEAT,						//重复登陆
	INVALID_CHARACTERS,					//无效的字符
	CREATE_ROLE_REPEAT,					//重复创建角色
	CREATE_ROLE_MAX,					//已达最大创建数
	INVALID_ARGUMENTS,					//无效的参数
};

enum SexEnum
{
	SEX_MALE,
	SEX_FEMALE
};

// 选择角色信息
struct ChooseRoleInfo
{
	ChooseRoleInfo(){ zeroInit(id, level) }
	uint64_t						id;
	uint8_t							sex;
	uint16_t						level;
	std::string						name;
};

//玩家数据
struct PlayerData
{
	PlayerData(){ zeroInit(clientID, level) }
	int64_t									clientID;		//客户端id
	uint64_t								offlineTime;	//离线时间
	bool									isChanged;

	uint64_t								id;						//id
	uint8_t									sex;					//性别
	uint16_t								level;					//等级
	std::string								account;				//平台UID
	std::string								roleName;				//角色名
};

//用户账号
struct UserAccount
{
	UserAccount() :clientID(0), offlineTime(0){ }
	int64_t							clientID;
	uint64_t						offlineTime;	//离线时间
	std::string						account;
	std::vector<ChooseRoleInfo>		roleList;
};

#endif
