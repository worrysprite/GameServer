#ifndef __WS_HTTP_QUEUE_H__
#define __WS_HTTP_QUEUE_H__

#include <string>
#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <list>
#include <thread>
#include <curl/curl.h>

namespace ws
{
	enum RequestMethod
	{
		GET,
		POST
	};

	class HttpQueue;

	class HttpClient
	{
		friend class HttpQueue;
	public:
		HttpClient();
		virtual ~HttpClient();

		void curlRequest(std::string url, const std::map<std::string, std::string>* params = nullptr, RequestMethod method = GET, bool isNeedEscape = false);
		inline CURL* getHandle(){ return handle; }
		inline void reset(){ response = nullptr; curl_easy_reset(handle); }

		std::string buildQuery(const std::map<std::string, std::string>& params, bool isNeedEscape = false);
		std::string buildQuery(const std::map<std::string, std::string>& params,std::string Ssplit,bool isNeedEscape = false);

	private:
		CURL* handle;
		std::string* response;
		static size_t writeCallback(char* data, size_t size, size_t nmemb, void* userdata);
	};

	class HttpRequest
	{
		friend class HttpQueue;
	public:
		virtual ~HttpRequest(){}
		virtual void onRequest(HttpClient& client) = 0;
		virtual void onFinish() = 0;
	protected:
		std::string response;
	};
	typedef std::shared_ptr<HttpRequest> PtrHttpRequest;

	class HttpQueue
	{
	public:
		HttpQueue();
		virtual ~HttpQueue();

		void					init(int numThread);
		void					addQueueMsg(PtrHttpRequest request);
		void					update();
		inline size_t			getQueueLength(){ return workQueueLength; }

	private:
		typedef std::list<PtrHttpRequest> HttpRequestList;
		PtrHttpRequest			getRequest();
		void					finishRequest(PtrHttpRequest request);
		void					HttpWorkThread();

		std::mutex				workMtx;
		HttpRequestList			workQueue;
		size_t					workQueueLength;
		std::mutex				finishMtx;
		HttpRequestList			finishQueue;
		bool					isExit;
		std::vector<std::thread*>	workerThreads;
	};
}

#endif