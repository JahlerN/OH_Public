#ifndef REGION_H
#define REGION_H
#include <set>
#include "../SharedFiles/STLMap.h"
#include "../SharedFiles/types.h"
#include "..\SharedFiles\Timer.h"
#include "..\SharedFiles\globals.h"

#define ZONEITEM_MAX 2100000000

#define ITEM_DROP_EXPIRATION_TIME 40000
#define ITEM_DROP_RESERVATION_TIME ITEM_DROP_EXPIRATION_TIME / 2 //After half the expiration time, anyone can loot it.

struct _ZONE_ITEM
{
	uint32 m_regionItemId;
	_ITEM_DATA* m_pItem;
	uint16 m_itemCount;
	float m_x;
	float m_z;
	float m_y;
	TimeTracker m_dropTracker;
	TimeTracker m_expirationTracker;
	bool m_itemDeleted;

	_ZONE_ITEM()
	{
		m_regionItemId = 0;
		m_pItem = NULL;
		m_itemCount = 1;
		m_x = 0.0f;
		m_z = 0.0f;
		m_y = 0.0f;
		m_itemDeleted = false;
	}

	uint16 m_reservedForuserId;

	//TODO: Fix the object mgr so we don't have to send rarity in, dumb.
	__forceinline ByteBuffer GetRegionAddInfo(uint8 rarity)
	{
		ByteBuffer result;
		result << uint8(2);//Add to region
		result << uint16(m_regionItemId); //Item drop id, 0 doesn't seem to work.
		result << uint8(1) //0 idk seems to be nothing, 1 item, 2 gold
			<< uint8(1);//Idk
		result << m_x << m_y << m_z;
		result << uint32(m_pItem->itemId);//Master drac knife.
		result << uint8(0) << uint8(rarity);//Item rarity.
		result << uint16(m_itemCount);//count.
		result << uint32(0) << uint16(0) << m_reservedForuserId << uint16(0);
		return result;
	}

	__forceinline ByteBuffer GetRegionReservationTimeExpired()
	{
		ByteBuffer result;
		result << uint8(3);
		result << m_regionItemId << uint32(-1);
		return result;
	}

	__forceinline ByteBuffer GetRegionRemoveInfo()
	{
		ByteBuffer result;
		result << uint8(4)
			<< uint16(m_regionItemId);
		return result;
	}
};

typedef CSTLMap<_ZONE_ITEM> ZoneItemArray;
typedef CSTLMap<int> ZoneUserMap;
typedef CSTLMap<int> ZoneNpcMap;

class CNpc;
class CUser;

class Region
{
public:
	ZoneItemArray m_regionItemMap;
	ZoneUserMap m_regionUserMap;
	ZoneNpcMap m_regionNpcMap;
	uint8 m_moving;

	void Add(CUser* pUser);
	void Remove(CUser* pUser);
	void Add(CNpc* pNpc);
	void Remove(CNpc* pNpc);
};
#endif