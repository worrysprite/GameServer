#ifndef __WS_CLIENT_MANAGER_H__
#define __WS_CLIENT_MANAGER_H__

#include <assert.h>
#include <vector>
#include <list>
#include <set>
#include <mutex>
#include "ServerSocket.h"
#include "ByteArray.h"
#include "Database.h"
#include "Timer.h"

namespace ws
{
	class ServerSocket;
	class ClientManager;

	class ClientSocket
	{
		friend class ServerSocket;
	public:
		ClientSocket();
		virtual ~ClientSocket();

		sockaddr_in addr;
		Socket socket;
		bool isClosing;
		bool isUpdate;
		std::chrono::steady_clock::time_point lastActiveTime;

		ClientManager* manager;

		virtual void onRecv() = 0;
		virtual void send(const char* data, size_t length);
		virtual void send(const ByteArray& packet);
		virtual void flush();

	protected:
		ByteArray* readBuffer;
		ByteArray* writeBuffer;
	};

	class ClientManager
	{
	public:
		ClientManager();
		virtual ~ClientManager();

		virtual ClientSocket* addClient(Socket client, const sockaddr_in &addr);
		virtual void removeClient(ClientSocket* cs);
		unsigned long getCount() { return allClients.size(); }

		virtual void flagUpdate(ClientSocket* client);
		virtual void update();

		void flush(ClientSocket* client);
		void sendToAll(const char* data, size_t size);
		void addDBRequest(DBRequest* request);
		void closeClient(ClientSocket* cs);

	protected:
		ServerSocket* socket;
		DBRequestQueue* dbQueue;

		std::mutex addMtx;
		std::set<ClientSocket*> addingClients;
		std::mutex removeMtx;
		std::set<ClientSocket*> removingClients;
		std::set<ClientSocket*> allClients;

		std::chrono::minutes disconnectTime;

		// must be override
		virtual ClientSocket* createClient() = 0;
		virtual void destroyClient(ClientSocket* cs);
	};
}

#endif	//__WS_CLIENT_MANAGER_H__
