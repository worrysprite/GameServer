#ifndef __WS_DATABASE_H__
#define __WS_DATABASE_H__

#include <mysql.h>
#include <string>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include <thread>
#include <memory>
#include "utils/ByteArray.h"

namespace ws
{
	using namespace utils;

	class Database;

	struct MYSQL_CONFIG
	{
		unsigned int nPort;
		std::string strHost;
		std::string strUser;
		std::string strPassword;
		std::string strDB;
		std::string strUnixSock;
	};

	class DBStatement
	{
		friend class Database;
	public:
		DBStatement(const char* sql, MYSQL_STMT* mysql_stmt);
		~DBStatement();

		template<typename T>
		DBStatement& bindNumberParam(T& value, enum_field_types type, bool isUnsigned);
		DBStatement& operator<<(int8_t& value);
		DBStatement& operator<<(uint8_t& value);
		DBStatement& operator<<(int16_t& value);
		DBStatement& operator<<(uint16_t& value);
		DBStatement& operator<<(int32_t& value);
		DBStatement& operator<<(uint32_t& value);
		DBStatement& operator<<(int64_t& value);
		DBStatement& operator<<(uint64_t& value);
		DBStatement& operator<<(float& value);
		DBStatement& operator<<(double& value);
		DBStatement& operator<<(const std::string& value);
		DBStatement& operator<<(ByteArray& value);
		DBStatement& bindString(const char* value, unsigned long* length);
		void bindBlob(enum_field_types type, void* data, unsigned long* size);

		template<typename T>
		DBStatement& getNumberValue(T& value);
		DBStatement& operator>>(int8_t& value);
		DBStatement& operator>>(uint8_t& value);
		DBStatement& operator>>(int16_t& value);
		DBStatement& operator>>(uint16_t& value);
		DBStatement& operator>>(int32_t& value);
		DBStatement& operator>>(uint32_t& value);
		DBStatement& operator>>(int64_t& value);
		DBStatement& operator>>(uint64_t& value);
		DBStatement& operator>>(float& value);
		DBStatement& operator>>(double& value);
		DBStatement& operator>>(std::string& value);
		DBStatement& operator>>(ByteArray& value);
		void* getBlob(unsigned long& datasize);

		bool execute();
		bool nextRow();
		void reset();

		inline void skipFields(int num){ resultIndex += num; }
		inline int numParams(){ return _numParams; }
		inline int numResultFields(){ return _numResultFields; }
		inline my_ulonglong numRows(){ return _numRows; }
		inline my_ulonglong lastInsertId(){ return _lastInsertId; }
		inline const char* strSQL(){ return _strSQL.c_str(); }

	private:
		int				paramIndex;
		int				_numParams;
		int				resultIndex;
		int				_numResultFields;
		my_ulonglong	_numRows;
		my_ulonglong	_lastInsertId;
		std::string		_strSQL;
		MYSQL_STMT*		stmt;
		MYSQL_BIND*		paramBind;
		MYSQL_BIND*		resultBind;
		MYSQL_RES*		resultMetadata;
		void**			paramsBuffer;
	};

	class Recordset
	{
	public:
		Recordset(MYSQL_RES* pMysqlRes);
		virtual ~Recordset();

		bool nextRow();
		Recordset& operator >> (int8_t& value);
		Recordset& operator >> (uint8_t& value);
		Recordset& operator >> (int16_t& value);
		Recordset& operator >> (uint16_t& value);
		Recordset& operator >> (int32_t& value);
		Recordset& operator >> (uint32_t& value);
		Recordset& operator >> (int64_t& value);
		Recordset& operator >> (uint64_t& value);
		Recordset& operator >> (float& value);
		Recordset& operator >> (double& value);
		Recordset& operator >> (std::string& value);
		Recordset& operator >> (ByteArray& value);
		void*				getBlob(unsigned long& datasize);

	protected:
		unsigned int		fieldIndex;
		unsigned int		numFields;
		MYSQL_ROW			mysqlRow;
		MYSQL_RES*			mysqlRes;

		template<typename T>
		Recordset&			getInt(T& value);
	};
	typedef std::shared_ptr<Recordset> PtrDBRecord;

	class DBRequest
	{
	public:
		virtual ~DBRequest(){};
		virtual void onRequest(Database& db) = 0;
		virtual void onFinish() = 0;
	};
	typedef std::shared_ptr<DBRequest> PtrDBRequest;

	class Database
	{
	public:
		Database();
		Database(int _id) :id(_id){}
		virtual ~Database();
		
		/************************************************************************/
		/* init db connection                                                   */
		/************************************************************************/
		void setDBConfig(const MYSQL_CONFIG& config);
		bool logon();
		void logoff();

		inline bool isConnected()
		{
			if (mysql == NULL) return false;
			return (mysql_ping(mysql) == 0);
		}
		/************************************************************************/
		/* return num affected rows of query                                    */
		/************************************************************************/
		inline my_ulonglong				getAffectedRows() { return numAffectedRows; };
		/************************************************************************/
		/* return num result rows of query                                      */
		/************************************************************************/
		inline my_ulonglong				getResultRows() { return numResultRows; };
		/************************************************************************/
		/* return last insert id of query                                       */
		/************************************************************************/
		inline my_ulonglong				getInsertId()
		{
			if (mysql == NULL) return 0;
			return mysql_insert_id(mysql);
		}

		/************************************************************************/
		/* prepare a statement for query                                        */
		/************************************************************************/
		DBStatement*					prepare(const char* strSQL);
		void							release(DBStatement* dbStmt);
		PtrDBRecord						query(const char* strSQL, int nCommit = 1);
		
	protected:
		MYSQL_CONFIG						dbConfig;
		MYSQL*								mysql;
		my_ulonglong						numAffectedRows;
		my_ulonglong						numResultRows;
		typedef std::list<DBStatement*>		StmtPool;
		std::map<std::string, StmtPool*>	stmtCache;
		int									id;
	};

	class DBQueue
	{
	public:
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
		void				DBWorkThread(int id);

		MYSQL_CONFIG				config;
		std::mutex					workMtx;
		DBRequestList				workQueue;
		size_t						workQueueLength;
		std::mutex					finishMtx;
		DBRequestList				finishQueue;
		bool						isExit;
		std::vector<std::thread*>	workerThreads;
	};

}
#endif
