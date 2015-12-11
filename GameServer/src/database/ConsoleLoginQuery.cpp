#include "ConsoleLoginQuery.h"

ConsoleLoginQuery::ConsoleLoginQuery() : loginMsg(nullptr)
{

}

ConsoleLoginQuery::~ConsoleLoginQuery()
{
	if (loginMsg)
	{
		delete loginMsg;
		loginMsg = nullptr;
	}
}

void ConsoleLoginQuery::queryUserinfo(const ConsoleLoginMessage& msg, CallbackType callback)
{
	loginMsg = new ConsoleLoginMessage(msg);
	this->callback = callback;
}

void ConsoleLoginQuery::onRequest(Database& db)
{
	char sql[] = "SELECT `id` FROM `t_admin_user` WHERE `username`=%s AND `password`=%s;";
	char buffer[1024] = {0};
	sprintf(buffer, sql, loginMsg->username, loginMsg->password);
	std::shared_ptr<Recordset> record = db.query(buffer);
	if (record && record->MoveNext())
	{
		(*record) >> loginMsg->id;
	}
}

void ConsoleLoginQuery::onFinish()
{
	if (callback)
	{
		callback(loginMsg);
	}
	delete loginMsg;
	loginMsg = nullptr;
}
