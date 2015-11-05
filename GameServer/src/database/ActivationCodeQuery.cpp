#include "ActivationCodeQuery.h"
#include "Message.h"
#include <stdio.h>

void ActivationCodeQuery::onRequest(Database& db)
{
	switch (type)
	{
	case ActivationCodeQuery::QUERY_ACTIVATION_CODE:
	{
		ActivationMessage* activation = new ActivationMessage;
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
		data = activation;
		break;
	}
	case ActivationCodeQuery::UPDATE_ACTIVATION_STATUS:
	{
		char sql[] = "UPDATE `t_activation_code` SET `status`=%u WHERE `code`='%s';";
		char buffer[1024] = {0};
		sprintf(buffer, sql, status, code.c_str());
		db.query(buffer);
		data = new int((int)db.getAffectedRows());
		break;
	}
	default:
		break;
	}
}

void ActivationCodeQuery::queryActivationCode(const std::string& code)
{
	type = QUERY_ACTIVATION_CODE;
	this->code = code;
}

void ActivationCodeQuery::updateActivationStatus(const std::string& code, unsigned char status)
{
	type = UPDATE_ACTIVATION_STATUS;
	this->code = code;
	this->status = status;
}

