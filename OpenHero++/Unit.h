#ifndef UNIT_H
#define UNIT_H
//#include "MAP.h"

//class MapInstance;
//TODO: Get server in here, we need to be able to get the units server for some functions.
class Unit
{
public:
	Unit(bool player = false);
	virtual void Initialize();
	virtual uint16 GetID() = 0;
	virtual void GetInOut(Packet& result, uint8 inOutType) = 0;

	__forceinline bool IsPlayer() { return m_player; };
	__forceinline bool IsNpc() { return !IsPlayer(); }

	virtual bool IsDead() = 0;

	virtual void HpChange(int amount, Unit* pAttacker = NULL, uint32 skillId = 0) = 0;
	virtual void ChiChange(int amount) = 0;
	virtual void OnDeath(Unit* pKiller);

	virtual float GetX() = 0;
	virtual float GetZ() = 0;
	virtual float GetY() = 0;

	//SERVER_INSTANCE_CHANGE
	__forceinline MapInstance* GetMapInstance() { return m_curMapInstance; }
	//__forceinline MAP* GetMap() { return GetMapInstance()->GetMap(); }
	//__forceinline MAP* GetMap() { return m_map; }
	//void SetCurrentMap(MAP* map) { m_map = map; }
	void SetCurrentMapInstance(MapInstance* mapInstance) { m_curMapInstance = mapInstance; }
	__forceinline Region* GetRegion() { return m_region; }

	bool RegisterRegion();

	uint16 GetRegionX() { return m_regionX; }
	uint16 GetRegionZ() { return m_regionZ; }

	uint16 GetNewRegionX() { return (uint16)(GetX()) / VIEW_DIST; }
	uint16 GetNewRegionZ() { return (uint16)(GetZ()) / VIEW_DIST; }

	__forceinline void SetRegion(uint16 x = -1, uint16 z = -1) 
	{ 
		m_regionX = x;
		m_regionZ = z;
		//SERVER_INSTANCE_CHANGE
		m_region = GetMapInstance()->GetRegion(x, z);
		//m_region = m_map->GetRegion(x, z);
	}

	void RemoveRegion(int x, int z);
	void InsertRegion(int x, int z);

	//SERVER_INSTANCE_CHANGE
	void SendToRegion(Packet& pkt, CUser* pExceptUser = NULL);
	void SendToMapInstance(Packet& pkt);

	virtual uint32 GetDamage(Unit* pTarget) = 0;//TODO: Add skills here

	bool m_player;
	//MAP* m_map;
	MapInstance* m_curMapInstance;
	Region* m_region;

	uint16 m_regionX, m_regionZ;
};
#endif