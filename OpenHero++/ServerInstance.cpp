#include "stdafx.h"

CRITICAL_SECTION g_server_instance_critical;

ServerInstanceMgr::ServerInstanceMgr(std::vector<_SERVER_TAB*> serverTabVec)
{
	InitializeCriticalSection(&g_server_instance_critical);
	float startTime = GetFloatTime();
	printf("\n\nStarting server setup for %d tabs and %d servers.", serverTabVec.size(), serverTabVec.at(0)->m_serverInfoArr.size());
	m_serverTabVec = serverTabVec;
	m_npcThreadMgr = new CNpcThreadMgr();

	CreateServerInstances();

	printf("\nServer setup finished in %.2f second(s)\n", GetFloatTime() - startTime);
}

ServerInstanceMgr::~ServerInstanceMgr()
{
	m_npcThreadMgr->ShutdownThreads();

	foreach(itr, m_serverInstances)
		foreach(inner, (*itr))
		{
			delete (*inner);
		}

	DeleteCriticalSection(&g_server_instance_critical);
}

void ServerInstanceMgr::Update(uint32 diff)
{
	//Update all server instances
	for (int i = 0; i < m_serverInstances.size(); i++)
	{
		foreach(itr, m_serverInstances.at(i))
		{
			(*itr)->Update(diff);
		}
	}
}

//TODO: Make this fucntion return false incase ANY of the steps would fail.
bool ServerInstanceMgr::CreateServerInstances()
{
	auto zoneArr = sObjMgr->GetMapArray();
	if (zoneArr.size() == 0)
		return false;

	for (auto itr = m_serverTabVec.begin(); itr != m_serverTabVec.end(); itr++)
	{
		_SERVER_TAB* pTab = (*itr);
		std::vector<ServerInstance*> tempVecInner;

		for (auto itr = pTab->m_serverInfoArr.begin(); itr != pTab->m_serverInfoArr.end(); itr++)
		{
			_SERVER_INFO* pInfo = (*itr);

			ServerInstance* pInstance = new ServerInstance(pInfo, zoneArr);

			m_npcThreadMgr->CreateAndAssignNpcThreads(pInstance->m_mapInstances);

			/*
			uint32 npcsInThread = m_npcThreadMgr->CreateAndAssignNpcThreads(&pInstance->m_allNpcList);
			printf("\nCreated %d NPCThreads for %d npcs on server tab %d server ID %d.", THREADS_PER_SERVER, npcsInThread, pInstance->m_serverInfo->m_tabId, pInstance->m_serverInfo->m_serverId);
			*/
			tempVecInner.push_back(pInstance);
		}

		m_serverInstances.push_back(tempVecInner);
	}

	return true;
}

bool ServerInstanceMgr::AddUserToServerInstance(CUser * pUser, uint8 tabId, uint8 serverId)
{
	if (pUser == NULL)
		return false;

	std::vector<ServerInstance*> tempInner;

	EnterCriticalSection(&g_server_instance_critical);

	if (m_serverInstances.size() > tabId + 1)
	{
		printf("Tried to access a non existing server tab.");
		ASSERT(0);
	}

	tempInner = m_serverInstances.at(tabId);

	foreach(itr, tempInner)
	{
		ServerInstance* pServerInstance = (*itr);

		if (pServerInstance->m_serverInfo->m_serverId == serverId)
		{
			pServerInstance->AddUser(pUser);
			break;
		}
	}

	LeaveCriticalSection(&g_server_instance_critical);
	
	return false;
}

bool ServerInstanceMgr::MoveUserToServerInstance(CUser * pUser, uint8 tabId, uint8 serverId)
{	
	if (pUser == NULL)
		return false;
}

void ServerInstanceMgr::RemoveUserFromServerInstance(CUser * pUser, uint8 tabId, uint8 serverId)
{
	if (pUser == NULL)
		return;

	std::vector<ServerInstance*> tempInner;

	EnterCriticalSection(&g_server_instance_critical);

	if (tabId + 1 > m_serverInstances.size())
	{
		printf("Tried to access a non existing server tab.");
		ASSERT(0);
	}

	tempInner = m_serverInstances.at(tabId);

	foreach(itr, tempInner)
	{
		ServerInstance* pServerInstance = (*itr);

		if (pServerInstance->m_serverInfo->m_serverId == serverId)
		{
			pServerInstance->RemoveUser(pUser);
			break;
		}
	}

	LeaveCriticalSection(&g_server_instance_critical);
}

void ServerInstanceMgr::SendToAllCrossServer(Packet & pkt)
{
	g_main->SendToAll(pkt);
}


/////////////////////////
//SERVER INSTANCE START//
/////////////////////////

ServerInstance::ServerInstance(_SERVER_INFO* pServerInfo, std::map<long, MAP*> zoneArray)
{
	m_serverInfo = pServerInfo;
	m_totalNpcs = 0;

	foreach(itr, zoneArray)
	{
		MAP* pMap = itr->second;
		MapInstance* mapInstance = new MapInstance(pMap);

		m_mapInstances.insert(make_pair(pMap->m_zoneNum, mapInstance));
	}

	auto pNpcTable = g_main->GetNpcTableArray().m_UserTypeMap;

	foreach(itr, pNpcTable)
	{
		_NPC_GROUP* pGroup = itr->second;

		if (pGroup == NULL)
			continue;

		_NPC_DATA* pNpcData = g_main->GetNpcTemplateById(pGroup->m_npcId);
		if (pNpcData == NULL)
			continue;

		//Spawn npc in world and add it to the combined list.
		std::list<CNpc*> tempList = m_mapInstances.at(itr->second->m_zoneId)->SpawnNpc(pNpcData, pGroup);
		foreach(itr, tempList)
			m_allNpcList.push_back((*itr));

		m_totalNpcs += pGroup->m_npcsInGroup;
	}
}

ServerInstance::~ServerInstance()
{

}

void ServerInstance::Update(uint32 diff)
{
	foreach(itr, m_mapInstances)
	{
		(*itr).second->UpdateRegions(diff);
	}
}

void ServerInstance::AddUser(CUser * pUser)
{
	if (pUser == NULL)
		return;
	m_userMapLock.Acquire();
	m_userMap.insert(make_pair(pUser->GetID(), pUser));
	AddToUserCounter();
	pUser->SetServerInstance(this);
	m_userMapLock.Release();
}

void ServerInstance::RemoveUser(CUser * pUser)
{
	if (pUser == NULL)
		return;

	m_userMapLock.Acquire();
	m_userMap.erase(pUser->GetID());
	RemoveFromUserCounter();
	pUser->SetServerInstance(NULL);
	m_userMapLock.Release();
}

bool ServerInstance::AddUserToMapInstance(CUser * pUser)
{
	if (pUser == NULL)
		return false;

	MapInstance* mapInstance = GetMapInstanceByZoneId(pUser->m_userData->m_zone);

	if (mapInstance == NULL)
		return false;

	mapInstance->AddUser(pUser);
}

bool ServerInstance::RemoveUserFromMapInstance(CUser * pUser)
{
	if (pUser == NULL)
		return false;

}

void ServerInstance::SendToAll(Packet & pkt)
{
	m_userMapLock.Acquire();
	foreach(itr, m_userMap)
	{
		CUser* pUser = (*itr).second;
		if (pUser == NULL || !pUser->IsInGame())
			continue;
		pUser->Send(&pkt);
	}
	m_userMapLock.Release();
}
