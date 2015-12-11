#ifndef __WS_SERVER_SOCKET_H__
#define __WS_SERVER_SOCKET_H__

#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>
#pragma comment(lib, "Ws2_32.lib")		// win32 Socket lib
#pragma comment(lib, "Kernel32.lib")	// IOCP lib
typedef SOCKET Socket;
#elif LINUX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#define EPOLL_SIZE 5000
typedef int Socket;
#endif

#include <assert.h>
#include <mutex>
#include <list>
#include <iostream>
#include <thread>
#include <functional>

#include <vector>
#include <set>
#include <map>
#include "ByteArray.h"
#include "Database.h"
#include "Timer.h"
#include "ObjectPool.h"

#define BUFFER_SIZE 1024
#define ADDRESS_LENGTH sizeof(sockaddr_in)+16
#define NUM_ACCEPTEX 100

namespace ws
{
	class ServerSocket;
	class ClientSocket
	{
		friend class ServerSocket;
	public:
		ClientSocket();
		virtual ~ClientSocket();

		long long				id;
		bool					isUpdate;
		std::chrono::steady_clock::time_point lastActiveTime;

		char*			getIP() { return inet_ntoa(addr.sin_addr); }
		virtual void	onRecv() = 0;
		virtual void	send(const char* data, size_t length);
		virtual void	send(const ByteArray& packet);
		virtual void	flush();

	protected:
		bool					isClosing;
		Socket					socket;
		sockaddr_in				addr;
		ServerSocket*			server;
		ByteArray*				readBuffer;
		ByteArray*				writeBuffer;
	};

	struct ServerConfig
	{
		unsigned short							listenPort;
		unsigned int							maxConnection;
		unsigned char							numIOCPThreads;
		std::chrono::minutes					kickTime;
		std::function<ClientSocket*()>			createClient;
		std::function<void(ClientSocket*)>		destroyClient;
	};

	class ServerSocket
	{
		friend class ClientSocket;
	public:
		ServerSocket(const ServerConfig& cfg);
		virtual ~ServerSocket();

		int					startListen();
		void				update();
		void				kickClient(long long clientID);
		ClientSocket*		getClient(long long clientID);
		size_t				getCount();
		
	private:
		ServerConfig							config;
		long long								nextClientID;
		size_t									numClients;
		Socket									listenSocket;
		std::mutex								addMtx;
		std::map<long long, ClientSocket*>		addingClients;
		std::map<long long, ClientSocket*>		allClients;
		
		int					processEventThread();
		void				writeClientBuffer(ClientSocket* client, char* data, size_t size);
		ClientSocket*		addClient(Socket client, const sockaddr_in &addr);
		void				destroyClient(ClientSocket* client);
		void				flushClient(ClientSocket* client);

#ifdef WIN32
	private:
		enum SocketOperation
		{
			ACCEPT,
			RECEIVE,
			SEND,
			CLOSE_SERVER
		};
		
		struct OverlappedData
		{
			OVERLAPPED overlapped;
			WSABUF wsabuff;
			char buffer[BUFFER_SIZE];
			SocketOperation operationType;
			Socket acceptSocket;
		};

		HANDLE completionPort;
		LPFN_ACCEPTEX lpfnAcceptEx;
		
		ObjectPool<OverlappedData> ioDataPool;
		std::list<OverlappedData*> ioDataPosted;
		std::list<std::thread*> eventThreads;
		static bool isInitWinsock;

		int initWinsock();
		int postAcceptEx();
		int getAcceptedSocketAddress(char* buffer, sockaddr_in* addr);
		void postCloseServer();

		OverlappedData* createOverlappedData(SocketOperation operation, size_t size = BUFFER_SIZE, Socket client = NULL);
		void releaseOverlappedData(OverlappedData* data);
		void initOverlappedData(OverlappedData& data, SocketOperation operation, size_t size = BUFFER_SIZE, Socket client = NULL);

#elif LINUX
	private:
		int epfd;
		bool isExit;
		std::thread* eventThread;

		void readIntoBuffer(ClientSocket* client);
		void writeFromBuffer(ClientSocket* client);
#endif
	};
}

#endif	//__SERVER_SOCKET_H__
