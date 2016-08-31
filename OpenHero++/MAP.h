#ifndef MAP_H
#define MAP_H

#include "../N3Base/N3ShapeMgr.h"
#include "../SharedFiles/STLMap.h"
#include "Region.h"
#include "Define.h"
//#include "RoomEvent.h"

struct CSize
{
	int cx;
	int cy;

	CSize()
	{
		cx = 0;
		cy = 0;
	}

	CSize(int x, int y)
	{
		cx = x;
		cy = y;
	}

	CSize& operator+(CSize cSize)
	{
		cx += cSize.cx;
		cy += cSize.cy;
	}
};

struct _ZONE_INFO
{
	int m_serverNum;
	uint32 m_zoneNum;
	std::string m_mapName;
	std::string m_smdFile;
	uint8 m_roomEvent;
	float m_initX, m_initZ, m_initY;
	uint8 m_type;
	bool isPVPArea;

	_ZONE_INFO()
	{
		
	}
};

struct _ZONECHANGE_DATA
{
	uint32 m_zoneChangeId;
	uint8 m_toZoneId;
	float m_posX;
	float m_posZ;
	uint32 m_reqLevel;
};

struct _ZONESTART_POSITION
{
	uint8 m_zoneId;
	float m_x;
	float m_z;
	float m_y;
	uint8 m_rangeX;
	uint8 m_rangeZ;
	uint8 m_rangeY;
};

typedef CSTLMap<_OBJECT_EVENT> ObjectEventArray;
//typedef CSTLMap<CRoomEvent> RoomEventArray;

class Region;
class Unit;
class CNpc;
class CUser;

class CMapInfo
{
public:
	short m_event;

	CMapInfo();
	virtual ~CMapInfo();
};

//TODO: Remove all active data from this class
class MAP
{
public:
	CN3ShapeMgr m_N3ShapeMgr;
	CMapInfo** m_map;
	Region** m_region;
	CSize m_sizeMap;
	CSize m_sizeRegion;
	uint8 m_zoneNum;
	int m_serverNum;
	std::string m_mapName;
	std::string m_smdFile;
	float m_mapSize;
	float m_unitDist;
	float** m_height;
	uint8 m_roomType;
	uint8 m_roomEvent;
	uint8 m_roomStatus;
	uint8 m_initRoomCount;
//	ObjectEventArray m_objectEventArray; TODO: Implement this, fuck it for now.
//	RoomEventArray m_roomEventArray;
	//short m_karusRoom;
	//short m_elmoradRoom;

	//C3DMap stuff
	uint32 m_bundleMax;

public:
	MAP();
	virtual ~MAP();

	bool Initialize(_ZONE_INFO* pZone);

	bool LoadMap(HANDLE hFile);
	void LoadTerrain(HANDLE hFile);
	void LoadMapTile(HANDLE hFile);
	void LoadObjectEvent(HANDLE hFile);
	bool LoadRoomEvent(int zoneNum);
	bool ObjectIntersect(float x1, float z1, float y1, float x2, float z2, float y2);
	float GetHeight(float x, float z);

	//bool RegionNpcRemove(int rx, int rz, int nId);
	//void RegionNpcAdd(int rx, int rz, int nId);
	//bool RegionUserRemove(int rx, int rz, int uId);
	//void RegionUserAdd(int rx, int rz, int uId);
	//int GetRegionUserSize(int rx, int rz);
	//int GetRegionNpcSize(int rx, int rz);

	int GetXRegionMax() { return m_sizeRegion.cx - 1; }
	int GetZRegionMax() { return m_sizeRegion.cy - 1; }

	int IsRoomCheck(float fx, float fz);
	bool IsRoomStatusCheck();

	bool IsMovable(int dest_x, int dest_y);
	void InitializeRoom();

	//methods from C3DMap
	//Region* GetRegion(uint16 regionX, uint16 regionZ);
	//bool RegionItemRemove(uint16 rx, uint16 rz, int bundle_index, int itemid, int count);
	//bool RegionItemAdd(uint16 rx, uint16 rz, _ZONE_ITEM* pItem);
	//BOOL ObjectCollision(float x1, float z1, float y1, float x2, float z2, float y2); Same as intersect
	__forceinline bool IsValidPosition(float x, float z, float y) { return (x < m_N3ShapeMgr.Width() && z < m_N3ShapeMgr.Height()); } //TODO: more thorough check wanted

//	CRoomEvent* SetRoomEvent(int number);
	__forceinline int PositionToRegion(float pos) { return pos / VIEW_DIST; }

protected:
	void RemoveMapData();
};
#endif