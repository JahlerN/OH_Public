#include "stdafx.h"
#include "Packets.h"

MapInstance::MapInstance(MAP* pMap)
{
	m_npcIdCounter = NPC_START;
	m_mapTemplate = pMap;

	//Initialize region
	m_region = new Region*[GetMap()->m_sizeRegion.cx];
	for (int i = 0; i < GetMap()->m_sizeRegion.cx; i++)
	{
		m_region[i] = new Region[GetMap()->m_sizeRegion.cy];
		m_region[i]->m_moving = 0;
	}

	foreach(itr, m_npcArray)
	{
		delete itr->second;
		m_npcArray.erase(itr);
	}
}

MapInstance::~MapInstance()
{
	
}

void MapInstance::UpdateRegions(uint32 diff)
{
	for (int x = 0; x < GetMap()->GetXRegionMax(); x++)
	{
		for (int z = 0; z < GetMap()->GetZRegionMax(); z++)
		{
			Region* pRegion = GetRegion(x, z);

			if (!pRegion->m_regionItemMap.IsEmpty())
				CheckForExpiredItems(diff, x, z);
		}
	}
}

void MapInstance::CheckForExpiredItems(uint32 diff, float rX, float rZ)
{
	Region* pRegion = GetRegion(rX, rZ);
	auto itemMap = pRegion->m_regionItemMap.m_UserTypeMap;
	foreach(itr, itemMap)
	{
		_ZONE_ITEM* pZoneItem = (*itr).second;
		pZoneItem->m_dropTracker.Update(diff);
		pZoneItem->m_expirationTracker.Update(diff);
		if (pZoneItem->m_dropTracker.Passed())
		{
			m_regionMutex.Acquire();
			RegionItemRemove(pZoneItem);
			m_regionMutex.Release();
		}
		else if (pZoneItem->m_expirationTracker.Passed())
		{
			pZoneItem->m_reservedForuserId = -1;
			Packet result(PKT_GAMESERVER_ADD_ITEM_TO_GROUND);
			result.append(pZoneItem->GetRegionReservationTimeExpired());
			SendToRegion(result, rX, rZ, NULL);
		}
	}
}

std::list<CNpc*> MapInstance::SpawnNpc(_NPC_DATA* pNpcData, _NPC_GROUP* pGroup)
{
	std::list<CNpc*> groupList;
	uint8 npcMultiplier = 2;
	for (int i = 0; i < pGroup->m_npcsInGroup * (pNpcData->IsMonster() ? npcMultiplier : 1); i++)
	{
		CNpc* pNpc = new CNpc(pNpcData, pGroup, this);

		pNpc->m_uniqueId = m_npcIdCounter;
		m_npcIdCounter++;
		
		if (pNpc->GetMapInstance() == NULL)
		{
			printf("Npc map was NULL when spawning group id = %d", pGroup->m_npcGroupId);
			ASSERT(0);
		}

		RegionNpcAdd(pNpc->GetRegionX(), pNpc->GetRegionZ(), pNpc->GetID());

		m_npcArray.insert(make_pair(pNpc->GetID(), pNpc));
		groupList.push_back(pNpc);
	}

	return groupList;
}

void MapInstance::RegionUserAdd(int rx, int rz, int uId)
{
	//TODO: Figure out what sizeRegion thing actually is, it's 11 now. So does that mean that the big region is divided into 11 smaller ones, and that's the ones we actually work with, like vision?
	if (rx < 0 || rz < 0 || rx >= GetMap()->m_sizeRegion.cx || rz >= GetMap()->m_sizeRegion.cy)
		return;

	int* pInt = NULL;

	m_regionMutex.Acquire();
	pInt = new int;
	*pInt = uId;
	m_region[rx][rz].m_regionUserMap.PutData(uId, pInt);
	m_regionMutex.Release();
}

bool MapInstance::RegionUserRemove(int rx, int rz, int uId)
{
	if (rx < 0 || rz < 0 || rx >= GetMap()->m_sizeRegion.cx || rz >= GetMap()->m_sizeRegion.cy)
		return false;

	Region* region = NULL;
	map<int, int*>::iterator itr;

	m_regionMutex.Acquire();

	region = &m_region[rx][rz];
	region->m_regionUserMap.DeleteData(uId);

	m_regionMutex.Release();
	return true;
}

void MapInstance::RegionNpcAdd(int rx, int rz, int nId)
{
	if (rx < 0 || rz < 0 || rx >= GetMap()->m_sizeRegion.cx || rz >= GetMap()->m_sizeRegion.cy)
		return;

	int* pInt = NULL;

	m_regionMutex.Acquire();
	pInt = new int;
	*pInt = nId;
	m_region[rx][rz].m_regionNpcMap.PutData(nId, pInt);
	m_regionMutex.Release();
}

bool MapInstance::RegionNpcRemove(int rx, int rz, int nId)
{
	if (rx < 0 || rz < 0 || rx >= GetMap()->m_sizeRegion.cx || rz >= GetMap()->m_sizeRegion.cy)
		return false;

	Region* region = NULL;

	m_regionMutex.Acquire();

	region = &m_region[rx][rz];
	region->m_regionNpcMap.DeleteData(nId);

	m_regionMutex.Release();

	return true;
}

bool MapInstance::RegionItemAdd(std::list<_ZONE_ITEM*> itemList)
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_TO_GROUND);
	float rX, rZ;

	m_regionMutex.Acquire();
	foreach(itr, itemList)
	{
		_ZONE_ITEM* pZoneItem = (*itr);
		if (pZoneItem == NULL)
			continue;

		rX = PositionToRegion(pZoneItem->m_x);
		rZ = PositionToRegion(pZoneItem->m_z);

		//Invalid position, cleanup invalid items
		if (rX < 0 || rZ < 0 || rX > GetMap()->GetXRegionMax() || rZ > GetMap()->GetZRegionMax())
		{
			delete pZoneItem->m_pItem;
			delete pZoneItem;
			continue;
		}

		pZoneItem->m_regionItemId = m_uniqueDropId++;
		GetRegion(rX, rZ)->m_regionItemMap.PutData(pZoneItem->m_regionItemId, pZoneItem);

		result.Initialize(PKT_GAMESERVER_ADD_ITEM_TO_GROUND);
																						//True colors,     all yellow but + yellow
		result.append(pZoneItem->GetRegionAddInfo(sObjMgr->GetItemTemplate(pZoneItem->m_pItem->itemId)->m_itemRarity));// (pZoneItem->m_pItem->upgradeCount > 0 ? 162 : 161)
		SendToRegion(result, rX, rZ, NULL);
	}

	m_regionMutex.Release();
	return true;
}

bool MapInstance::RegionItemRemove(_ZONE_ITEM* pZoneItem)
{
	float rX, rZ;
	rX = PositionToRegion(pZoneItem->m_x);
	rZ = PositionToRegion(pZoneItem->m_z);

	if (rX < 0 || rZ < 0 ||
		rX >= GetMap()->GetXRegionMax()
		|| rZ >= GetMap()->GetZRegionMax())
		return false;

	m_regionMutex.Acquire();
	Region* pRegion = GetRegion(rX, rZ);

	_ZONE_ITEM* pItem = pRegion->m_regionItemMap.GetData(pZoneItem->m_regionItemId);
	if (pItem != NULL)
	{
		Packet result(PKT_GAMESERVER_ADD_ITEM_TO_GROUND);//TODO: NO MAGIC NUMBERS, but i'm lazy. Also rename packet to something that doesn't say the opposite.
		result.append(pZoneItem->GetRegionRemoveInfo());
		SendToRegion(result, rX, rZ, NULL);
		if (!pItem->m_itemDeleted)
			delete pItem->m_pItem;
		pRegion->m_regionItemMap.DeleteData(pZoneItem->m_regionItemId);
	}

	m_regionMutex.Release();

	return true;
}

void MapInstance::RegionUserInfoToMe(CUser* pUser)
{
	if (pUser == NULL)
		return;

	uint16 rx = pUser->GetRegionX(), rz = pUser->GetRegionZ();
	foreach_region(x, z)
		SendRegionUserList(rx + x, rz + z, pUser);
}

void MapInstance::RegionNpcInfoToMe(CUser * pSender)
{
	if (pSender == NULL)
		return;

	uint16 rx = pSender->GetRegionX(), rz = pSender->GetRegionZ();
	foreach_region(x, z)
		SendRegionNpcList(rx + x, rz + z, pSender);
}

void MapInstance::RegionItemInfoToMe(CUser * pSender)
{
	if (pSender == NULL)
		return;

	uint16 rx = pSender->GetRegionX(), rz = pSender->GetRegionZ();
	foreach_region(x, z)
		SendRegionItemList(rx + x, rz + z, pSender);
}

void MapInstance::SendRegionUserList(uint16 rx, uint16 rz, CUser* pSendUser)//, Packet& result)
{
	if (rx < 0 || rz < 0 || rx > GetMap()->GetXRegionMax() || rz > GetMap()->GetZRegionMax())
		return;

	m_regionMutex.Acquire();

	Region* pRegion = GetRegion(rx, rz);
	Packet result;
	foreach(itr, pRegion->m_regionUserMap.m_UserTypeMap)
	{
		result.Initialize(PKT_GAMESERVER_REGION_CHANGE_USER, uint8(0x01));
		CUser* pUser = g_main->GetUserPtr(itr->first);
		//TODO: Only send to user IF they haven't already been sent(meaning they're already in the region.)
		if (pUser == NULL || !pUser->IsInGame() || pUser->GetRegionX() != rx || pUser->GetRegionZ() != rz)
			continue;

		//result << pUser->GetID();
		result.append(pUser->GetUserInfoForRegion());
		pSendUser->Send(&result);
	}

	m_regionMutex.Release();
}

void MapInstance::SendRegionNpcList(uint16 rx, uint16 rz, CUser * pSendUser)
{
	if ( rx < 0 || rz < 0 || rx > GetMap()->GetXRegionMax() || rz > GetMap()->GetZRegionMax())
		return;

	m_regionMutex.Acquire();

	Region* pRegion = GetRegion(rx, rz);
	Packet result;
	foreach(itr, pRegion->m_regionNpcMap.m_UserTypeMap)
	{
		result.Initialize(PKT_GAMESERVER_REGION_CHANGE_NPC, uint8(0x01));
		CNpc* pNpc = GetNpcPtr(itr->first);
		if (pNpc == NULL || pNpc->IsDead() || pNpc->GetType() == 25)//25=summon, shouldn't be seen untill actualy summoned.
			continue;

		pNpc->GetNpcInfo(result);
		pSendUser->Send(&result);
	}
	m_regionMutex.Release();
}

void MapInstance::SendRegionItemList(uint16 rx, uint16 rz, CUser * pSendUser)
{
	if (rx < 0 || rz < 0 || rx > GetMap()->GetXRegionMax() || rz > GetMap()->GetZRegionMax())
		return;

	m_regionMutex.Acquire();

	Region* pRegion = GetRegion(rx, rz);
	Packet result;
	foreach(itr, pRegion->m_regionItemMap.m_UserTypeMap)
	{
		result.Initialize(PKT_GAMESERVER_ADD_ITEM_TO_GROUND);
		_ZONE_ITEM* pZoneItem = (*itr).second;
		if (pZoneItem == NULL)//TODO: Fix this, we need to check if it's a non combat npc and skip alive check, or add a state
			continue;

		result.append(pZoneItem->GetRegionAddInfo(sObjMgr->GetItemTemplate(pZoneItem->m_pItem->itemId)->m_itemRarity));
		pSendUser->Send(&result);
	}
	m_regionMutex.Release();
}

//THERE ARE NO NPC CHECKS IN HERE! It just returns all npcs inside the aoe.
std::list<Unit*> MapInstance::GetListOfNpcsHit(float x, float z, float aoe)
{
	std::list<Unit*> hitList;
	float mapSize = GetMap()->m_mapSize;
	float leftX = x - aoe, leftZ = z - aoe,
		rightX = x + aoe, rightZ = z + aoe;
	if (leftX < 0)
		leftX = 0;
	if (leftZ < 0)
		leftZ = 0;
	if (rightX > mapSize)
		rightX = mapSize;
	if (rightZ > mapSize)
		rightZ = mapSize;

	int uRX = PositionToRegion(x), uRZ = PositionToRegion(z);

	foreach_region(rx, rz)
	{
		if (rx + uRX < 0 || rz + uRZ < 0 ||
			rx + uRX > GetMap()->GetXRegionMax() ||
			rz + uRZ > GetMap()->GetZRegionMax())
			continue;

		Region* pRegion = GetRegion(rx + uRX, rz + uRZ);
		if (pRegion->m_regionNpcMap.IsEmpty())
			continue;

		foreach_stlmap(itr, pRegion->m_regionNpcMap)
		{
			CNpc* pNpc = GetNpcPtr(itr->first);
			if (pNpc == NULL || !pNpc->IsMonster())
				continue;

			float nX = pNpc->GetX(), nZ = pNpc->GetZ();

			if (nX >= leftX && nZ >= leftZ &&
				nX <= rightX && nZ <= rightZ)
				hitList.push_back(pNpc);
		}
	}

	return hitList;
}

void MapInstance::SendToAll(Packet & pkt)
{
	m_regionMutex.Acquire();
	foreach(itr, m_userArray)
	{
		CUser* pUser = (*itr);
		if (pUser == NULL || !pUser->IsInGame())
			continue;
		pUser->Send(&pkt);
	}
	m_regionMutex.Release();
}

_ZONE_ITEM* MapInstance::LootDroppedItemById(uint16 regionItemId, float rX, float rZ)
{
	foreach_region(x, z)
	{
		Region* pRegion = GetRegion(rX + x, rZ + z);
		if (pRegion == NULL)
			return NULL;

		_ZONE_ITEM* pZoneItem = pRegion->m_regionItemMap.GetData(regionItemId);
		if (pZoneItem == NULL)
			continue;

		if (pZoneItem->m_pItem != NULL)
		{
			//Remove from clients
			return pZoneItem;
		}
	}
	return NULL;
}

void MapInstance::SendToRegion(Packet& pkt, int x, int z, CUser* exceptUser = NULL)
{
	foreach_region(rx, rz)
		SendToRegionUnit(&pkt, rx + x, rz + z, exceptUser);
}

void MapInstance::SendToOldRegions(Packet* pkt, int oX, int oZ, int x, int z, CUser* pExceptUser /*= NULL*/)
{
	if (oX != 0)
	{
		SendToRegionUnit(pkt, x + oX * 2, z + oZ - 1);
		SendToRegionUnit(pkt, x + oX * 2, z + oZ);
		SendToRegionUnit(pkt, x + oX * 2, z + oZ + 1);
	}

	if (oZ != 0)
	{
		SendToRegionUnit(pkt, x + oX, z + oZ * 2);
		if (oX < 0)
			SendToRegionUnit(pkt, x + oX + 1, z + oZ * 2);
		else if (oX > 0)
			SendToRegionUnit(pkt, x + oX - 1, z + oZ * 2);
		else
		{
			SendToRegionUnit(pkt, x + oX - 1, z + oZ * 2);
			SendToRegionUnit(pkt, x + oX + 1, z + oZ * 2);
		}
	}
}

void MapInstance::SendToNewRegions(Packet* pkt, int nX, int nZ, int x, int z, CUser* pExceptUser /*= NULL*/)
{
	if (nX != 0)
	{
		SendToRegionUnit(pkt, x + nX, z - 1);
		SendToRegionUnit(pkt, x + nX, z);
		SendToRegionUnit(pkt, x + nX, z + 1);
	}

	if (nZ != 0)
	{
		SendToRegionUnit(pkt, x, z + nZ);

		if (nX < 0)
			SendToRegionUnit(pkt, x + 1, z + nZ);
		else if (nX > 0)
			SendToRegionUnit(pkt, x - 1, z + nZ);
		else
		{
			SendToRegionUnit(pkt, x - 1, z + nZ);
			SendToRegionUnit(pkt, x + 1, z + nZ);
		}
	}
}

void MapInstance::SendToRegionUnit(Packet* pkt, int x, int z, CUser* exceptUser /* = NULL*/)
{
	MAP* pMap = GetMap();
	if (pMap == NULL || x < 0 || z < 0 || x > pMap->GetXRegionMax() || z > pMap->GetZRegionMax())
		return;

	m_regionMutex.Acquire();
	Region* pRegion = GetRegion(x, z);

	foreach(itr, pRegion->m_regionUserMap.m_UserTypeMap)
	{
		CUser* pUser = g_main->GetUserPtr(itr->first);
		if (pUser == NULL || pUser == exceptUser || !pUser->IsInGame())
			continue;

		pUser->Send(pkt);
	}
	m_regionMutex.Release();
}

void MapInstance::AddUser(CUser* pUser)
{
	if (pUser == NULL)
		return;
	m_regionMutex.Acquire();

	m_userArray.push_back(pUser);
	GetRegion(pUser->GetRegionX(), pUser->GetRegionZ())->Add(pUser);

	m_regionMutex.Release();
}

void MapInstance::RemoveUser(CUser * pUser)
{
	if (pUser == NULL)
		return;

	m_regionMutex.Acquire();

	m_userArray.remove(pUser);
	GetRegion(pUser->GetRegionX(), pUser->GetRegionZ())->Remove(pUser);

	m_regionMutex.Release();
}

void MapInstance::RegionExit(CUser * pUser, float oRX, float oRZ)
{
	foreach_region(x, z)
	{

	}
}