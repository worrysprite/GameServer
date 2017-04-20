#ifndef __WS_SERVER_SOCKET_H__
#define __WS_SERVER_SOCKET_H__

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")		// win32 Socket lib
#pragma comment(lib, "Kernel32.lib")	// IOCP lib
#ifndef Socket
typedef SOCKET Socket;
#endif
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#define EPOLL_SIZE 1000
#ifndef Socket
typedef int Socket;
#endif
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
#include "Database.h"
#include "utils/ByteArray.h"
#include "utils/ObjectPool.h"

#define BUFFER_SIZE 1024
#define ADDRESS_LENGTH sizeof(sockaddr_in)+16
#define NUM_ACCEPTEX 100

namespace ws
{
	using namespace utils;
	using namespace std::chrono;

	class ServerSocket;
	class Client
	{
		friend class ServerSocket;
	public:
		Client();
		virtual ~Client();

		long long					id;
		unsigned long long			lastActiveTime;

		std::string		getIP()
		{
			char buffer[20] = {0};
			std::string result(inet_ntop(AF_INET, (void*)&addr.sin_addr, buffer, 20));
			return result;
		}
		virtual void	onRecv() = 0;
		virtual void	onDisconnected(){}
		virtual void	update(){}
		virtual void	send(const char* data, size_t length);
		virtual void	send(const ByteArray& packet);
		inline void		kick(){ isClosing = true; }

	protected:
		Socket					socket;
		sockaddr_in				addr;
		ServerSocket*			server;
		ByteArray*				readBuffer;
		ByteArray*				writeBuffer;
	private:
		bool					isClosing;
		bool					hasData;
	};

	struct ServerConfig
	{
		ServerConfig() :listenPort(0), maxConnection(0), numIOCPThreads(0), kickTime(0){}
		unsigned short							listenPort;
		unsigned int							maxConnection;
		unsigned char							numIOCPThreads;
		unsigned long long						kickTime;
		std::function<Client*()>				createClient;
		std::function<void(Client*)>			destroyClient;
	};

	class ServerSocket
	{
		friend class Client;
	public:
		ServerSocket(const ServerConfig& cfg);
		virtual ~ServerSocket();

		int											startListen();
		void										update();
		bool										kickClient(long long clientID);
		Client*										getClient(long long clientID);
		const std::map<long long, Client*>&			getAllClients() const { return allClients; }
		size_t										numOnlines();
		
	private:
		ServerConfig								config;
		long long									nextClientID;
		size_t										numClients;
		Socket										listenSocket;
		std::mutex									addMtx;
		std::map<long long, Client*>				addingClients;
		std::map<long long, Client*>				allClients;
		
		int							processEventThread();
		void						writeClientBuffer(Client* client, char* data, size_t size);
		Client*						addClient(Socket client, const sockaddr_in &addr);
		void						destroyClient(Client* client);
		void						flushClient(Client* client);

#ifdef _WIN32
	public:
		inline size_t getIODataPoolSize(){ return ioDataPoolSize; }
		inline size_t getIODataPostedSize(){ return ioDataPostedSize; }

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
		size_t ioDataPoolSize;
		size_t ioDataPostedSize;
		std::list<std::thread*> eventThreads;

		int initWinsock();
		int postAcceptEx();
		int getAcceptedSocketAddress(char* buffer, sockaddr_in* addr);
		void postCloseServer();

		OverlappedData* createOverlappedData(SocketOperation operation, size_t size = BUFFER_SIZE, Socket client = NULL);
		void releaseOverlappedData(OverlappedData* data);
		void initOverlappedData(OverlappedData& data, SocketOperation operation, size_t size = BUFFER_SIZE, Socket client = NULL);

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
	private:
		int epfd, pipe_fd[2];
		bool isExit;
		std::thread* eventThread;

		void readIntoBuffer(Client* client);
		void writeFromBuffer(Client* client);
		unsigned long long GetTickCount64();
#endif
	};
}

#endif	//__SERVER_SOCKET_H__
