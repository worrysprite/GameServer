#include "ServerSocket.h"
#include "Log.h"
#include <errno.h>

namespace ws
{
#ifdef WIN32
	// main thread
	ServerSocket::ServerSocket(ClientManager* manager) : completionPort(nullptr), clientManager(manager), lpfnAcceptEx(nullptr)
	{
		
	}

	// main thread
	ServerSocket::~ServerSocket()
	{
		// stop event threads
		for (int i = 0; i < eventThreads.size(); ++i)
		{
			postCloseServer();
		}
		for (auto th : eventThreads)
		{
			th->join();
			Log::d("server socket event thread joined");
			delete th;
		}
		eventThreads.clear();

		// clear all caches
		for (auto ioData : ioDataPool)
		{
			if (ioData->acceptSocket)
			{
				closesocket(ioData->acceptSocket);
			}
			delete ioData;
		}
		for (auto ioData : ioDataPosted)
		{
			if (ioData->acceptSocket)
			{
				closesocket(ioData->acceptSocket);
			}
			delete ioData;
		}
		shutdown(listenSocket, SD_BOTH);
		closesocket(listenSocket);
		WSACleanup();
	}
#elif LINUX
	ServerSocket::ServerSocket(ClientManager* manager) : clientManager(manager), isExit(false)
	{
		
	}

	ServerSocket::~ServerSocket()
	{
		// stop event threads
		isExit = true;
		eventThread->join();
		Log::d("server socket event thread joined");
		delete eventThread;
		eventThread = NULL;
	}
#endif

	int ServerSocket::processEventThread()
	{
#ifdef WIN32
		DWORD BytesTransferred;
		LPOVERLAPPED lpOverlapped;
		ClientSocket* client = NULL;
		OverlappedData* ioData = NULL;
		DWORD numBytes;
		DWORD Flags = 0;
		BOOL bRet = false;

		while (true)
		{
			bRet = GetQueuedCompletionStatus(completionPort, &BytesTransferred,
				(PULONG_PTR)&client, (LPOVERLAPPED*)&lpOverlapped, INFINITE);
			ioData = (OverlappedData*)CONTAINING_RECORD(lpOverlapped, OverlappedData, overlapped);
			if (bRet == 0)
			{
				Log::e("GetQueuedCompletionStatus Error: %d", GetLastError());
				clientManager->removeClient(client);
				releaseOverlappedData(ioData);
				continue;
			}
			switch (ioData->operationType)
			{
			case SocketOperation::ACCEPT:
			{
				sockaddr_in localAddr;
				getAcceptedSocketAddress(ioData->buffer, &localAddr);
				client = clientManager->addClient(ioData->acceptSocket, localAddr);
				CreateIoCompletionPort((HANDLE)client->socket, completionPort, (ULONG_PTR)client, 0);

				initOverlappedData(*ioData, SocketOperation::RECEIVE);
				WSARecv(client->socket, &(ioData->wsabuff), 1, &numBytes, &Flags, &(ioData->overlapped), NULL);
				postAcceptEx();
				break;
			}

			case SocketOperation::RECEIVE:
			{
				if (BytesTransferred == 0)
				{
					clientManager->removeClient(client);
					releaseOverlappedData(ioData);
				}
				else
				{
					if (client->isClosing)	// ignore if closing
					{
						continue;
					}
					writeClientBuffer(client, ioData->buffer, BytesTransferred);
					clientManager->flagUpdate(client);
					initOverlappedData(*ioData, SocketOperation::RECEIVE);
					WSARecv(client->socket, &(ioData->wsabuff), 1, &numBytes, &Flags, &(ioData->overlapped), NULL);
				}
				break;
			}

			case SocketOperation::SEND:
			{
				if (BytesTransferred == 0)
				{
					clientManager->removeClient(client);
					releaseOverlappedData(ioData);
					Log::e("send error! %d", WSAGetLastError());
				}
				else
				{
					releaseOverlappedData(ioData);
				}
				break;
			}
			case SocketOperation::CLOSE_CLIENT:
			{
				clientManager->removeClient(client);
				releaseOverlappedData(ioData);
				break;
			}

			case SocketOperation::CLOSE_SERVER:
			{
				releaseOverlappedData(ioData);
				return 0;
			}
			}
		}
#elif LINUX
		epoll_event ev;
		ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP;
		epoll_event events[EPOLL_SIZE];
		sockaddr clientAddr;
		socklen_t addrlen = sizeof(sockaddr);
		while(true)
		{
			int eventCount = epoll_wait(epfd, events, EPOLL_SIZE, -1);
			for (int i = 0; i < eventCount; ++i)
			{
				epoll_event& evt(events[i]);
				if (evt.events & EPOLLERR || evt.events & EPOLLHUP)
				{
					// handle errors
				}
				if (evt.events & EPOLLIN)
				{
					if (evt.data.fd == listenSocket)
					{
						Socket acceptedSocket = accept(listenSocket, &clientAddr, &addrlen);
						if (acceptedSocket < 0)
						{
							continue;
						}
						setNonblocking(acceptedSocket);
						ev.data.ptr = clientManager->addClient(acceptedSocket, (sockaddr_in&)clientAddr);
						epoll_ctl(epfd, EPOLL_CTL_ADD, acceptedSocket, &ev);
					}
					else
					{
						ClientSocket* client = (ClientSocket*)evt.data.ptr;
						readIntoBuffer(client);
					}
				}
				if (evt.events & EPOLLOUT)
				{
					ClientSocket* client = (ClientSocket*)evt.data.ptr;
					writeFromBuffer(client);
				}
				if (evt.events & EPOLLRDHUP)
				{
					ClientSocket* client = (ClientSocket*)evt.data.ptr;
					clientManager->removeClient(client);
				}
			}
		}
#endif
		return 0;
	}	//end of processEvent

	int ServerSocket::startListen(unsigned short port)
	{
#ifdef WIN32
		WORD wVersionRequested = MAKEWORD(2, 2); // request WinSock lib v2.2
		WSADATA wsaData;	// Windows Socket info struct
		DWORD err = WSAStartup(wVersionRequested, &wsaData);

		if (0 != err)
		{
			Log::e("Request Windows Socket Library Error!");
			return -1;
		}
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			WSACleanup();
			Log::e("Request Windows Socket Version 2.2 Error!");
			return -1;
		}

		// create an i/o completion port
		completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (NULL == completionPort)
		{
			Log::e("CreateIoCompletionPort failed. Error: %d", GetLastError());
			return -1;
		}

		// get number of cores of cpu
		SYSTEM_INFO mySysInfo;
		GetSystemInfo(&mySysInfo);

		// create threads to process i/o completion port events
		std::function<int()> eventProc(std::bind(&ServerSocket::processEventThread, this));
		for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i)
		{
			std::thread* th = new std::thread(eventProc);
			eventThreads.push_back(th);
		}

		// create listen socket
		listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		// bind listen socket to i/o completion port
		CreateIoCompletionPort((HANDLE)listenSocket, completionPort, (ULONG_PTR)0, 0);

		// bind socket
		SOCKADDR_IN srvAddr;
		srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		srvAddr.sin_family = AF_INET;
		srvAddr.sin_port = htons(port);
		int result = bind(listenSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
		if (SOCKET_ERROR == result)
		{
			Log::e("Bind failed. Error: %d", GetLastError());
			return -1;
		}

		// start listen
		result = listen(listenSocket, 100);
		if (SOCKET_ERROR == result)
		{
			Log::e("Listen failed. Error: %d", GetLastError());
			return -1;
		}
		listeningPort = port;
		Log::i("server is listening port %d, waiting for clients...", port);

		//AcceptEx function pointer
		lpfnAcceptEx = NULL;
		//AcceptEx function GUID
		GUID guidAcceptEx = WSAID_ACCEPTEX;
		//get acceptex function pointer
		DWORD dwBytes = 0;
		result = WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
							&guidAcceptEx, sizeof(guidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx),
							&dwBytes, NULL, NULL);
		if (result != 0)
		{
			Log::e("WSAIoctl get AcceptEx function pointer failed... %d", WSAGetLastError());
			return -1;
		}

		//post acceptEx
		for (int i = 0; i < NUM_ACCEPTEX; ++i)
		{
			postAcceptEx();
		}
#elif LINUX
		listenSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (listenSocket < 0)
		{
			Log::e("create listen socket error.");
			return -1;
		}
		setNonblocking(listenSocket);
		
		sockaddr_in srvAddr;
		srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		srvAddr.sin_family = PF_INET;
		srvAddr.sin_port = htons(port);
		
		int result = bind(listenSocket, (sockaddr*)&srvAddr, sizeof(srvAddr));
		if (result < 0)
		{
			Log::e("bind port %d error. errno=%d", port, errno);
			return -1;
		}
		result = listen(listenSocket, 100);
		if (result < 0)
		{
			Log::e("listen port %d error.", port);
			return -1;
		}
		listeningPort = port;
		Log::i("server is listening port %d, waiting for clients...", port);

		epfd = epoll_create(EPOLL_SIZE);
		if (epfd < 0)
		{
			return -1;
		}
		epoll_event ev;
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = listenSocket;
		epoll_ctl(epfd, EPOLL_CTL_ADD, listenSocket, &ev);

		// create threads to process epoll events
		std::function<int()> eventProc(std::bind(&ServerSocket::processEventThread, this));
		eventThread = new std::thread(eventProc);
#endif
		return 0;
	}	//end of startListen

	// main thread
	void ServerSocket::sendToClient(ClientSocket* client)
	{
#ifdef WIN32
		if (client->isClosing)
		{
			return;
		}
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		client->writeBuffer->lock();
		client->writeBuffer->position = 0;
		size_t remain = client->writeBuffer->available();
		while (remain > 0)
		{
			size_t length = client->writeBuffer->readObject(buffer, BUFFER_SIZE);
			OverlappedData* sendData = createOverlappedData(SEND, length);
			memcpy(sendData->buffer, buffer, length);
			WSASend(client->socket, &(sendData->wsabuff), 1, NULL, 0, &(sendData->overlapped), NULL);
			if (length >= remain)
			{
				break;
			}
			remain = client->writeBuffer->available();
		}
		client->writeBuffer->cutHead(client->writeBuffer->position);
		client->writeBuffer->unlock();
		
#elif LINUX
		writeFromBuffer(client);
#endif
	}

	// socket thread
	void ServerSocket::writeClientBuffer(ClientSocket* client, char* data, size_t size)
	{
		client->readBuffer->lock();
		size_t oldPosition = client->readBuffer->position;
		// write at the end
		client->readBuffer->position = client->readBuffer->getSize();
		client->readBuffer->writeObject(data, size);
		client->readBuffer->position = oldPosition;
		client->readBuffer->unlock();
	}

#ifdef WIN32
	// main thread and socket threads
	ServerSocket::OverlappedData* ServerSocket::createOverlappedData(SocketOperation operation, size_t size /*= BUFFER_SIZE*/, SOCKET client /*= NULL*/)
	{
		OverlappedData* ioData = NULL;
		ioDataMutex.lock();
		if (!ioDataPool.empty())
		{
			ioData = ioDataPool.back();
			ioDataPool.pop_back();
		}
		else
		{
			ioData = new OverlappedData;
			ioData->wsabuff.buf = ioData->buffer;
		}
		initOverlappedData(*ioData, operation, size, client);
		ioDataPosted.push_back(ioData);
		ioDataMutex.unlock();
		return ioData;
	}

	// socket threads
	void ServerSocket::releaseOverlappedData(OverlappedData* data)
	{
		ioDataMutex.lock();
		ioDataPosted.remove(data);
		ioDataPool.push_back(data);
		ioDataMutex.unlock();
	}

	void ServerSocket::initOverlappedData(OverlappedData& data, SocketOperation operation, size_t size /*= BUFFER_SIZE*/, SOCKET client /*= NULL*/)
	{
		memset(&(data.overlapped), 0, sizeof(OVERLAPPED));
		memset(data.buffer, 0, BUFFER_SIZE);
		data.wsabuff.len = size;
		data.operationType = operation;
		data.acceptSocket = client;
	}

	// main thread and socket threads
	int ServerSocket::postAcceptEx()
	{
		SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
		OverlappedData* ioData = createOverlappedData(ACCEPT, BUFFER_SIZE, acceptSocket);

		DWORD dwBytes = 0;
		int result = lpfnAcceptEx(listenSocket, acceptSocket, ioData->buffer, 0,
			ADDRESS_LENGTH, ADDRESS_LENGTH, &dwBytes, &(ioData->overlapped));
		if (result == FALSE && WSAGetLastError() != ERROR_IO_PENDING)
		{
			Log::e("lpfnAcceptEx error.. %d", WSAGetLastError());
			return -1;
		}
		return 0;
	}

	// socket threads
	int ServerSocket::getAcceptedSocketAddress(char* buffer, sockaddr_in* addr)
	{
		//GetAcceptExSockAddrs function pointer
		LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockAddrs = NULL;
		//GetAcceptExSockAddrs function GUID
		GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
		//get GetAcceptExSockAddrs function pointer
		DWORD dwBytes = 0;
		int result = WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidGetAcceptExSockAddrs, sizeof(guidGetAcceptExSockAddrs), &lpfnGetAcceptExSockAddrs, sizeof(lpfnGetAcceptExSockAddrs),
			&dwBytes, NULL, NULL);
		if (result == 0)
		{
			sockaddr* localAddr = NULL;
			int localAddrLength;
			sockaddr* remoteAddr = NULL;
			int remoteAddrLength;
			lpfnGetAcceptExSockAddrs(buffer, 0, ADDRESS_LENGTH, ADDRESS_LENGTH, &localAddr, &localAddrLength, &remoteAddr, &remoteAddrLength);
			memcpy(addr, localAddr, localAddrLength);
			return 0;
		}
		else
		{
			return -1;
		}
	}

	// main thread
	void ServerSocket::postCloseServer()
	{
		OverlappedData* ioData = createOverlappedData(CLOSE_SERVER);
		PostQueuedCompletionStatus(completionPort, 0, listenSocket, &(ioData->overlapped));
	}

	// main thread
	void ServerSocket::closeClient(ClientSocket* client)
	{
		OverlappedData* ioData = createOverlappedData(CLOSE_CLIENT);
		PostQueuedCompletionStatus(completionPort, 0, (ULONG_PTR)client, &(ioData->overlapped));
	}

#elif LINUX
	// socket thread
	void ServerSocket::readIntoBuffer(ClientSocket* client)
	{
		if (client->isClosing)
		{
			return;
		}
		char buffer[BUFFER_SIZE];
		bzero(buffer, BUFFER_SIZE);
		int length = recv(client->socket, buffer, BUFFER_SIZE, 0);
		if (length <= 0)
		{
			clientManager->removeClient(client);
			return;
		}
		while (length > 0)
		{
			writeClientBuffer(client, buffer, length);
			if (length < BUFFER_SIZE)
			{
				break;
			}
			length = recv(client->socket, buffer, BUFFER_SIZE, 0);
		}
		clientManager->flagUpdate(client);
	}

	// main thread and socket thread
	void ServerSocket::writeFromBuffer(ClientSocket* client)
	{
		if (client->isClosing)
		{
			return;
		}
		char buffer[BUFFER_SIZE];
		bzero(buffer, BUFFER_SIZE);
		client->writeBuffer->lock();
		client->writeBuffer->position = 0;
		size_t remain = client->writeBuffer->available();
		while (remain > 0)
		{
			size_t length = client->writeBuffer->readObject(buffer, BUFFER_SIZE);
			long sentLength = send(client->socket, buffer, length, 0);
			if (sentLength < (long)length)	// tcp buffer full, stop write and wait epoll notify
			{
				client->writeBuffer->position -= length - sentLength;
				break;
			}
			if (length >= remain)
			{
				break;
			}
			remain = client->writeBuffer->available();
		}
		client->writeBuffer->cutHead(client->writeBuffer->position);
		client->writeBuffer->unlock();
	}

#endif
}
