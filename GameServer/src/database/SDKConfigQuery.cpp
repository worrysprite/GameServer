#include "SDKConfigQuery.h"

void SDKConfigQuery::querySDKConfig(unsigned char version, unsigned char platform, std::function<void(SDKConfigMessage*)> callback)
{
	this->version = version;
	this->platform = platform;
	this->callback = callback;
}

void SDKConfigQuery::onRequest(Database& db)
{
	config = nullptr;
	char sql[] = "SELECT `opensdk` FROM `t_sdk_config` WHERE `version`=%u AND `platform`=%u;";
	char buffer[1024] = {0};
	sprintf(buffer, sql, version, platform);
	Recordset* record = db.query(buffer);
	if (record && record->MoveNext())
	{
		config = new SDKConfigMessage;
		(*record) >> config->opensdk;
	}
	db.cleanRecordset(record);
}

void SDKConfigQuery::onFinish()
{
	if (callback)
	{
		callback(config);
	}
	delete config;
	config = nullptr;
	callback = nullptr;
}
