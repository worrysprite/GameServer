#include "ClientSocket.h"
#include "utils/Log.h"

#ifdef _WIN32

ws::ClientSocket::ClientSocket() :sockfd(0), workerThread(nullptr), status(DISCONNECTED),
isExit(false), _remotePort(0), lastStatus(DISCONNECTED)
{
	initWinsock();
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	workerThread = new std::thread(std::bind(&ClientSocket::workerProc, this));
}

ws::ClientSocket::~ClientSocket()
{
	isExit = true;
	workerThread->join();
	delete workerThread;
	WSACleanup();
}

int ws::ClientSocket::initWinsock()
{
	WORD wVersionRequested = MAKEWORD(2, 2); // request WinSock lib v2.2
	WSADATA wsaData;	// Windows Socket info struct
	DWORD err = WSAStartup(wVersionRequested, &wsaData);
	if (0 != err)
	{
		utils::Log::e("Request Windows Socket Library Error!");
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		utils::Log::e("Request Windows Socket Version 2.2 Error!");
		return -1;
	}
	return 0;
}

void ws::ClientSocket::reset()
{
	closesocket(sockfd);
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	readBuffer.lock();
	readBuffer.truncate();
	readBuffer.unlock();
	writeBuffer.lock();
	writeBuffer.truncate();
	writeBuffer.unlock();
}

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)

ws::ClientSocket::ClientSocket() :sockfd(0), workerThread(nullptr), status(DISCONNECTED),
isExit(false), _remotePort(0), lastStatus(DISCONNECTED)
{
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	workerThread = new std::thread(std::bind(&ClientSocket::workerProc, this));
}

ws::ClientSocket::~ClientSocket()
{
	isExit = true;
	workerThread->join();
	delete workerThread;
}

void ws::ClientSocket::reset()
{
	::close(sockfd);
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	readBuffer.lock();
	readBuffer.truncate();
	readBuffer.unlock();
	writeBuffer.lock();
	writeBuffer.truncate();
	writeBuffer.unlock();
}

#endif // WIN32

void ws::ClientSocket::workerProc()
{
	while (!isExit)
	{
		switch (status)
		{
		case ws::ClientSocket::CONNECTING:
		{
			sockaddr_in addr;
			memset(&addr, 0, sizeof(sockaddr_in));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(_remotePort);
			inet_pton(AF_INET, _remoteIP.c_str(), &addr.sin_addr);
			if (::connect(sockfd, (sockaddr*)&addr, sizeof(sockaddr_in)) == 0)
			{
				status = CONNECTED;
			}
			else
			{
				status = DISCONNECTED;
				utils::Log::d("ClientSocket connect failed");
			}
			break;
		}
		case ws::ClientSocket::CONNECTED:
		{
			if (!tryToRecv())
			{
				status = DISCONNECTED;
				break;
			}
			if (!tryToSend())
			{
				status = DISCONNECTED;
			}
			break;
		}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

bool ws::ClientSocket::tryToRecv()
{
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sockfd, &set);
	timeval time = {0, 0};
	while (select((int)sockfd + 1, &set, nullptr, nullptr, &time) > 0)
	{
		char buffer[BUFFER_SIZE];
		int length = recv(sockfd, buffer, BUFFER_SIZE, 0);
		if (length == 0 || length == -1)
		{
			return false;
		}
		readBuffer.lock();
		size_t oldPos = readBuffer.position;
		readBuffer.position = readBuffer.getSize();
		readBuffer.writeObject(buffer, length);
		readBuffer.position = oldPos;
		readBuffer.unlock();
		if (length < BUFFER_SIZE)
		{
			break;
		}
	}
	return true;
}

bool ws::ClientSocket::tryToSend()
{
	writeBuffer.lock();
	writeBuffer.position = 0;
	size_t remain = writeBuffer.getSize();
	while (remain)
	{
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t length = writeBuffer.readObject(buffer, BUFFER_SIZE);
		int sentLength = ::send(sockfd, buffer, (int)length, 0);
		if (sentLength == -1)
		{
			writeBuffer.unlock();
			return false;
		}
		else
		{
			if (sentLength < length)
			{
				writeBuffer.position -= length - sentLength;
			}
			remain -= sentLength;
		}
	}
	writeBuffer.cutHead(writeBuffer.position);
	writeBuffer.unlock();
	return true;
}

void ws::ClientSocket::connect(const std::string ip, uint16_t port)
{
	switch (status)
	{
	case ws::ClientSocket::DISCONNECTED:
		_remoteIP = ip;
		_remotePort = port;
		status = lastStatus = CONNECTING;
		break;
	case ws::ClientSocket::CONNECTING:
		Log::d("socket is connecting, please wait...");
		break;
	case ws::ClientSocket::CONNECTED:
		Log::d("socket is connected. you must disconnect before connect again.");
		break;
	}
}

void ws::ClientSocket::update()
{
	if (lastStatus != status)
	{
		lastStatus = status;
		switch (status)
		{
		case ws::ClientSocket::DISCONNECTED:
			reset();
			onClosed();
			break;
		case ws::ClientSocket::CONNECTED:
			onConnected();
			break;
		}
	}
}

void ws::ClientSocket::send(const ByteArray& packet)
{
	writeBuffer.lock();
	writeBuffer.writeBytes(packet);
	writeBuffer.unlock();
}

void ws::ClientSocket::send(const char* data, size_t length)
{
	writeBuffer.lock();
	writeBuffer.writeObject(data, length);
	writeBuffer.unlock();
}

void ws::ClientSocket::close()
{
	if (isConnected())
	{
		status = DISCONNECTED;
	}
}
