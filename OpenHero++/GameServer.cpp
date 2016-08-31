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
	m_npcThreadMgr->ShutdownThreads();

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
	m_characterInfoArray.DeleteAllData();
	m_npcInfoArray.DeleteAllData();
	m_npcTableArray.DeleteAllData();
	m_skillBookTableArray.DeleteAllData();
	m_skillTableArray.DeleteAllData();
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
		!sObjMgr->LoadLevelData() ||
		!sObjMgr->LoadZoneChangeTable() ||
		!sObjMgr->LoadCharacterInfoTable())
		return false;

	if (//!m_dbAgent.LoadCharacterInfoTable(m_characterInfoArray) ||
		!m_dbAgent.LoadNpcInfoTable(m_npcInfoArray) ||
		!m_dbAgent.LoadNpcTable(m_npcTableArray) ||
		//THIS TABLE HAS TO BE LOADED BEFORE BOOK DATA
		!m_dbAgent.LoadSkillTable(m_skillTableArray) ||
		!m_dbAgent.LoadSkillBookTable(m_skillBookTableArray) ||
		!m_dbAgent.LoadZoneStartTable(m_zoneStartTableArray))
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
//
//bool GameServer::LoadMaps()
//{
//	uint8 totalMap;
//	if (!m_dbAgent.LoadMapTable(m_zoneTableArray, totalMap))
//		return false;
//
//	return true;
//}

//bool GameServer::InitializeNpcs()
//{
//	//TODO: Npcs and players can't have same unique id. fix that players 0-20000? 45k npcs per server! should be enough i guess ..
//	//TODO: Add check for max npc count, implement muiltiplier to npc count in group, only affect those with more than 1(else we duplicate npcs lol)
//	//Also find npc tag, so we can skip them and actualy send monster types too.
//	//Note: For each server we can reset the unique id. So each server has a max of 0xFFFF - player band npcs
//	uint16 uniqueId = NPC_UNIQUEID_START;
//	uint32 totalNpcs = 0;
//	uint8 serverPhase = 0;
//	float start = GetFloatTime();
//
//	//For each server
//	std::map<uint16, CNpc*> inner;
//	
//	foreach_stlmap(itr, m_npcTableArray)
//	{
//		_NPC_GROUP* pNpcGroup = itr->second;
//		if (pNpcGroup == NULL)
//			continue;
//
//		_NPC_DATA* pNpcData = m_npcInfoArray.GetData(pNpcGroup->m_npcId);
//		if (pNpcData == NULL)
//			continue;
//
//		for (int i = 0; i < pNpcGroup->m_npcsInGroup; i++)
//		{
//			CNpc* pNpc = new CNpc(pNpcData, pNpcGroup);
//			pNpc->m_uniqueId = uniqueId++;
//
//			if (pNpc->GetMap() == NULL)
//				return false;
//			pNpc->GetMap()->RegionNpcAdd(pNpc->GetRegionX(), pNpc->GetRegionZ(), pNpc->GetID());
//
//			inner.insert(make_pair(pNpc->m_uniqueId, pNpc));
//			totalNpcs++;
//		}
//	}
//	m_npcArray.insert(make_pair(serverPhase, inner));
//
//	printf("\nSpawned %d Npcs over %d server phases in %.3f second(s).", totalNpcs, m_npcArray.size(), GetFloatTime() - start);
//	m_npcCount = totalNpcs;
//	return true;
//}

//bool GameServer::InitializeNpcThreading()
//{
//	m_npcThreadMgr = new CNpcThreadMgr();
//
//	m_npcThreadMgr->ResumeAi();
//
//	return true;
//}

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

//SERVER_INSTANCE_CHANGE
//void GameServer::RegionUserInfoToMe(CUser* pUser)
//{
//	if (pUser == NULL)
//		return;
//
//	//Packet result(0x21, uint8(1));
//	MAP* pMap = pUser->GetMap();
//	ASSERT(pMap != NULL);
//
//	uint16 rx = pUser->GetRegionX(), rz = pUser->GetRegionZ();
//	foreach_region(x, z)
//		SendRegionUserList(pMap, rx + x, rz + z, pUser);
//}
//
//void GameServer::RegionNpcInfoToMe(CUser * pSender)
//{
//	if (pSender == NULL)
//		return;
//
//	if (pSender->GetMap() == NULL)
//		return;
//
//	uint16 rx = pSender->GetRegionX(), rz = pSender->GetRegionZ();
//	foreach_region(x, z)
//		SendRegionNpcList(pSender->GetMap(), rx + x, rz + z, pSender);
//}
//
//void GameServer::SendRegionUserList(MAP* pMap, uint16 rx, uint16 rz, CUser* pSendUser)//, Packet& result)
//{
//	if (pMap == NULL || rx < 0 || rz < 0 || rx > pMap->GetXRegionMax() || rz > pMap->GetZRegionMax())
//		return;
//
//	EnterCriticalSection(&g_region_critical);
//
//	Region* pRegion = pMap->GetRegion(rx, rz);
//	Packet result;
//	foreach(itr, pRegion->m_regionUserMap.m_UserTypeMap)
//	{
//		result.Initialize(PKT_GAMESERVER_REGION_CHANGE_USER, uint8(0x01));
//		CUser* pUser = GetUserPtr(itr->first);
//		//TODO: Only send to user IF they haven't already been sent(meaning they're already in the region.)
//		if (pUser == NULL || !pUser->IsInGame() || pUser->GetRegionX() != rx || pUser->GetRegionZ() != rz)
//			continue;
//
//		//result << pUser->GetID();
//		result.append(pUser->GetUserInfoForRegion());
//		pSendUser->Send(&result);
//	}
//
//	LeaveCriticalSection(&g_region_critical);
//}
//
//void GameServer::SendRegionNpcList(MAP * pMap, uint16 rx, uint16 rz, CUser * pSendUser)
//{
//	if (pMap == NULL || rx < 0 || rz < 0 || rx > pMap->GetXRegionMax() || rz > pMap->GetZRegionMax())
//		return;
//
//	EnterCriticalSection(&g_region_critical);
//
//	Region* pRegion = pMap->GetRegion(rx, rz);
//	Packet result;
//	foreach(itr, pRegion->m_regionNpcMap.m_UserTypeMap)
//	{
//		result.Initialize(PKT_GAMESERVER_REGION_CHANGE_NPC, uint8(0x01));
//		CNpc* pNpc = GetNpcPtr(0, itr->first);
//		if (pNpc == NULL)// || pNpc->IsDead())//TODO: Fix this, we need to check if it's a non combat npc and skip alive check, or add a state
//			continue;
//
//		pNpc->GetNpcInfo(result);
//		pSendUser->Send(&result);
//	}
//	LeaveCriticalSection(&g_region_critical);
//}

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
//SERVER_INSTANCE_CHANGE
//
//void GameServer::SendToRegion(Packet& pkt, MAP* map, int x, int z, CUser* exceptUser = NULL)
//{
//	foreach_region(rx, rz)
//		SendToRegionUnit(&pkt, map, rx + x, rz + z, exceptUser);
//}
//
//bool GameServer::RegionIntersect(int nX, int nZ, int rPosX, int rPosZ)
//{
//	for (int i = -1; i <= 1; i++)
//	{
//		for (int x = -1; x <= 1; x++)
//		{
//			if (rPosX == nX + i && rPosZ == nZ + x)
//				return true;
//		}
//	}
//	return false;
//}
//
//void GameServer::SendToOldRegions(Packet* pkt, int oX, int oZ, MAP* pMap, int x, int z, CUser* pExceptUser /*= NULL*/)
//{
//	//for (int i = -1; i <= 1; i++)
//	//{
//	//	for (int q = -1; q <= 1; q++)
//	//	{
//	//		if (RegionIntersect(oX, oZ, x + i, z + q) == old) //SWitch for old or new(true or false)
//	//			SendToRegionUnit(pkt, pMap, x + i, z + q);
//	//	}
//	//}
//	if (oX != 0)
//	{
//		SendToRegionUnit(pkt, pMap, x + oX * 2, z + oZ - 1);
//		SendToRegionUnit(pkt, pMap, x + oX * 2, z + oZ);
//		SendToRegionUnit(pkt, pMap, x + oX * 2, z + oZ + 1);
//	}
//
//	if (oZ != 0)
//	{
//		SendToRegionUnit(pkt, pMap, x + oX, z + oZ * 2);
//		if (oX < 0)
//			SendToRegionUnit(pkt, pMap, x + oX + 1, z + oZ * 2);
//		else if (oX > 0)
//			SendToRegionUnit(pkt, pMap, x + oX - 1, z + oZ * 2);
//		else
//		{
//			SendToRegionUnit(pkt, pMap, x + oX - 1, z + oZ * 2);
//			SendToRegionUnit(pkt, pMap, x + oX + 1, z + oZ * 2);
//		}
//	}
//}
//
//void GameServer::SendToNewRegions(Packet* pkt, int nX, int nZ, MAP* pMap, int x, int z, CUser* pExceptUser /*= NULL*/)
//{
//	if (nX != 0)
//	{
//		SendToRegionUnit(pkt, pMap, x + nX, z - 1);
//		SendToRegionUnit(pkt, pMap, x + nX, z);
//		SendToRegionUnit(pkt, pMap, x + nX, z + 1);
//	}
//
//	if (nZ != 0)
//	{
//		SendToRegionUnit(pkt, pMap, x, z + nZ);
//
//		if (nX < 0)
//			SendToRegionUnit(pkt, pMap, x + 1, z + nZ);
//		else if (nX > 0)
//			SendToRegionUnit(pkt, pMap, x - 1, z + nZ);
//		else
//		{
//			SendToRegionUnit(pkt, pMap, x - 1, z + nZ);
//			SendToRegionUnit(pkt, pMap, x + 1, z + nZ);
//		}
//	}
//}
//
//void GameServer::SendToRegionUnit(Packet* pkt, MAP* map, int x, int z, CUser* exceptUser /* = NULL*/)
//{
//	if (map == NULL || x < 0 || z < 0 || x > map->GetXRegionMax() || z > map->GetZRegionMax())
//		return;
//
//	EnterCriticalSection(&g_region_critical);
//	Region* pRegion = map->GetRegion(x, z);
//
//	foreach(itr, pRegion->m_regionUserMap.m_UserTypeMap)
//	{
//		CUser* pUser = GetUserPtr(itr->first);
//		if (pUser == NULL || pUser == exceptUser || !pUser->IsInGame())
//			continue;
//
//		pUser->Send(pkt);
//	}
//	LeaveCriticalSection(&g_region_critical);
//}

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
//
//_CHARACTER_DATA* GameServer::GetCharacterDataByType(uint8 type)
//{
//	return m_characterInfoArray.GetData(type);
//}

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
