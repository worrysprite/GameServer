#ifndef __ACTIVATION_CODE_QUERY_H__
#define __ACTIVATION_CODE_QUERY_H__

#include "SQLRequest.h"

enum ActivationStatus
{
	SUCCESS,
	USED_CODE,
	INVALID_CODE
};

class ActivationCodeQuery : public SQLRequest
{
public:
	void queryActivationCode(const std::string& code);
	void updateActivationStatus(const std::string& code, unsigned char status);
	virtual void onRequest(Database& db);

private:
	std::string code;
	unsigned char status;
	enum Type
	{
		QUERY_ACTIVATION_CODE,
		UPDATE_ACTIVATION_STATUS
	};
	Type type;
};

#endif