#pragma once
#include "../SharedFiles/OHSocketMgr.h"
#include "../SharedFiles/ClientSocketMgr.h"
#include "User.h"
#include "NpcThread.h"
#include "..\SharedFiles\Timer.h"
//#include "../Database/MySQLConnection.h"
//#include "MAP.h"

#include <unordered_map>

//class CUser;
typedef std::unordered_map<std::string, CUser*> NameMap;
typedef std::map<uint8, std::map<uint16, CNpc*>> NpcArray;

class GameServer
{
public:
	GameServer();
	~GameServer();

	NameMap m_accountNameMap, m_characterNameMap;
	FastMutex m_accountNameLock, m_characterNameLock;

	bool Startup();

	void Update(uint32 diff);
	void UpdateUserSessions(uint32 diff);
	bool IsRunning();

	void AddAccountName(CUser* pSession);
	void AddCharacterName(CUser* pSesson);

	CUser* GetUserPtr(const char* userId, NameTypes t);
	__forceinline CUser* GetUserPtr(int sId) { return s_socketMgr[sId]; }
	//SERVER_INSTANCE_CHANGE
	//void RegionUserInfoToMe(CUser* pUser);
	//void RegionNpcInfoToMe(CUser* pSender);
	//void GetRegionUserList(MAP* map, uint16 rx, uint16 rz, CUser* pSendUser);//, Packet& result);
	//void SendRegionUserList(MAP* map, uint16 rx, uint16 rz, CUser* pSendUser);
	//void SendRegionNpcList(MAP* map, uint16 rx, uint16 rz, CUser* pSendUser);
	//MAP* GetZoneById(uint8 zoneId);

	//Used when logout;
	void RemoveSessions(CUser* pSession);

	void PrintSQLError(OdbcError* pError);

	bool AddObjectEventNpc(_OBJECT_EVENT* pEvent, int zoneNum);

	static OHSocketMgr<CUser> s_socketMgr; //TODO: Do i integrate the login server aswell?

	
	void SendToAll(Packet& pkt);
	//SERVER_INSTANCE_CHANGE
	/*void SendToRegion(Packet& pkt, MAP* map, int x, int z, CUser* exceptUser);
	void SendToOldRegions(Packet* pkt, int oX, int oZ, MAP* pMap, int x, int z, CUser* pExceptUser = NULL);
	void SendToNewRegions(Packet* pkt, int nX, int nZ, MAP* pMap, int x, int z, CUser* pExceptUser = NULL);
	void SendToRegionUnit(Packet* pkt, MAP* map, int x, int z, CUser* exceptUser = NULL);
	bool RegionIntersect(int oX, int oZ, int rPosX, int rPosZ);*/

	char m_gameDBName[32], m_accountDBName[32];
	char m_gameDBId[32], m_accountDBId[32];
	char m_gameDBPw[32], m_accountDBPw[32];

	CDBAgent m_dbAgent;

	__forceinline ServerInstanceMgr* GetServerInstanceMgr() { return m_serverInstanceMgr; }

	uint64 GetExpReqByLevel(uint16 level);
	uint32 GetStatPointsByLevel(uint16 level);
	uint32 GetElementPointsByLevel(uint16 level);

	//_CHARACTER_DATA* GetCharacterDataByType(uint8 type);
	__forceinline _SKILL_BOOK_DATA* GetSkillBookData(uint32 bookId) { return m_skillBookTableArray.GetData(bookId); }
	__forceinline _SKILL_DATA* GetSkillDataById(uint32 skillId) { return m_skillTableArray.GetData(skillId); }
	__forceinline _ZONESTART_POSITION* GetZoneStartPosition(uint8 zoneId) { return m_zoneStartTableArray.GetData(zoneId); }
	__forceinline _NPC_DATA* GetNpcTemplateById(uint32 npcId) { return m_npcInfoArray.GetData(npcId); }

	__forceinline NpcTableArray GetNpcTableArray() { return m_npcTableArray; }
	//__forceinline ZoneTableArray GetZoneTableArray() { return m_zoneTableArray; }

	__forceinline std::map<uint16, CNpc*> GetNpcListForServer(uint8 serverId)
	{
		auto npcs = m_npcArray.find(serverId);
		if (npcs != m_npcArray.end())
		{
			return npcs->second;
		}
		return std::map<uint16, CNpc*>();
	}

	//TODO: Move this to like, ItemDropFactory
	__forceinline std::list<_ITEM_DATA*> GenerateItemDropList(uint32 dropId, uint8 lootRolls)
	{
		_ITEM_DROP_TABLE* pDropTable = sObjMgr->GetItemDropTable(dropId);
		std::list<_ITEM_DATA*> dropList;
		if (pDropTable == NULL)
			return dropList;

		std::random_device r;
		std::seed_seq seed { r(), r(), r(), r(), r(), r(), r(), r() };
		std::mt19937 rng(seed);

		//TODO: Randomize 
		for (int i = 0; i < lootRolls; i++)
		{
			_ITEM_DATA* pItem = GenerateItem(rng, pDropTable);
			if (pItem != NULL)
				dropList.push_back(pItem);
		}
		return dropList;
	}

	__forceinline _ITEM_DATA* GenerateItem(std::mt19937& rng, _ITEM_DROP_TABLE* pDropitem)
	{
		std::uniform_int_distribution<> gen(0, 1000);
		uint16 roll = gen(rng);

		if (roll > pDropitem->m_maxRoll)
			return NULL;

		for (int i = 0; i < NUM_DROPS_IN_DROP_TABLE; i++)
		{
			if (roll > pDropitem->m_dropableChance[i])
				continue;

			_ITEM_DROP_TABLE* pDrop = sObjMgr->GetItemDropTable(pDropitem->m_dropableId[i]);
			//If it doesn't point to another drop table, generate the item.
			if (pDrop == NULL)
			{
				_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(pDropitem->m_dropableId[i]);
				if (pTable == NULL)
				{
					printf("Tried to generate item drop, but the item table was null.");
					return NULL;
				}

				//TODO: Check if the player has active quest, set a flag to add instantly or just add it here and move on.
				if (pTable->m_itemType == 202)//Quest item
					return NULL;

				//TODO: Randomize if it should be upgraded or not, also stats.
				_ITEM_DATA* pItem = new _ITEM_DATA();
				pItem->itemId = pTable->m_itemId;
				pItem->count = 1;

				return pItem;
			}
			else
			{
				return GenerateItem(rng, pDrop);
			}
		}
	}

	void SaveIngamePlayers();
	void UpdateServerUserCount();

	//TODO: Temporary while we only handle 1 server!
	uint32 m_npcCount;

private:
	float m_lastTime;
	bool m_isRunning = true;
	short m_totalMap = 0;

	ServerInstanceMgr* m_serverInstanceMgr;

	//ZoneTableArray m_zoneTableArray;

	//TODO: When i have time, i'd like to place these in their own classes, i don't think it's reasonable to keep them all in the main server class.
	//Considering all functions to grab values from them.


	//ZoneChangeTableArray m_zoneChangeTableArray;
	CharacterInfoArray m_characterInfoArray;
	NpcInfoArray m_npcInfoArray;
	NpcTableArray m_npcTableArray;
	SkillTableArray m_skillTableArray;
	SkillBookTableArray m_skillBookTableArray;
	ZoneStartTableArray m_zoneStartTableArray;

	//Get npcs per server phase
	NpcArray m_npcArray;

	CNpcThreadMgr* m_npcThreadMgr;

	std::vector<IntervalTimer> m_timers;

	MySQLConnection* m_mySqlCon;
};

extern GameServer* g_main;