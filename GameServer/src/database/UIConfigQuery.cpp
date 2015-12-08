#include "UIConfigQuery.h"

void UIConfigQuery::queryUIConfig(unsigned char version, unsigned char platform, CallbackType callback)
{
	this->version = version;
	this->platform = platform;
	this->callback = callback;
}

void UIConfigQuery::onRequest(Database& db)
{
	config = new UIConfigMessage;
	char sql[] = "SELECT `ui`, `open` FROM `t_ui_config` WHERE `version`=%u AND `platform`=%u;";
	char buffer[1024] = {0};
	sprintf(buffer, sql, version, platform);
	Recordset* record = db.query(buffer);
	size_t numRows = db.getResultRows();
	if (numRows > 0)
	{
		config->numConfig = (unsigned short)numRows;
		config->configList = new UIConfig[numRows];
		for (int i(0); i < numRows; ++i)
		{
			if (record && record->MoveNext())
			{
				Recordset& row(*record);
				row >> config->configList[i].ui;
				row >> config->configList[i].open;
			}
			else
			{
				break;
			}
		}
	}
	db.cleanRecordset(record);
}

void UIConfigQuery::onFinish()
{
	if (callback)
	{
		callback(config);
	}
	delete config;
	config = nullptr;
	callback = nullptr;
}

