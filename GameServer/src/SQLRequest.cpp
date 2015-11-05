#include "SQLRequest.h"

SQLRequest::SQLRequest() : data(NULL), callback(nullptr)
{

}

SQLRequest::~SQLRequest()
{

}

void SQLRequest::onFinish()
{
	if (callback)
	{
		callback(data);
	}
	delete data;
	data = nullptr;
}