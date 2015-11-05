#ifndef __SQL_REQUEST_H__
#define __SQL_REQUEST_H__

#include "Database.h"

using namespace ws;

class SQLRequest : public DBRequest
{
public:
	SQLRequest();
	virtual ~SQLRequest();

	std::function<void(void* data)> callback;
	virtual void onFinish();

protected:
	void* data;
};

#endif