#ifndef __UPGRADER_H__
#define __UPGRADER_H__

#include "Database.h"

using namespace ws;

class Patcher
{
public:
	Patcher() :_isFinished(false){}
	Patcher(const char* name) :_name(name), _isFinished(false), _successful(false){ }
	virtual ~Patcher(){}

	virtual void update() = 0;

	inline bool isFinished(){ return _isFinished; }
	inline bool successful(){ return _successful; }
	inline const std::string& name(){ return _name; }

protected:
	std::string				_name;
	bool					_isFinished;
	bool					_successful;

private:
};

typedef std::shared_ptr<Patcher> PtrPatcher;

class Upgrader
{
public:
	static bool init();
	static void update();
	static void cleanup();
	inline static bool isFinished(){ return _isFinished; }
	inline static DBQueue* getDBQueue(){ return dbQueue; }
	inline static const std::string& getDBName(){ return dbName; }
	inline static const std::string& getConfigDB(){ return configDB; }

private:
	static bool									_isFinished;
	static DBQueue*								dbQueue;
	static std::list<PtrPatcher>				patcherList;
	static std::string							dbName;
	static std::string							configDB;
};

#endif