#ifndef __WS_DATABASE_H__
#define __WS_DATABASE_H__

#include <mysql.h>
#include <string>
#include <list>
#include <mutex>
#include <vector>
#include <thread>
#include <memory>

namespace ws
{
	struct MYSQL_CONFIG
	{
		unsigned int nPort;
		std::string strHost;
		std::string strUser;
		std::string strPassword;
		std::string strDB;
		std::string strUnixSock;
	};

	class Recordset
	{
	public:
		Recordset(MYSQL_RES* pMysqlRes);
		virtual ~Recordset();

		bool MoveNext();
		bool operator >> (unsigned long long& nValue);
		bool operator >> (unsigned int& nValue);
		bool operator >> (unsigned short& nValue);
		bool operator >> (int& nValue);
		bool operator >> (short& nValue);
		bool operator >> (unsigned char& nValue);
		bool operator >> (char& nValue);
		bool operator >> (std::string& strValue);
		bool operator >> (float& value);
		bool operator >> (double& value);

	protected:
		unsigned int       m_nIndex;
		unsigned int       m_NumFields;
		MYSQL_ROW  m_MysqlRow;
		MYSQL_RES* m_pMysqlRes;
	};

	class Database
	{
	public:
		Database();
		virtual ~Database();

		void setDBConfig(const MYSQL_CONFIG& config);

		bool logon();
		void logoff();
		bool isConnected();
		inline my_ulonglong getAffectedRows() { return numAffectedRows; };
		inline my_ulonglong getResultRows() { return numResultRows; };
		std::shared_ptr<Recordset> query(const char* strSQL, int nCommit = 1);
		my_ulonglong getInsertId();

	protected:
		MYSQL_CONFIG dbConfig;
		MYSQL* mysql;
		my_ulonglong numAffectedRows;
		my_ulonglong numResultRows;

	private:
	};

	class DBRequest
	{
	public:
		virtual ~DBRequest(){};
		virtual void onRequest(Database& db) = 0;
		virtual void onFinish() = 0;
	};

	class DBQueue
	{
	public:
		typedef std::shared_ptr<DBRequest> PtrDBRequest;

		DBQueue();
		virtual ~DBQueue();

		void				init(int nThread, const MYSQL_CONFIG& config);
		void				addQueueMsg(PtrDBRequest request);
		void				update();
		inline size_t		getQueueLength(){ return workQueueLength; }

	private:
		typedef std::list<PtrDBRequest> DBRequestList;
		PtrDBRequest		getRequest();
		void				finishRequest(PtrDBRequest request);
		void				DBWorkThread();

		MYSQL_CONFIG		config;
		std::mutex			workMtx;
		DBRequestList		workQueue;
		size_t				workQueueLength;
		std::mutex			finishMtx;
		DBRequestList		finishQueue;
		bool				isExit;
		std::vector<std::thread*> workerThreads;
	};

}
#endif
