#include "ClientManager.h"
#include "Log.h"

namespace ws
{
//===================== ClientManager Implements ========================
	ClientManager::ClientManager() : socket(nullptr), dbQueue(nullptr), disconnectTime(10)
	{
		
	}

	ClientManager::~ClientManager()
	{
		for (auto client : allClients)
		{
			destroyClient(client);
		}
		allClients.clear();
	}

	// socket threads
	ClientSocket* ClientManager::addClient(Socket client, const sockaddr_in &addr)
	{
		ClientSocket* cs = createClient();
		cs->lastActiveTime = std::chrono::steady_clock::now();
		cs->socket = client;
		cs->addr = addr;
		cs->manager = this;
		
		addMtx.lock();
		addingClients.insert(cs);
		addMtx.unlock();
		return cs;
	}

	// socket threads and main thread
	void ClientManager::removeClient(ClientSocket* cs)
	{
		removeMtx.lock();
		removingClients.insert(cs);
		removeMtx.unlock();
	}

	// main thread
	void ClientManager::destroyClient(ClientSocket* cs)
	{
#ifdef WIN32
		shutdown(cs->socket, SD_BOTH);
		closesocket(cs->socket);
#elif LINUX
		close(cs->socket);
#endif
	}

	// socket threads
	void ClientManager::flagUpdate(ClientSocket* client)
	{
		client->lastActiveTime = std::chrono::steady_clock::now();
		client->isUpdate = true;
	}

	// main thread
	void ClientManager::update()
	{
		removeMtx.lock();
		if (removingClients.size() > 0)
		{
			for (auto client : removingClients)
			{
				if (allClients.erase(client))
				{
					destroyClient(client);
				}
			}
			Log::v("%d client is disconnected. current online: %d", removingClients.size(), allClients.size());
			removingClients.clear();
		}
		removeMtx.unlock();

		addMtx.lock();
		if (addingClients.size() > 0)
		{
			allClients.insert(addingClients.begin(), addingClients.end());
			Log::v("%d client is connected. current online: %d", addingClients.size(), allClients.size());
			addingClients.clear();
		}
		addMtx.unlock();

		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		for (auto client : allClients)
		{
			if (client->isUpdate)
			{
				client->onRecv();
				client->isUpdate = false;
			}
			else if (now - client->lastActiveTime > disconnectTime)
			{
				closeClient(client);
			}
		}
		dbQueue->update();
	}

	// main thread
	void ClientManager::flush(ClientSocket* client)
	{
		assert(socket != NULL && client != NULL);
		socket->sendToClient(client);
	}

	// main thread
	void ClientManager::sendToAll(const char* data, size_t size)
	{
		assert(socket != NULL && data != NULL);
		for (auto client : allClients)
		{
			client->send(data, size);
			socket->sendToClient(client);
		}
	}

	// main thread
	void ClientManager::addDBRequest(DBRequest* request)
	{
		assert(dbQueue != NULL && request != NULL);
		dbQueue->addQueueMsg(request);
	}

	// main thread
	void ClientManager::closeClient(ClientSocket* cs)
	{
		assert(cs != NULL);
		cs->isClosing = true;
#ifdef WIN32
		socket->closeClient(cs);
#elif LINUX
		removeClient(cs);
#endif
	}

//===================== ClientSocket Implements ========================
	// socket threads
	ClientSocket::ClientSocket() : manager(nullptr), socket(0), readBuffer(nullptr), isClosing(false), writeBuffer(nullptr)
	{
		readBuffer = new ByteArray(BUFFER_SIZE);
		writeBuffer = new ByteArray(BUFFER_SIZE);
	}

	// socket threads
	ClientSocket::~ClientSocket()
	{
		delete readBuffer;
		delete writeBuffer;
	}

	// main thread
	void ClientSocket::send(const char* data, size_t length)
	{
		writeBuffer->lock();
		writeBuffer->position = writeBuffer->getSize();
		writeBuffer->writeObject(data, length);
		writeBuffer->unlock();
	}

	// main thread
	void ClientSocket::send(const ByteArray& packet)
	{
		writeBuffer->lock();
		writeBuffer->position = writeBuffer->getSize();
		writeBuffer->writeBytes(packet);
		writeBuffer->unlock();
	}

	// main thread
	void ClientSocket::flush()
	{
		assert(manager != nullptr);
		manager->flush(this);
	}

}

