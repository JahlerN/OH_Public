#ifndef SERVERINSTANCE_H
#define SERVERINSTANCE_H

class ServerInstance
{
public:
	ServerInstance(_SERVER_INFO* pInfo, std::map<long, MAP*> zoneArr);
	~ServerInstance();

	FastMutex m_userMapLock;

	void Update(uint32 diff);

	__forceinline std::string GetServerName() { return m_serverInfo->m_serverName; }
	__forceinline void AddToUserCounter() { m_serverInfo->m_curPlayers++; }
	__forceinline void RemoveFromUserCounter() { m_serverInfo->m_curPlayers--; }

	__forceinline uint16 GetTotalNpcs() { return m_totalNpcs; }
	__forceinline MapInstance* GetMapInstanceByZoneId(uint8 zoneId) { return m_mapInstances.find(zoneId) != m_mapInstances.end() ? m_mapInstances.at(zoneId) : NULL; }

	void AddUser(CUser* pUser);
	void RemoveUser(CUser* pUser);

	bool AddUserToMapInstance(CUser* pUser);
	bool RemoveUserFromMapInstance(CUser* pUser);

	//KEY = zoneId, VALUE = MapInstance.
	std::map<uint8, MapInstance*> m_mapInstances;

	void SendToAll(Packet& pkt);

	_SERVER_INFO* m_serverInfo;
	std::map<uint16, CUser*> m_userMap;
	std::list<CNpc*> m_allNpcList;
private:
	uint16 m_totalNpcs;//Redundant due to allNpcList? Can just return size of it
};

class ServerInstanceMgr
{
public:
	ServerInstanceMgr(std::vector<_SERVER_TAB*> serverTabVec);
	~ServerInstanceMgr();

	void Update(uint32 diff);

	std::vector<_SERVER_TAB*> GetServerTabVec() { return m_serverTabVec; }

	//__forceinline void AddToPlayerCountOnServer(uint8 tabId, uint8 serverId) { GetServerInfoByTabAndServerId(tabId, serverId)->m_curPlayers++; }
	//__forceinline void RemovePlayerCountOnServer(uint8 tabId, uint8 serverId) { GetServerInfoByTabAndServerId(tabId, serverId)->m_curPlayers--; }

	__forceinline _SERVER_INFO* GetServerInfoByTabAndServerId(uint8 tab, uint8 serverId)
	{
		std::vector<_SERVER_INFO*> vec = GetServerInfoListByTab(tab);

		std::vector<_SERVER_INFO*>::iterator it = std::next(vec.begin(), serverId);

		if (it == vec.end())
			return nullptr;

		return (*it);
	}

	__forceinline std::vector<_SERVER_INFO*> GetServerInfoListByTab(uint8 tabId)
	{ 
		std::vector<_SERVER_INFO*> temp;

		if (tabId > m_serverTabVec.size())
			return temp;

		_SERVER_TAB* tab = m_serverTabVec.at(tabId);

		return tab->m_serverInfoArr;

	}

	__forceinline std::string GetTabNameByTabId(uint8 tabId) 
	{
		if (tabId > m_serverTabVec.size())
			return "";

		_SERVER_TAB* tab = m_serverTabVec.at(tabId);

		return tab->m_tabName;
	}

	__forceinline void PauseAllServerAI() { m_npcThreadMgr->PauseAi(); }
	__forceinline void ResumeAllServerAI() { m_npcThreadMgr->ResumeAi(); }

	bool CreateServerInstances();

	bool AddUserToServerInstance(CUser* pUser, uint8 tabId, uint8 serverId);
	bool MoveUserToServerInstance(CUser* pUser, uint8 tabId, uint8 serverId);
	void RemoveUserFromServerInstance(CUser* pUser, uint8 tabId, uint8 serverId);

	void SendToAllCrossServer(Packet& pkt);

private:
	std::vector<_SERVER_TAB*> m_serverTabVec;
	//first array is tab, 2nd is instances
	std::vector<std::vector<ServerInstance*>> m_serverInstances;
	CNpcThreadMgr* m_npcThreadMgr;
};
#endif