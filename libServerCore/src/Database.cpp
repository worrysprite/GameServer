#include "Database.h"
#include "Log.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>

namespace ws
{
//===================== Recordset Implements ========================
	Recordset::Recordset(MYSQL_RES* pMysqlRes) :m_pMysqlRes(pMysqlRes)
	{
		m_NumFields = mysql_num_fields(m_pMysqlRes);
		m_MysqlRow = NULL;
	}

	Recordset::~Recordset()
	{
		mysql_free_result(m_pMysqlRes);
		m_pMysqlRes = NULL;
	}

	bool Recordset::MoveNext()
	{
		m_nIndex = 0x00;
		m_MysqlRow = mysql_fetch_row(m_pMysqlRes);
		return (m_MysqlRow != NULL);
	}

	bool Recordset::operator >> (unsigned long long& nValue)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		nValue = strtoull(szRow, (char**)(szRow + strlen(szRow)), 10);
		return true;
	}

	bool Recordset::operator >> (unsigned int& nValue)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		nValue = atoi(szRow);
		return true;
	}

	bool Recordset::operator >> (unsigned short& nValue)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		nValue = atoi(szRow);
		return true;
	}

	bool Recordset::operator >> (int& nValue)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		nValue = atoi(szRow);
		return true;
	}

	bool Recordset::operator >> (short& nValue)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		nValue = atoi(szRow);
		return true;
	}

	bool Recordset::operator >> (unsigned char& nValue)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		nValue = atoi(szRow);
		return true;
	}

	bool Recordset::operator >> (char& nValue)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		nValue = atoi(szRow);
		return true;
	}

	bool Recordset::operator >> (std::string& strValue)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		strValue = szRow;
		return true;
	}

	bool Recordset::operator>>(float& value)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		value = strtof(szRow, NULL);
		return true;
	}

	bool Recordset::operator>>(double& value)
	{
		if ((m_MysqlRow == NULL) || (m_nIndex >= m_NumFields))
			return false;
		const char* szRow = m_MysqlRow[m_nIndex++];
		if (szRow == NULL) return false;
		value = strtod(szRow, NULL);
		return true;
	}

//===================== Database Implements ========================
	Database::Database() :mysql(nullptr)
	{

	}

	Database::~Database()
	{
		logoff();
	}

	void Database::setDBConfig(const MYSQL_CONFIG& config)
	{
		dbConfig = config;
	}

	bool Database::logon()
	{
		static std::mutex s_dbInitLock;
		s_dbInitLock.lock();
		logoff();
		mysql = mysql_init(NULL);
		s_dbInitLock.unlock();

		if (mysql == NULL) return false;

		MYSQL* pMysql = mysql_real_connect(mysql, dbConfig.strHost.c_str(),
			dbConfig.strUser.c_str(), dbConfig.strPassword.c_str(),
			dbConfig.strDB.c_str(), dbConfig.nPort, dbConfig.strUnixSock.c_str(), CLIENT_MULTI_RESULTS);

		if (pMysql)
		{
			mysql_set_character_set(pMysql, "utf8");
			Log::i("mysql connect successful.");
		}
		else
		{
			Log::e("Fail to connect to mysql: %s", mysql_error(mysql));
		}
		return (pMysql != NULL) ? (mysql_autocommit(pMysql, 1) == 0) : false;
	}

	void Database::logoff()
	{
		if (mysql != NULL)
		{
			mysql_close(mysql);
			mysql = NULL;
		}
	}

	bool Database::isConnected()
	{
		if (mysql == NULL) return false;
		return (mysql_ping(mysql) == 0);
	}

	Recordset* Database::query(const char* strSQL, int nCommit /*= 1*/)
	{
		Recordset* pRecordset = nullptr;
		const char* pError = NULL;
		if (mysql_query(mysql, strSQL) == 0)
		{
			numAffectedRows = mysql_affected_rows(mysql);
			MYSQL_RES* pMysqlRes = mysql_store_result(mysql);
			while (mysql_next_result(mysql) == 0x00);

			if (pMysqlRes != NULL)
			{
				numResultRows = mysql_num_rows(pMysqlRes);
				if (numResultRows > 0x00)
				{
					pRecordset = new Recordset(pMysqlRes);
				}
				else if (pMysqlRes != NULL) mysql_free_result(pMysqlRes);
				if (nCommit) mysql_commit(mysql);
			}
		}
		else
		{
			numResultRows = 0;
			numAffectedRows = 0;
			pError = mysql_error(mysql);
			if (pError)
			{
				Log::e("DBError: %s, SQL: %s", pError, strSQL);
			}
		}
		return pRecordset;
	}

	my_ulonglong Database::getInsertId()
	{
		if (mysql == NULL) return 0;
		return mysql_insert_id(mysql);
	}

	void Database::cleanRecordset(Recordset* recordset)
	{
		delete recordset;
	}

//===================== DBRequestQueue Implements ========================
	DBQueue::DBQueue() : isExit(false)
	{

	}

	DBQueue::~DBQueue()
	{
		isExit = true;
		for (auto th : workerThreads)
		{
			th->join();
			delete th;
			Log::d("db worker thread joined.");
		}
	}

	void DBQueue::init(int nThread, const MYSQL_CONFIG& config)
	{
		this->config = config;
		for (int i = 0; i < nThread; i++)
		{
			std::thread* th(new std::thread(std::bind(&DBQueue::DBWorkThread, this)));
			workerThreads.push_back(th);
		}
	}

	void DBQueue::addQueueMsg(PtrDBRequest request)
	{
		m_WorkMutex.lock();
		m_WorkQueue.push_back(request);
		m_WorkMutex.unlock();
	}

	// main thread
	void DBQueue::update()
	{
		DBRequestList finishQueue;
		m_FinishMutex.lock();
		m_FinishQueue.swap(finishQueue);
		m_FinishMutex.unlock();

		while (!finishQueue.empty())
		{
			PtrDBRequest request = finishQueue.front();
			finishQueue.pop_front();
			request->onFinish();
		}
	}

	DBQueue::PtrDBRequest DBQueue::getRequest()
	{
		PtrDBRequest request(nullptr);
		m_WorkMutex.lock();
		if (!m_WorkQueue.empty())
		{
			request = m_WorkQueue.front();
			m_WorkQueue.pop_front();
		}
		m_WorkMutex.unlock();
		return request;
	}

	void DBQueue::finishRequest(PtrDBRequest request)
	{
		m_FinishMutex.lock();
		m_FinishQueue.push_back(request);
		m_FinishMutex.unlock();
	}

	void DBQueue::DBWorkThread()
	{
		Database db;
		db.setDBConfig(config);
		while (!this->isExit)
		{
			while (!db.isConnected())
			{
				db.logon();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
			PtrDBRequest request(getRequest());
			if (!request)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
			request->onRequest(db);
			finishRequest(request);
		}
	}
}
