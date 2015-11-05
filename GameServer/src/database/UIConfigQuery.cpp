#include "UIConfigQuery.h"
#include "Message.h"

void UIConfigQuery::queryUIConfig(unsigned char version, unsigned char platform)
{
	this->version = version;
	this->platform = platform;
}

void UIConfigQuery::onRequest(Database& db)
{
	data = nullptr;
	char sql[] = "SELECT `ui`, `open` FROM `t_ui_config` WHERE `version`=%u AND `platform`=%u;";
	char buffer[1024] = {0};
	sprintf(buffer, sql, version, platform);
	Recordset* record = db.query(buffer);
	size_t numRows = db.getResultRows();
	if (numRows > 0)
	{
		UIConfigMessage* message = new UIConfigMessage;
		message->numConfig = (unsigned short)numRows;
		message->configList = new UIConfig[numRows];
		for (int i(0); i < numRows; ++i)
		{
			if (record && record->MoveNext())
			{
				Recordset& row(*record);
				row >> message->configList[i].ui;
				row >> message->configList[i].open;
			}
			else
			{
				break;
			}
		}
		data = message;
	}
	db.cleanRecordset(record);
}

