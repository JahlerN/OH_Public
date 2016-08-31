#pragma once
#include "stdafx.h"

bool CDBProcess::Connect(TCHAR *szDSN, TCHAR *szUser, TCHAR *szPass)
{
	if (!m_dbConnection.Connect(szDSN, szUser, szPass))
	{
		//g_main.ReportSQLError(m_dbConnection.GetError());
		return false;
	}

	return true;
}

//bool CDBProcess::LoadVersionList()
//{
//	bool result = false;
//	auto_ptr<OdbcCommand> dbCommand(m_dbConnection.CreateCommand());
//
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T("SELECT sVersion, sHistoryVersion, strFileName FROM VERSION")))
//	{
//		g_main.ReportSQLError(m_dbConnection.GetError());
//		return false;
//	}
//
//	if (dbCommand->hasData())
//	{
//		g_main.m_sLastVersion = 0;
//		do
//		{
//			_VERSION_INFO *pVersion = new _VERSION_INFO;
//
//			dbCommand->FetchUInt16(1, pVersion->sVersion);
//			dbCommand->FetchUInt16(2, pVersion->sHistoryVersion);
//			dbCommand->FetchString(3, pVersion->strFileName);
//
//			g_pMain.m_VersionList.insert(make_pair(pVersion->strFileName, pVersion));
//
//			if (g_pMain.m_sLastVersion < pVersion->sVersion)
//				g_pMain.m_sLastVersion = pVersion->sVersion;
//
//		} while (dbCommand->MoveNext());
//	}
//
//	return true;
//}

//bool CDBProcess::LoadUserCountList()
//{
//	std::auto_ptr<OdbcCommand> dbCommand(m_dbConnection.CreateCommand());
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T("SELECT serverid, zone1_count, zone2_count, zone3_count FROM CONCURRENT")))
//	{
//		//g_main.ReportSQLError(m_dbConnection.GetError());
//		return false;
//	}
//
//	if (dbCommand->hasData())
//	{
//		do
//		{
//			uint16 zone_1, zone_2, zone_3; uint8 serverID;
//			dbCommand->FetchByte(1, serverID);
//			dbCommand->FetchUInt16(2, zone_1);
//			dbCommand->FetchUInt16(3, zone_2);
//			dbCommand->FetchUInt16(4, zone_3);
//
//			if ((uint8)(serverID - 1) < g_main->m_serverList.size())
//				g_main->m_serverList[serverID - 1]->userCount = zone_1 + zone_2 + zone_3;
//		} while (dbCommand->MoveNext());
//	}
//
//	return true;
//}

uint16 CDBProcess::AccountLogin(std::string& id, std::string& pwd)
{
	uint16 result = 2; // account not found
	auto_ptr<OdbcCommand> dbCommand(m_dbConnection.CreateCommand());
	if (dbCommand.get() == NULL)
		return result;

	dbCommand->AddParameter(SQL_PARAM_INPUT, id.c_str(), id.length());
	dbCommand->AddParameter(SQL_PARAM_INPUT, pwd.c_str(), pwd.length());

	if (dbCommand->Execute((const tstring)SQL_SEL_LOGIN))
	{
		return dbCommand->FetchByte(1);
	}
	//printf(dbCommand->GetError());

	return result;
}

uint16 CDBProcess::AccountCreate(std::string& id, std::string& pwd)
{
	string aId = id.substr(0, id.length() - 2);

	SQLHSTMT hStmt;
	SQLAllocHandle(SQL_HANDLE_STMT, m_dbConnection.GetConnectionHandle(), &hStmt);
	SQLPrepare(hStmt, (SQLCHAR*)SQL_INS_ACCOUNT, sizeof(SQL_INS_ACCOUNT));
	SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)aId.c_str(), aId.length(), NULL);
	SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)pwd.c_str(), pwd.length(), NULL);

	SQLRETURN ret = SQLExecute(hStmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

	//Create warehouse
	auto_ptr<OdbcCommand> dbCommand(m_dbConnection.CreateCommand());
	if (dbCommand.get() == NULL)
		return 6;

	dbCommand->AddParameter(SQL_PARAM_INPUT, aId.c_str(), aId.length());

	if (!dbCommand->Execute(_T(SQL_INS_WAREHOUSE)))
		return 6;

	if (ret == SQL_ERROR)
	{
		return 6;
	}
	printf("Account %s created!", aId.c_str());

	return 5;
}

bool CDBProcess::LoadServerInformation(ServerInfoList& out)
{
	auto_ptr<OdbcCommand> dbCommand(m_dbConnection.CreateCommand());

	if (dbCommand.get() == NULL)
		return false;

	if (!dbCommand->Execute(SQL_SEL_SERVERTABS))
		return false;

	if (!dbCommand->hasData())
		return false;

	do
	{
		_SERVER_TAB* pTab = new _SERVER_TAB();
		dbCommand->FetchByte(1, pTab->m_tabId);
		dbCommand->FetchString(2, pTab->m_tabName);
		dbCommand->FetchString(3, pTab->m_serverIp);
		dbCommand->FetchUInt16(4, pTab->m_port);
		
		auto_ptr<OdbcCommand> dbCommand2(m_dbConnection.CreateCommand());

		if (dbCommand2.get() == NULL)
			return false;

		if (!dbCommand2->Execute(_T(string_format(SQL_SEL_SERVERINFO, pTab->m_tabId))))
			return false;

		if (!dbCommand2->hasData())
			return false;

		do
		{
			int field = 1;
			_SERVER_INFO* pInfo = new _SERVER_INFO();
			pInfo->m_tabId = pTab->m_tabId;

			dbCommand2->FetchByte(field++, pInfo->m_serverId);
			dbCommand2->FetchString(field++, pInfo->m_serverName);
			pInfo->m_serverName.append(string_format("\x20%d", pInfo->m_serverId + 1));
			dbCommand2->FetchUInt16(field++, pInfo->m_curPlayers);
			dbCommand2->FetchUInt16(field++, pInfo->m_maxPlayers);
			pInfo->m_serverStatus = (ServerStatus)dbCommand2->FetchByte(field++);

			pTab->m_serverInfoArr.push_back(pInfo);
		} while (dbCommand2->MoveNext());
		out.push_back(pTab);
	} while (dbCommand->MoveNext());
	return true;
}

void CDBProcess::UpdateServerInfo(ServerInfoList pVec)
{
	auto_ptr<OdbcCommand> dbCommand(m_dbConnection.CreateCommand());

	if (dbCommand.get() == NULL)
		return;

	foreach(itr, pVec)
	{
		if (!dbCommand->Execute(_T(string_format(SQL_SEL_UPDATE_SERVERINFO, (*itr)->m_tabId))))
			return;

		if (!dbCommand->hasData())
			return;

		foreach(server, (*itr)->m_serverInfoArr)
		{
			_SERVER_INFO* pServer = (*server);
			pServer->m_curPlayers = dbCommand->FetchUInt16(1);
			pServer->m_serverStatus = (ServerStatus)dbCommand->FetchByte(2);
		}
	}
}

bool CDBProcess::UpdateAccountSession(LoginSession* pSess)
{
	auto_ptr<OdbcCommand> dbCommand(m_dbConnection.CreateCommand());

	if (dbCommand.get() == NULL)
		return false;
	std::string ip = pSess->GetRemoteIP();
	dbCommand->AddParameter(SQL_PARAM_INPUT, ip.c_str(), ip.length());
	dbCommand->AddParameter(SQL_PARAM_INPUT, pSess->m_accountId.c_str(), pSess->m_accountId.length());

	if (!dbCommand->Execute(_T(string_format(SQL_UPD_ACCOUNT_SESSION, pSess->m_serverTab, pSess->m_serverId))))
		return false;
	return true;
}
