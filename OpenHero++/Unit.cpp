#include "stdafx.h"

Unit::Unit(bool player)
{
	m_player = player;
}

void Unit::Initialize()
{
	//m_map = NULL;
	m_regionX = 0;
	m_regionZ = 0;
	m_region = NULL;
	m_curMapInstance = NULL;
}

bool Unit::RegisterRegion()
{
	int nRegionX = GetNewRegionX(), nRegionZ = GetNewRegionZ(),
		oRegionX = GetRegionX(), oRegionZ = GetRegionZ();

	if (GetRegion() == NULL || nRegionX == oRegionX && nRegionZ == oRegionZ)
		return false;

	//GetMap()->IsValidPosition();
	if (IsPlayer())
	{
		CUser* me = static_cast<CUser*>(this);
		me->RemoveOldPlayersInVision(oRegionX - nRegionX, oRegionZ - nRegionZ);
		GetMapInstance()->RemoveUser(static_cast<CUser*>(this));
		SetRegion(nRegionX, nRegionZ);
		GetMapInstance()->AddUser(static_cast<CUser*>(this));
		/*GetRegion()->Remove(static_cast<CUser*>(this));
		SetRegion(nRegionX, nRegionZ);
		GetRegion()->Add(static_cast<CUser*>(this));*/
	}
	else
	{
		GetMapInstance()->RegionNpcRemove(GetRegionX(), GetRegionZ(), GetID());
		SetRegion(nRegionX, nRegionZ);
		GetMapInstance()->RegionNpcAdd(GetRegionX(), GetRegionZ(), GetID());
		/*GetRegion()->Remove(static_cast<CNpc*>(this));
		SetRegion(nRegionX, nRegionZ);
		GetRegion()->Add(static_cast<CNpc*>(this));*/
	}

	RemoveRegion(oRegionX - nRegionX, oRegionZ - nRegionZ);
	InsertRegion(nRegionX - oRegionX, nRegionZ - oRegionZ);
}

void Unit::RemoveRegion(int x, int z)
{
	ASSERT(GetMapInstance() != NULL);

	Packet result;
	GetInOut(result, 2);
	//SERVER_INSTANCE_CHANGE
	GetMapInstance()->SendToOldRegions(&result, x, z, GetRegionX(), GetRegionZ());
	//g_main->SendToOldRegions(&result, x, z, GetMap(), GetRegionX(), GetRegionZ());
}

void Unit::InsertRegion(int x, int z)
{
	ASSERT(GetMapInstance() != NULL);

	Packet result;
	GetInOut(result, 1);
	//SERVER_INSTANCE_CHANGE
	GetMapInstance()->SendToNewRegions(&result, x, z, GetRegionX(), GetRegionZ());
	//g_main->SendToNewRegions(&result, x, z, GetMap(), GetRegionX(), GetRegionZ());
}

void Unit::OnDeath(Unit* pKiller)
{

}

void Unit::SendToRegion(Packet& pkt, CUser* pExceptUser /*= NULL*/) 
{
	//SERVER_INSTANCE_CHANGE, set to g_main-> instead to revert
	GetMapInstance()->SendToRegion(pkt, GetRegionX(), GetRegionZ(), pExceptUser);
}

void Unit::SendToMapInstance(Packet & pkt)
{
	GetMapInstance()->SendToAll(pkt);
}
