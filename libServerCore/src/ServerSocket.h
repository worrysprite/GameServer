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
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#define EPOLL_SIZE 5000
typedef int Socket;
#endif

#include "ClientManager.h"

#include <mutex>
#include <list>
#include <iostream>
#include <thread>
#include <functional>

#define BUFFER_SIZE 2048
#define ADDRESS_LENGTH sizeof(sockaddr_in)+16
#define NUM_ACCEPTEX 100

namespace ws
{
	class ClientManager;
	class ClientSocket;

	class ServerSocket
	{
	public:
		ServerSocket(ClientManager* manager);
		virtual ~ServerSocket();

		int startListen(unsigned short port);
		void sendToClient(ClientSocket* client);

#ifdef WIN32
		void closeClient(ClientSocket* client);
#endif
		
	protected:
		unsigned short listeningPort;
		Socket listenSocket;
		ClientManager* clientManager;

		int processEventThread();
		void writeClientBuffer(ClientSocket* client, char* data, size_t size);

#ifdef WIN32
		enum SocketOperation
		{
			ACCEPT,
			RECEIVE,
			SEND,
			CLOSE_CLIENT,
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
		std::mutex ioDataMutex;
		std::list<OverlappedData*> ioDataPosted;
		std::list<OverlappedData*> ioDataPool;
		std::list<std::thread*> eventThreads;

		int postAcceptEx();
		int getAcceptedSocketAddress(char* buffer, sockaddr_in* addr);
		void postCloseServer();

		OverlappedData* createOverlappedData(SocketOperation operation, size_t size = BUFFER_SIZE, Socket client = NULL);
		void releaseOverlappedData(OverlappedData* data);
		void initOverlappedData(OverlappedData& data, SocketOperation operation, size_t size = BUFFER_SIZE, Socket client = NULL);
#elif LINUX
		int epfd;
		bool isExit;
		std::thread* eventThread;

		inline int setNonblocking(int socketfd)
		{
			fcntl(socketfd, F_SETFL, fcntl(socketfd, F_GETFL, 0) | O_NONBLOCK);
		}
		void readIntoBuffer(ClientSocket* client);
		void writeFromBuffer(ClientSocket* client);

#endif

	};
}

#endif	//__SERVER_SOCKET_H__
