#include "Upgrader.h"
#include <string.h>
#include <signal.h>
#include <map>
#include <string>
#include <vector>
#include "utils/Log.h"
#include "utils/String.h"

using namespace ws::utils;

bool Upgrader::_isFinished = false;
DBQueue* Upgrader::dbQueue = nullptr;
std::list<PtrPatcher> Upgrader::patcherList;
std::string Upgrader::dbName;
std::string Upgrader::configDB;


bool Upgrader::init()
{
	std::map<std::string, std::string> serverConfig;
	FILE* file = fopen("./config.ini", "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		auto size = ftell(file);
		fseek(file, 0, SEEK_SET);
		Log::d("config.ini size=%d", size);
		char* data = new char[size + 1];
		memset(data, 0, size + 1);
		if (size != fread(data, 1, size, file))
		{
			Log::e("read config.ini failed. %d", ferror(file));
			fclose(file);
			return false;
		}
		else
		{
			std::vector<char*> allRows;
#ifdef _WIN32
			String::split(data, "\r\n", allRows);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
			utils::String::split(data, "\n", allRows);
#endif
			for (auto row : allRows)
			{
				std::vector<char*> kv_pair;
				String::split(row, "=", kv_pair);
				if (kv_pair.size() == 2)
				{
					serverConfig[kv_pair[0]] = kv_pair[1];
				}
			}
		}
		delete[] data;
		fclose(file);
	}
	else
	{
		Log::e("open config.ini error: %s", strerror(errno));
		return false;
	}
	// init db
	MYSQL_CONFIG dbConfig;
	dbConfig.strHost = serverConfig["db_host"].c_str();
	dbConfig.nPort = atoi(serverConfig["db_port"].c_str());
	dbConfig.strUser = serverConfig["db_user"].c_str();
	dbConfig.strPassword = serverConfig["db_pass"].c_str();
	dbConfig.strDB = serverConfig["db_database"].c_str();
	dbName = serverConfig["db_database"];
	configDB = serverConfig["config_db"];
	if (configDB.empty())
	{
		configDB = dbName;
	}
#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
	dbConfig.strUnixSock = serverConfig["db_sock"].c_str();
#endif
	dbQueue = new DBQueue;
	dbQueue->init(5, dbConfig);
	return true;
}

void Upgrader::update()
{
	if (!patcherList.empty())
	{
		PtrPatcher currentPatcher = patcherList.front();
		if (currentPatcher->isFinished())
		{
			if (currentPatcher->successful())
			{
				Log::i("%s upgrade successful!", currentPatcher->name().c_str());
			}
			else
			{
				Log::i("%s upgrade failed!", currentPatcher->name().c_str());
			}
			patcherList.pop_front();
			return;
		}
		else
		{
			currentPatcher->update();
		}
		dbQueue->update();
	}
	else
	{
		_isFinished = true;
#ifdef _WIN32
		GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
		raise(SIGTERM);
#endif
	}
}

void Upgrader::cleanup()
{
	patcherList.clear();
	delete dbQueue;
}