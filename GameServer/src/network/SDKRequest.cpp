#include "SDKRequest.h"
#include "network/GameServer.h"
#include "utils/String.h"

#include <algorithm>

SDKRequest::SDKRequest()
{

}

SDKRequest::~SDKRequest()
{

}

void SDKRequest::onRequest(HttpClient& client)
{

}

void SDKRequest::onFinish()
{
	if (callback)
	{
		callback(response);
	}
	response.clear();
	params.clear();
	callback = nullptr;
}
