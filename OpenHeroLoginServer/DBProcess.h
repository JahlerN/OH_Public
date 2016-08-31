#pragma once

#include "..\SharedFiles\OdbcConnection.h"
#include "LoginSession.h"
typedef std::vector<_SERVER_TAB*> ServerInfoList;

class CDBProcess
{
public:
	bool Connect(TCHAR *szDSN, TCHAR *szUser, TCHAR *szPass);

//	bool LoadVersionList();
//	bool LoadUserCountList();

	uint16 AccountLogin(std::string& id, std::string& pwd);
	uint16 AccountCreate(std::string& id, std::string& pwd);
	bool LoadServerInformation(ServerInfoList& out);
	void UpdateServerInfo(ServerInfoList pVec);
	bool UpdateAccountSession(LoginSession* pSess);

private:
	OdbcConnection m_dbConnection;
};

#define SQL_SEL_LOGIN "SELECT COUNT(*) FROM ohdb.account WHERE accountId = ? AND pw = ?;"
#define SQL_INS_ACCOUNT "INSERT INTO ohdb.account (accountId, pw) VALUES (?, ?);"
#define SQL_INS_WAREHOUSE "INSERT INTO ohdb.warehouse(accountId) VALUES (?);"
#define SQL_SEL_SERVERTABS "SELECT tabId, tabName, publicIp, port FROM ohdb.servertabs;"
#define SQL_SEL_SERVERINFO "SELECT serverId, serverName, currentPlayers, maxPlayers, serverStatus FROM ohdb.servertable WHERE tabId = %d;"
#define SQL_SEL_UPDATE_SERVERINFO "SELECT currentPlayers, serverStatus FROM ohdb.servertable WHERE tabId = %d;"

#define SQL_UPD_ACCOUNT_SESSION "UPDATE ohdb.account SET lastIp = ?, serverTab = %d, serverId = %d WHERE accountId = ?;"