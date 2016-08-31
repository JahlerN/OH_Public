#include "stdafx.h"
//#include "MAP.h"
#include "GameServer.h"
#include "User.h"
//#include "Region.h"
//#include "RoomEvent.h"
#include "Packets.h"

using namespace std;

extern CRITICAL_SECTION	g_region_critical;

CMapInfo::CMapInfo()
{
	m_event = 0;
}

CMapInfo::~CMapInfo()
{

}

MAP::MAP()
{
	m_mapSize = 0;
	m_unitDist = 0.0f;
	m_height = NULL;
	m_sizeRegion.cx = 0;
	m_sizeRegion.cy = 0;
	m_sizeMap.cx = 0;
	m_sizeMap.cy = 0;

	m_region = NULL;
	m_map = NULL;
	m_zoneNum = 0;
	m_roomType = 0;
	m_roomEvent = 0;
	m_roomStatus = 0;
}

bool MAP::Initialize(_ZONE_INFO* pZone)
{
	m_serverNum = pZone->m_serverNum;
	m_zoneNum = pZone->m_zoneNum;
	m_mapName = pZone->m_mapName;
	m_smdFile = pZone->m_smdFile;
	m_roomEvent = pZone->m_roomEvent;

	//LoadMap(NULL);

	return true;
}

MAP::~MAP()
{

}

void MAP::RemoveMapData()
{
	if (m_region) {
		for (int i = 0; i< m_sizeRegion.cx; i++) {
			delete[] m_region[i];
			m_region[i] = NULL;
		}
		delete[] m_region;
		m_region = NULL;
	}

	if (m_height) {
		for (int i = 0; i< m_mapSize; i++) {
			delete[] m_height[i];
			m_height[i] = NULL;
		}
		delete[] m_height;
	}

	if (m_map) {
		for (int i = 0; i<m_sizeMap.cx; i++) {
			delete[] m_map[i];
			m_map[i] = NULL;
		}
		delete[] m_map;
		m_map = NULL;
	}

	//m_ObjectEventArray.DeleteAllData();
	//m_arRoomEventArray.DeleteAllData();
}

bool MAP::IsMovable(int dest_x, int dest_y)
{
	if (dest_x < 0 || dest_y < 0 || !m_map)
		return false;
	if (dest_x >= m_sizeMap.cx || dest_y >= m_sizeMap.cy)
		return false;

	bool ret = false;
	if (m_map[dest_x][dest_y].m_event == 0)
		ret = true;

	return ret;
}

bool MAP::LoadMap(HANDLE hFile)
{
	//TODO: Remove terrain load and tile, load map size instead, then load collision data, that's all we have for now.(make load time based on the info we got(mapSize, fake unit dist(2||4)
	DWORD dwRWC;
	//No colision map.
	float mapWidth;
	ReadFile(hFile, &m_mapSize, sizeof(float), &dwRWC, NULL);
	ReadFile(hFile, &mapWidth, sizeof(float), &dwRWC, NULL);
	ReadFile(hFile, &m_unitDist, sizeof(float), &dwRWC, NULL);
	//printf("Loading map %s\nX: %.1f Y: %.1f unitDist: %.0f.\n", m_mapName.c_str(), m_mapSize, mapWidth, m_unitDist);
	//m_mapSize = 512;
	//m_unitDist = 4;

	m_height = new float*[m_mapSize];
	for (int i = 0; i < m_mapSize; i++)
		m_height[i] = new float[m_mapSize];

	for (int z = 0; z < m_mapSize; z++)
		for (int x = 0; x < m_mapSize; x++)
			m_height[x][z] = m_mapSize / 4;

	//int mapWidth = 512;

	m_sizeRegion.cx = (int)(mapWidth / VIEW_DIST) + 1;
	m_sizeRegion.cy = (int)(mapWidth / VIEW_DIST) + 1;

	m_sizeMap.cx = m_mapSize;
	m_sizeMap.cy = m_mapSize;
	
	m_region = new Region*[m_sizeRegion.cx];
	for (int i = 0; i < m_sizeRegion.cx; i++)
	{
		m_region[i] = new Region[m_sizeRegion.cy];
		m_region[i]->m_moving = 0;
	}

	return true;

	//LoadTerrain(hFile);
	//m_N3ShapeMgr.Create((m_mapSize - 1) * m_unitDist, (m_mapSize - 1) * m_unitDist);
	//if (!m_N3ShapeMgr.LoadCollisionData(hFile))
	//	return false;

	//if ((m_mapSize - 1) * m_unitDist != m_N3ShapeMgr.Width() ||
	//	(m_mapSize - 1) * m_unitDist != m_N3ShapeMgr.Height())
	//	return false;

	//int mapWidth = (int)m_N3ShapeMgr.Width();

	//m_sizeRegion.cx = (int)(mapWidth / VIEW_DIST) + 1;
	//m_sizeRegion.cy = (int)(mapWidth / VIEW_DIST) + 1;

	//m_sizeMap.cx = m_mapSize;
	//m_sizeMap.cy = m_mapSize;

	//m_region = new Region*[m_sizeRegion.cx];
	//for (int i = 0; i < m_sizeRegion.cx; i++)
	//{
	//	m_region[i] = new Region[m_sizeRegion.cy];
	//	m_region[i]->m_moving = 0;
	//}

	////LoadMapEvent(hFile);//TODO Map events etc?
	//LoadMapTile(hFile);
}

void MAP::LoadTerrain(HANDLE hFile)
{
	DWORD dwRWC;
	ReadFile(hFile, &m_mapSize, sizeof(int), &dwRWC, NULL);
	ReadFile(hFile, &m_unitDist, sizeof(float), &dwRWC, NULL);

	m_height = new float*[m_mapSize];
	for (int i = 0; i < m_mapSize; i++)
		m_height[i] = new float[m_mapSize];

	int x, z;

	for (z = 0; z < m_mapSize; z++)
		for (x = 0; x < m_mapSize; x++)
			ReadFile(hFile, &(m_height[x][z]), sizeof(float), &dwRWC, NULL);
}

float MAP::GetHeight(float x, float z)
{
	int iX, iZ;
	iX = (int)(x / m_unitDist);
	iZ = (int)(z / m_unitDist);

	float y;
	float h1, h2, h3;
	float dX, dZ;
	dX = (x - iX * m_unitDist) / m_unitDist;
	dZ = (z - iZ * m_unitDist) / m_unitDist;

	if (!(dX >= 0.0f && dZ >= 0.0f && dX < 1.0f && dZ < 1.0f))
		return FLT_MIN;

	if ((iX + iZ) & 2 == 1)
	{
		if (dX + dZ < 1.0f)
		{
			h1 = m_height[iX][iZ + 1];
			h2 = m_height[iX + 1][iZ];
			h3 = m_height[iX][iZ];

			float h12 = h1 + (h2 - h1) * dX;
			float h32 = h3 + (h2 - h3) * dX;
			y = h32 + (h12 - h32) * ((dZ) / (1.0f - dX));
		}
		else
		{
			h1 = m_height[iX][iZ + 1];
			h2 = m_height[iX + 1][iZ];
			h3 = m_height[iX + 1][iZ + 1];

			if (dX == 0)
				return h1;

			float h12 = h1 + (h2 - h1) * dX;
			float h13 = h1 + (h3 - h1) * dX;
			y = h13 + (h12 - h13) * ((1.0f - dZ) / (dX));
		}
	}
	else
	{
		if (dZ > dX)
		{
			h1 = m_height[iX][iZ + 1];
			h2 = m_height[iX + 1][iZ + 1];
			h3 = m_height[iX][iZ];

			float h12 = h1 + (h2 - h1) * dX;
			float h32 = h3 + (h2 - h3) * dX;
			y = h12 + (h32 - h12) * ((1.0f - dZ) / (1.0f - dX));
		}
		else
		{
			h1 = m_height[iX][iZ];
			h2 = m_height[iX + 1][iZ];
			h3 = m_height[iX + 1][iZ + 1];

			float h12 = h1 + (h2 - h1) * dX;
			float h13 = h1 + (h3 - h1) * dX;
			y = h12 + (h13 - h12) * ((dZ) / (dX));
		}
	}
	return y;
}

bool MAP::ObjectIntersect(float x1, float z1, float y1, float x2, float z2, float y2)
{
	__Vector3 vec1(x1, y1, z1), vec2(x2, y2, z2);
	__Vector3 vDir = vec2 - vec1;
	float speed = vDir.Magnitude();
	vDir.Normalize();

	return m_N3ShapeMgr.CheckCollision(vec1, vDir, speed);
}

//void MAP::RegionUserAdd(int rx, int rz, int uId)
//{
//	//TODO: Figure out what sizeRegion thing actually is, it's 11 now. So does that mean that the big region is divided into 11 smaller ones, and that's the ones we actually work with, like vision?
//	if (rx < 0 || rz < 0 || rx >= m_sizeRegion.cx || rz >= m_sizeRegion.cy)
//		return;
//
//	int* pInt = NULL;
//
//	EnterCriticalSection(&g_region_critical);
//	pInt = new int;
//	*pInt = uId;
//	m_region[rx][rz].m_regionUserMap.PutData(uId, pInt);
//	LeaveCriticalSection(&g_region_critical);
//}
//
//bool MAP::RegionUserRemove(int rx, int rz, int uId)
//{
//	if (rx < 0 || rz < 0 || rx >= m_sizeRegion.cx || rz >= m_sizeRegion.cy)
//		return false;
//
//	Region* region = NULL;
//	map<int, int*>::iterator itr;
//
//	EnterCriticalSection(&g_region_critical);
//
//	region = &m_region[rx][rz];
//	region->m_regionUserMap.DeleteData(uId);
//
//	LeaveCriticalSection(&g_region_critical);
//
//	return true;
//}
//
//void MAP::RegionNpcAdd(int rx, int rz, int nId)
//{
//	if (rx < 0 || rz < 0 || rx >= m_sizeRegion.cx || rz >= m_sizeRegion.cy)
//		return;
//
//	int* pInt = NULL;
//
//	EnterCriticalSection(&g_region_critical);
//	pInt = new int;
//	*pInt = nId;
//	m_region[rx][rz].m_regionNpcMap.PutData(nId, pInt);
//	LeaveCriticalSection(&g_region_critical);
//}
//
//bool MAP::RegionNpcRemove(int rx, int rz, int nId)
//{
//	if (rx < 0 || rz < 0 || rx >= m_sizeRegion.cx || rz >= m_sizeRegion.cy)
//		return false;
//
//	Region* region = NULL;
//	map<int, int*>::iterator itr;
//
//	EnterCriticalSection(&g_region_critical);
//
//	region = &m_region[rx][rz];
//	region->m_regionNpcMap.DeleteData(nId);
//
//	LeaveCriticalSection(&g_region_critical);
//
//	return true;
//}
//
//bool MAP::RegionItemAdd(uint16 rx, uint16 rz, _ZONE_ITEM * pItem)
//{
//	if (rx >= GetXRegionMax()
//		|| rz >= GetZRegionMax())
//		return false;
//
//	EnterCriticalSection(&g_region_critical);
//	
//	pItem->bundle_index = m_bundleMax++;
//	m_region[rx][rz].m_regionItemMap.PutData(pItem->bundle_index, pItem);
//	if (m_bundleMax > ZONEITEM_MAX)
//		m_bundleMax = 1;
//
//	LeaveCriticalSection(&g_region_critical);
//
//	return true;
//}
//
//bool MAP::RegionItemRemove(uint16 rx, uint16 rz, int bundle_index, int itemId, int count)
//{
//	if (rx >= GetXRegionMax()
//		|| rz >= GetZRegionMax())
//		return false;
//
//	_ZONE_ITEM* pItem = NULL;
//	Region* region = NULL;
//	bool find = false;
//	short iCount = 0;
//
//	EnterCriticalSection(&g_region_critical);
//
//	pItem = (_ZONE_ITEM*)region->m_regionItemMap.GetData(bundle_index);
//	if (pItem)
//	{
//		for (int j = 0; j < 6; j++)
//			if (pItem->itemId[j] == itemId && pItem->count[j] == count)
//			{
//				pItem->itemId[j] = 0;
//				pItem->count[j] = 0;
//				find = true;
//				break;
//			}
//		if (find)
//		{
//			for (int j = 0; j < 6; j++)
//				if (pItem->itemId[j] != 0)
//					iCount++;
//			if (!iCount)
//				region->m_regionItemMap.DeleteData(bundle_index);
//		}
//	}
//
//	LeaveCriticalSection(&g_region_critical);
//
//	return true;
//}

void MAP::LoadMapTile(HANDLE hFile)
{
	int x1 = m_sizeMap.cx;
	int z1 = m_sizeMap.cy;
	DWORD dwNum;
	short** event;
	event = new short*[m_sizeMap.cx];
	for (int a = 0; a < m_sizeMap.cx; a++)
		event[a] = new short[m_sizeMap.cx];
	for (int x = 0; x < m_sizeMap.cx; x++)
		ReadFile(hFile, event[x], sizeof(short) * m_sizeMap.cy, &dwNum, NULL);

	m_map = new CMapInfo*[m_sizeMap.cx];

	for (int i = 0; i < m_sizeMap.cx; i++)
		m_map[i] = new CMapInfo[m_sizeMap.cy];
	
	int count = 0;
	for (int i = 0; i < m_sizeMap.cy; i++)
		for (int j = 0; j < m_sizeMap.cx; j++)
		{
			m_map[j][i].m_event = (short)event[j][i];

			if (m_map[j][i].m_event >= 1)
			{
				count++;
			}
		}

	if (event)
	{
		for (int i = 0; i < m_sizeMap.cx; i++)
		{
			delete[] event[i];
			event[i] = NULL;
		}
		delete[] event;
		event = NULL;
	}
}

//Region* MAP::GetRegion(uint16 regionX, uint16 regionZ)
//{
//	Region* pRegion = NULL;
//
//	if (regionX > GetXRegionMax()
//		|| regionZ > GetZRegionMax())
//		return pRegion;
//
//	EnterCriticalSection(&g_region_critical);
//	pRegion = &m_region[regionX][regionZ];
//	LeaveCriticalSection(&g_region_critical);
//
//	return pRegion;
//}
//
//int MAP::GetRegionUserSize(int rx, int rz)
//{
//	if (rx < 0 || rz < 0 || rx >= m_sizeRegion.cx || rz >= m_sizeRegion.cy)
//		return -1;
//
//	EnterCriticalSection(&g_region_critical);
//	Region* region = NULL;
//	region = &m_region[rx][rz];
//	int ret = region->m_regionUserMap.GetSize();
//	LeaveCriticalSection(&g_region_critical);
//
//	return ret;
//}
//
//int MAP::GetRegionNpcSize(int rx, int rz)
//{
//	if (rx < 0 || rz < 0 || rx >= m_sizeRegion.cx || rz >= m_sizeRegion.cy)
//		return -1;
//
//	EnterCriticalSection(&g_region_critical);
//	Region* region = NULL;
//	region = &m_region[rx][rz];
//	int ret = region->m_regionNpcMap.GetSize();
//	LeaveCriticalSection(&g_region_critical);
//
//	return ret;
//}

//void MAP::LoadObjectEvent(HANDLE hFile)
//{
//	int eventObjectCount = 0;
//	DWORD dwNum;
//
//	ReadFile(hFile, &eventObjectCount, 4, &dwNum, NULL);
//	for (int i = 0; i < eventObjectCount; i++)
//	{
//		_OBJECT_EVENT* pEvent = new _OBJECT_EVENT;
//		ReadFile(hFile, &(pEvent->m_belong), 4, &dwNum, NULL);
//		ReadFile(hFile, &(pEvent->m_index), 2, &dwNum, NULL);
//		ReadFile(hFile, &(pEvent->m_type), 2, &dwNum, NULL);
//		ReadFile(hFile, &(pEvent->m_controlNpcId), 2, &dwNum, NULL);
//		ReadFile(hFile, &(pEvent->m_status), 2, &dwNum, NULL);
//		ReadFile(hFile, &(pEvent->m_posX), 4, &dwNum, NULL);
//		ReadFile(hFile, &(pEvent->m_posY), 4, &dwNum, NULL);
//		ReadFile(hFile, &(pEvent->m_posZ), 4, &dwNum, NULL);
//
//		if (pEvent->m_type == OBJECT_GATE || pEvent->m_type == OBJECT_GATE2
//			|| pEvent->m_type == OBJECT_GATE_LEVER || pEvent->m_type == OBJECT_ANVIL
//			|| pEvent->m_type == OBJECT_ARTIFACT)
//			g_main->AddObjectEventNpc(pEvent, m_zoneNum);
//
//		if (pEvent->m_index <= 0)
//			continue;
//
//		if (!m_objectEventArray.PutData(pEvent->m_index, pEvent))
//			delete pEvent;
//	}
//}

bool MAP::LoadRoomEvent(int zoneNum)
{
	return true;
}

int MAP::IsRoomCheck(float fx, float fz)
{
	return 0;
}

//CRoomEvent* MAP::SetRoomEvent(int num)
//{
//
//}

bool MAP::IsRoomStatusCheck()
{
	return true;
}

//void MAP::InitializeRoom()
//{
//	foreach_stlmap(itr, m_roomEventArray)
//	{
//
//	}
//}