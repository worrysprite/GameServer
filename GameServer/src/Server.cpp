#include "Server.h"
#include "Client.h"

Server::Server()
{
	socket = new ServerSocket(this);

	MYSQL_CONFIG config;
#ifdef WIN32
	config.strHost = "192.168.11.151";
	config.strPassword = "";
#elif LINUX
	config.strHost = "localhost";
	config.strPassword = "";
	config.strUnixSock = "/tmp/mysql.sock";
#endif
	config.nPort = 3306;
	config.strUser = "root";
	config.strDB = "star2015";

	dbQueue = new DBRequestQueue;
	dbQueue->init(5, config);
}

Server::~Server()
{
	delete socket;
	delete dbQueue;
}

ClientSocket* Server::createClient()
{
	return new Client;
}

int Server::startListen(int port)
{
	return socket->startListen(port);
}

void Server::destroyClient(ClientSocket* cs)
{
	ClientManager::destroyClient(cs);
	delete cs;
}