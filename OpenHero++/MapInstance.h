#ifndef MAPINSTANCE_H
#define MAPINSTANCE_H
#include "..\SharedFiles\Timer.h"

class MapInstance
{
public:
	MapInstance(MAP* pMap);
	~MapInstance();

	FastMutex m_regionMutex;

	void UpdateRegions(uint32 diff);
	void CheckForExpiredItems(uint32 diff, float rX, float rZ);

	std::list<CNpc*> SpawnNpc(_NPC_DATA* pNpcData, _NPC_GROUP* pGroup);

	bool RegionNpcRemove(int rx, int rz, int nId);
	void RegionNpcAdd(int rx, int rz, int nId);
	bool RegionUserRemove(int rx, int rz, int uId);
	void RegionUserAdd(int rx, int rz, int uId);
	bool RegionItemRemove(_ZONE_ITEM* pZoneItem);
	bool RegionItemAdd(std::list<_ZONE_ITEM*> itemList);

	void RegionUserInfoToMe(CUser* pUser);
	void RegionNpcInfoToMe(CUser* pSender);
	void RegionItemInfoToMe(CUser* pSender);
	void SendRegionUserList( uint16 rx, uint16 rz, CUser* pSendUser);
	void SendRegionNpcList(uint16 rx, uint16 rz, CUser * pSendUser);
	void SendRegionItemList(uint16 rx, uint16 rz, CUser * pSendUser);

	__forceinline MAP* GetMap() { return m_mapTemplate; }
	__forceinline Region* GetRegion(int x, int z) 
	{ 
		if (x < 0 || z < 0 || x > GetMap()->GetXRegionMax() || z > GetMap()->GetZRegionMax())
			return NULL;
		return &m_region[x][z]; 
	}
	__forceinline CNpc* GetNpcPtr(uint16 id) { return m_npcArray.at(id); }
	std::list<Unit*> GetListOfNpcsHit(float x, float z, float aoe);
	std::map<uint16, CNpc*> GetNpcList() { return m_npcArray; }
	__forceinline int PositionToRegion(float pos) { return pos / VIEW_DIST; }

	//Searches the adjecent/visible regions(myX, myZ -1 to +1)
	_ZONE_ITEM* LootDroppedItemById(uint16 regionItemId, float rX, float rZ);

	void SendToAll(Packet& pkt);
	void SendToRegion(Packet& pkt, int x, int z, CUser* exceptUser);
	void SendToOldRegions(Packet* pkt, int oX, int oZ, int x, int z, CUser* pExceptUser = NULL);
	void SendToNewRegions(Packet* pkt, int nX, int nZ, int x, int z, CUser* pExceptUser = NULL);
	void SendToRegionUnit(Packet* pkt, int x, int z, CUser* exceptUser = NULL);

	void AddUser(CUser* pUser);
	void RemoveUser(CUser* pUser);

	//Remove all items on ground, npcs and players from old region
	void RegionExit(CUser* pUser, float oRX, float oRZ);

	__forceinline std::list<CUser*> GetUserList() { return m_userArray; }
	bool HasUsers() { return m_userArray.size() >= 1; }

private:
	MAP* m_mapTemplate;
	Region** m_region;
	uint16 m_uniqueDropId;
	uint16 m_npcIdCounter;

	std::map<uint16, CNpc*> m_npcArray;
	std::list<CUser*> m_userArray;
};
#endif