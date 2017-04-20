#include "HttpClient.h"
#include <functional>
#include "utils/Log.h"

namespace ws
{
	using namespace utils;

	HttpClient::HttpClient() :handle(nullptr), response(nullptr)
	{
		handle = curl_easy_init();
		if (!handle)
		{
			Log::e("curl_easy_init failed!");
		}
	}

	HttpClient::~HttpClient()
	{
		curl_easy_cleanup(handle);
	}

	void HttpClient::curlRequest(std::string url, const std::map<std::string, std::string>* params /*= nullptr*/, RequestMethod method /*= GET*/,bool isNeedEscape)
	{
		curl_easy_setopt(handle, CURLOPT_TIMEOUT, 15);
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &HttpClient::writeCallback);
		std::string queryStr;
		if (params)
		{
			queryStr = buildQuery(*params,isNeedEscape);
		}
		switch (method)
		{
		case ws::GET:
			if (queryStr.size())
			{
				url += "?";				
				url += queryStr;
			}
			break;
		case ws::POST:
			curl_easy_setopt(handle, CURLOPT_POST, 1);
			if (queryStr.size())
			{
				curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, queryStr.size());
				curl_easy_setopt(handle, CURLOPT_POSTFIELDS, queryStr.c_str());
			}
			break;
		}
		curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
		Log::d("curl requesting: %s", url.c_str());
		CURLcode result = curl_easy_perform(handle);
		if (result != CURLE_OK)
		{
			Log::w("curl request failed: %s", curl_easy_strerror(result));
		}
	}

	std::string HttpClient::buildQuery(const std::map<std::string, std::string>& params,bool isNeedEscape)
	{
		std::string outstr;
		int count(0);
		for (auto &p : params)
		{
			if (count > 0)
			{
				outstr += "&";
			}
			outstr += p.first;
			outstr += "=";
			if (isNeedEscape)
			{
				char* temp = curl_easy_escape(handle, p.second.c_str(), (int)p.second.length());
				outstr += temp;
				curl_free(temp);
			}
			else
			{
				outstr += p.second;
			}
			++count;
		}
		return outstr;
	}

	std::string HttpClient::buildQuery(const std::map<std::string, std::string>& params, std::string Ssplit/*="&"*/, bool isNeedEscape /*= false*/)
	{
		{
			std::string outstr;
			int count(0);
			for (auto &p : params)
			{
				if (count > 0)
				{
					outstr += Ssplit;
				}
			//	outstr += p.first;
			//	outstr += "=";
				if (isNeedEscape)
				{
					char* temp = curl_easy_escape(handle, p.second.c_str(), (int)p.second.length());
					outstr += temp;
					curl_free(temp);
				}
				else
				{
					outstr += p.second;
				}
				++count;
			}
			return outstr;
		}
	}

	size_t HttpClient::writeCallback(char* data, size_t size, size_t nmemb, void* userdata)
	{
		HttpClient* client = (HttpClient*)userdata;
		size_t numBytes = size * nmemb;
		if (client->response)
		{
			client->response->append(data, numBytes);
		}
		return numBytes;
	}


	HttpQueue::HttpQueue()
	{

	}

	HttpQueue::~HttpQueue()
	{
		if (workQueueLength > 0)
		{
			Log::w("HttpQueue remains %d requests aborted.", workQueueLength);
		}
		isExit = true;
		for (auto th : workerThreads)
		{
			th->join();
			delete th;
			Log::d("HttpQueue worker thread joined.");
		}
		curl_global_cleanup();
	}

	void HttpQueue::init(int numThread)
	{
		curl_global_init(CURL_GLOBAL_ALL);
		for (int i = 0; i < numThread; ++i)
		{
			std::thread* th = new std::thread(std::bind(&HttpQueue::HttpWorkThread, this));
			workerThreads.push_back(th);
		}
	}

	void HttpQueue::addQueueMsg(PtrHttpRequest request)
	{
		workMtx.lock();
		workQueue.push_back(request);
		workQueueLength = workQueue.size();
		workMtx.unlock();
	}

	void HttpQueue::update()
	{
		HttpRequestList tmpQueue;
		finishMtx.lock();
		tmpQueue.swap(finishQueue);
		finishMtx.unlock();

		while (!tmpQueue.empty())
		{
			PtrHttpRequest request = tmpQueue.front();
			tmpQueue.pop_front();
			Log::d("curl response: %s", request->response.c_str());
			request->response;
			request->onFinish();
		}
	}

	PtrHttpRequest HttpQueue::getRequest()
	{
		PtrHttpRequest request(nullptr);
		workMtx.lock();
		if (!workQueue.empty())
		{
			request = workQueue.front();
			workQueue.pop_front();
			workQueueLength = workQueue.size();
		}
		workMtx.unlock();
		return request;
	}

	void HttpQueue::finishRequest(PtrHttpRequest request)
	{
		finishMtx.lock();
		finishQueue.push_back(request);
		finishMtx.unlock();
	}

	void HttpQueue::HttpWorkThread()
	{
		HttpClient client;
		PtrHttpRequest request(nullptr);
		const std::chrono::milliseconds requestWait(50);
		while (!isExit)
		{
			if (request = getRequest())
			{
				client.response = &request->response;
				request->onRequest(client);
				finishRequest(request);
				request.reset();
				client.reset();
			}
			else
			{
				std::this_thread::sleep_for(requestWait);
			}
		}
	}
}
