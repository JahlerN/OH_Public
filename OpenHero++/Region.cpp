#include "stdafx.h"

extern CRITICAL_SECTION g_region_critical;

void Region::Add(CUser* pUser)
{
	EnterCriticalSection(&g_region_critical);
	int* id = new int();
	*id = pUser->GetID();
	m_regionUserMap.PutData(pUser->GetID(), id);
	LeaveCriticalSection(&g_region_critical);
}

void Region::Remove(CUser* pUser)
{
	EnterCriticalSection(&g_region_critical);
	m_regionUserMap.DeleteData(pUser->GetID());
	LeaveCriticalSection(&g_region_critical);
}

void Region::Add(CNpc* pNpc)
{
	EnterCriticalSection(&g_region_critical);
	int* id = new int();
	*id = pNpc->GetID();
	m_regionNpcMap.PutData(pNpc->GetID(), id);
	LeaveCriticalSection(&g_region_critical);
}

void Region::Remove(CNpc* pNpc)
{
	EnterCriticalSection(&g_region_critical);
	m_regionNpcMap.DeleteData(pNpc->GetID());
	LeaveCriticalSection(&g_region_critical);
}