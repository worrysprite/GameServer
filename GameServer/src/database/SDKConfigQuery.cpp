#include "SDKConfigQuery.h"
#include "Message.h"

void SDKConfigQuery::querySDKConfig(unsigned char version, unsigned char platform)
{
	this->version = version;
	this->platform = platform;
}

void SDKConfigQuery::onRequest(Database& db)
{
	data = nullptr;
	char sql[] = "SELECT `opensdk` FROM `t_sdk_config` WHERE `version`=%u AND `platform`=%u;";
	char buffer[1024] = {0};
	sprintf(buffer, sql, version, platform);
	Recordset* record = db.query(buffer);
	if (record && record->MoveNext())
	{
		SDKConfigMessage* message = new SDKConfigMessage;
		(*record) >> message->opensdk;
		data = message;
	}
	db.cleanRecordset(record);
}
