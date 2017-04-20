#ifndef __SDK_REQUEST_H__
#define __SDK_REQUEST_H__

#include "HttpClient.h"
#include "network/Message.h"

using namespace ws;

class SDKRequest : public HttpRequest
{
public:
	SDKRequest();
	~SDKRequest();

	typedef std::function<void(std::string&)> HttpCallback;

	virtual void onRequest(HttpClient& client);
	virtual void onFinish();

private:
	enum RequestType
	{
		
	};
	RequestType type;
	std::map<std::string, std::string> params;
	HttpCallback callback;
};

#endif