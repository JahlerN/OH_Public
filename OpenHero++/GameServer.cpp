#include "stdafx.h"
#include "GameServer.h"
#include <ctime>
#include "packets.h"
#include "..\Database\GameDatabase.h"
#include "..\Database\AccountDatabase.h"
//#include "ObjectMgr.h"

OHSocketMgr<CUser> GameServer::s_socketMgr;
CommandHandler* g_commandHandler;
CRITICAL_SECTION g_region_critical;

GameServer::GameServer()
{
}


GameServer::~GameServer()
{
	//TODO: Disconnect all players by hand.
	m_dbAgent.UpdateAllServerUserCount(0);

	delete g_commandHandler;

	GameDatabase.Close();
	AccountDatabase.Close();

	if (m_dbAgent.m_GameDB.isConnected())
		m_dbAgent.m_GameDB.Disconnect();

	if (m_dbAgent.m_AccountDB.isConnected())
		m_dbAgent.m_AccountDB.Disconnect();

	DeleteCriticalSection(&g_region_critical);
}

bool GameServer::Startup()
{
	InitializeCriticalSection(&g_region_critical);
	float startTime = GetFloatTime();
	m_lastTime = GetFloatTime();
	CIni ini("GameServerConf.ini");

	ConnectionInfo accConInfo;
	ConnectionInfo gameConInfo;

	ini.GetString("ODBC", "ACCOUNT_DBN", "ohdb", m_accountDBName, sizeof(m_accountDBName));
	ini.GetString("ODBC", "ACCOUNT_UID", "OpenHero", m_accountDBId, sizeof(m_accountDBId));
	ini.GetString("ODBC", "ACCOUNT_UPW", "root", m_accountDBPw, sizeof(m_accountDBPw));
	ini.GetString("ODBC", "GAME_DBN", "world", m_gameDBName, sizeof(m_gameDBName));
	ini.GetString("ODBC", "GAME_UID", "OpenHero", m_gameDBId, sizeof(m_gameDBId));
	ini.GetString("ODBC", "GAME_UPW", "root", m_gameDBPw, sizeof(m_gameDBPw));

	accConInfo.m_dbName = m_accountDBName;
	accConInfo.m_username = m_accountDBId;
	accConInfo.m_password = m_accountDBPw;

	gameConInfo.m_dbName = m_gameDBName;
	gameConInfo.m_username = m_gameDBId;
	gameConInfo.m_password = m_gameDBPw;

	AccountDatabase.Open(accConInfo);
	GameDatabase.Open(gameConInfo);
	
	if (!m_dbAgent.Connect())
		return false;

	//Just to be sure we reset user count.
	m_dbAgent.UpdateAllServerUserCount(0);

	std::vector<_SERVER_TAB*> m_servers = sObjMgr->LoadServerTables();
	if (m_servers.size() <= 0)
		return false;

	if (!sObjMgr->LoadItemSet() ||
		!sObjMgr->LoadItemTemplate() ||
		!sObjMgr->LoadItemDropTable() ||
		!sObjMgr->LoadGamblingItemTable() ||
		!sObjMgr->LoadMakeItemTable() ||
		!sObjMgr->LoadMakeItemFusionTable() ||
		!sObjMgr->LoadDismantleItemTable() ||
		!sObjMgr->LoadLevelData() ||
		!sObjMgr->LoadZoneChangeTable() ||
		!sObjMgr->LoadZoneStartPositionTable() ||
		!sObjMgr->LoadCharacterInfoTable() ||
		!sObjMgr->LoadNpcGossipTable() ||
		!sObjMgr->LoadGossipOptionTable() ||
		!sObjMgr->LoadNpcInfoTable() ||
		!sObjMgr->LoadNpcGroupTable() ||
		!sObjMgr->LoadSkillTable() ||
		!sObjMgr->LoadSkillBookTable() ||
		!sObjMgr->LoadShopTable() ||
		!sObjMgr->LoadShopItemTable())
		return false;
	
	if (!sObjMgr->LoadMapTables())
		return false;

	//With the fresh data, create all server instances.
	m_serverInstanceMgr = new ServerInstanceMgr(m_servers);

	m_serverInstanceMgr->ResumeAllServerAI();

	//Start the command handler.
	g_commandHandler = new CommandHandler();
	if (!g_commandHandler->InitCommands())
		return false;

	//Create timers.
	IntervalTimer savePlayers, updateInstanceManager;
	savePlayers.SetInterval(2500);
	updateInstanceManager.SetInterval(3000);
	m_timers.push_back(savePlayers);
	m_timers.push_back(updateInstanceManager);


	srand((unsigned)time(NULL));
	s_socketMgr.Listen(15001, 255);
	s_socketMgr.RunServer();

	printf("Time elapsed during startup in seconds: %.3f\n", GetFloatTime() - startTime);

	return true;
}

void GameServer::Update(uint32 diff)
{
	m_timers.at(WUpdate_Save_Players).Update(diff);
	if (m_timers.at(WUpdate_Save_Players).Passed())
	{
		SaveIngamePlayers();
		UpdateServerUserCount();
		m_timers.at(WUpdate_Save_Players).ResetTimer();
	}

	m_serverInstanceMgr->Update(diff);

	UpdateUserSessions(diff);
}

void GameServer::UpdateUserSessions(uint32 diff)
{
	m_characterNameLock.Acquire();
	auto itr = m_characterNameMap.begin();
	for (; itr != m_characterNameMap.end(); itr++)
	{
		if (itr->second->IsInGame())
			itr->second->Update(diff);
	}
	m_characterNameLock.Release();
}

bool GameServer::IsRunning()
{
	return m_isRunning;
}

CUser* GameServer::GetUserPtr(const char* userId, NameTypes t)
{
	CUser* res = NULL;
	std::string findName = userId;
	STRTOUPPER(findName);

	if (t == NAME_TYPE_ACCOUNT)
	{
		m_accountNameLock.Acquire();
		auto itr = m_accountNameMap.find(findName);
		if (itr != m_accountNameMap.end())
			res = itr->second;
		m_accountNameLock.Release();
	}
	else if (t == NAME_TYPE_CHARACTER)
	{
		m_characterNameLock.Acquire();
		auto itr = m_characterNameMap.find(findName);
		if (itr != m_characterNameMap.end())
			res = itr->second;
		m_characterNameLock.Release();
	}

	return res;
}

void GameServer::SendToAll(Packet& pkt)
{
	SessionMap& sessMap = s_socketMgr.GetActiveSessionMap();
	foreach(itr, sessMap)
	{
		CUser* pUser = (CUser*)itr->second;
		if (!pUser->IsInGame())
			continue;
		pUser->Send(&pkt);
	}
	s_socketMgr.ReleaseLock();
}

void GameServer::AddAccountName(CUser* pSession)
{
	std::string upperName = pSession->m_accountId;
	STRTOUPPER(upperName);
	m_accountNameLock.Acquire();
	m_accountNameMap[upperName] = pSession;
	m_accountNameLock.Release();
}

void GameServer::AddCharacterName(CUser* pSession)
{
	std::string upperName = pSession->m_userData->m_charId;
	STRTOUPPER(upperName);
	m_characterNameLock.Acquire();
	m_characterNameMap[upperName] = pSession;
	m_characterNameLock.Release();

}

void GameServer::RemoveSessions(CUser* pSession)
{
	std::string upperName = pSession->m_accountId;
	STRTOUPPER(upperName);
	m_accountNameLock.Acquire();
	m_accountNameMap.erase(upperName);
	m_accountNameLock.Release();

	if (pSession->IsInGame())
	{
		upperName = pSession->m_userData->m_charId;
		STRTOUPPER(upperName);
		m_characterNameLock.Acquire();
		m_characterNameMap.erase(upperName);
		m_characterNameLock.Release();
	}
}

void GameServer::PrintSQLError(OdbcError* pError)
{
	if (pError == NULL)
		return;

	string errorMessage = string_format(_T("ODBC error occured.\r\nSource: %s\r\nError: %s\r\nDesc: %s\n"), pError->Source.c_str(), pError->ExtendedErrorMessage.c_str(), pError->ErrorMessage.c_str());

	printf("%s", errorMessage.c_str());
	delete pError;
}

uint64 GameServer::GetExpReqByLevel(uint16 level)
{
	return sObjMgr->GetLevelData(level)->m_expReq;
}

uint32 GameServer::GetStatPointsByLevel(uint16 level)
{
	return sObjMgr->GetLevelData(level)->m_statPoint;
}

uint32 GameServer::GetElementPointsByLevel(uint16 level)
{
	return sObjMgr->GetLevelData(level)->m_statElement;
}

void GameServer::SaveIngamePlayers()
{
	m_characterNameLock.Acquire();
	auto itr = m_characterNameMap.begin();
	for (; itr != m_characterNameMap.end(); itr++)
	{
		_USER_DATA* pUser = (_USER_DATA*)itr->second->m_userData;
		if (pUser == NULL || itr->second->m_accountId.empty())
			continue;

		std::string accId = itr->second->m_accountId;
		std::string charId = pUser->m_charId;
		m_dbAgent.SaveUserInventory(pUser);
		m_dbAgent.SaveUserWarehouse(pUser, itr->second->m_accountId);
		m_dbAgent.SaveUser(pUser);
	}
	//printf("\nSaved %d players.\n", m_characterNameMap.size());
	m_characterNameLock.Release();
}

void GameServer::UpdateServerUserCount()
{
	auto tabVec = m_serverInstanceMgr->GetServerTabVec();
	for (auto itr = tabVec.begin(); itr != tabVec.end(); itr++)
	{
		auto serverVec = (*itr)->m_serverInfoArr;
		for (auto itr2 = serverVec.begin(); itr2 != serverVec.end(); itr2++)
		{
			m_dbAgent.UpdateServerUserCount((*itr2)->m_tabId, (*itr2)->m_serverId, (*itr2)->m_curPlayers);
		}
	}
}
