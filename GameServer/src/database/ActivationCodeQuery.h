#ifndef __ACTIVATION_CODE_QUERY_H__
#define __ACTIVATION_CODE_QUERY_H__

#include "Database.h"
#include "Message.h"

using ws::Database;

enum ActivationStatus
{
	SUCCESS,
	USED_CODE,
	INVALID_CODE
};

class ActivationCodeQuery : public ws::DBRequest
{
public:
	typedef std::function<void(ActivationMessage*)> CallbackType;

	void queryActivationCode(const std::string& code, CallbackType callback);
	void updateActivationStatus(const ActivationMessage& code, unsigned char status, CallbackType callback);
	virtual void onRequest(Database& db);
	virtual void onFinish();

private:
	std::string code;
	unsigned char status;
	ActivationMessage* activation;

	CallbackType queryCallback;
	CallbackType updateCallback;

	enum Type
	{
		QUERY_ACTIVATION_CODE,
		UPDATE_ACTIVATION_STATUS
	};
	Type type;
};

#endif