#include "ActivationCodeQuery.h"
#include <stdio.h>

void ActivationCodeQuery::onRequest(Database& db)
{
	switch (type)
	{
	case ActivationCodeQuery::QUERY_ACTIVATION_CODE:
	{
		activation = new ActivationMessage;
		activation->code = code;
		char sql[] = "SELECT a.`code`, a.`reward`, a.`status`, b.`coin`, b.`bomb`, b.`shield`, b.`plane2`, b.`plane3`, b.`plane4` FROM t_activation_code AS a INNER JOIN t_reward AS b ON a.`code`='%s' AND a.`reward`=b.`id`;";
		char buffer[1024] = {0};
		sprintf(buffer, sql, code.c_str());
		Recordset* record = db.query(buffer);
		if (record && record->MoveNext())
		{
			Recordset& row(*record);
			row >> activation->code;
			row >> activation->reward;
			row >> activation->status;
			row >> activation->coin;
			row >> activation->bomb;
			row >> activation->shield;
			row >> activation->plane2;
			row >> activation->plane3;
			row >> activation->plane4;
		}
		else
		{
			activation->status = ActivationStatus::INVALID_CODE;
		}
		db.cleanRecordset(record);
		break;
	}
	case ActivationCodeQuery::UPDATE_ACTIVATION_STATUS:
	{
		char sql[] = "UPDATE `t_activation_code` SET `status`=%u WHERE `code`='%s';";
		char buffer[1024] = {0};
		sprintf(buffer, sql, status, activation->code.c_str());
		db.query(buffer);
		if (db.getAffectedRows() <= 0)
		{
			activation->status = INVALID_CODE;
		}
		break;
	}
	default:
		break;
	}
}

void ActivationCodeQuery::queryActivationCode(const std::string& code, CallbackType callback)
{
	type = QUERY_ACTIVATION_CODE;
	this->code = code;
	this->queryCallback = callback;
}

void ActivationCodeQuery::updateActivationStatus(const ActivationMessage& code, unsigned char status, CallbackType callback)
{
	type = UPDATE_ACTIVATION_STATUS;
	this->activation = new ActivationMessage(code);
	this->status = status;
	this->updateCallback = callback;
}

void ActivationCodeQuery::onFinish()
{
	if (queryCallback)
	{
		queryCallback(activation);
	}
	queryCallback = nullptr;
	delete activation;
	activation = nullptr;
}

