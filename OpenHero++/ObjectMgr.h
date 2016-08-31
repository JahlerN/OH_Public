#ifndef OBJECTMGR_H
#define OBJECTMGR_h

//#include "..\OpenHero++\DBAgent.h"
//#include "..\OpenHero++\GameServer.h"
//#include "..\OpenHero++\ItemTemplate.h"
#include "..\Database\GameDatabase.h"
#include "..\Database\AccountDatabase.h"

class ObjectMgr
{
public:
	static ObjectMgr* instance();

	std::vector<_SERVER_TAB*> LoadServerTables();
	bool LoadMapTables();
	MAP* ReadMapFile(_ZONE_INFO* pZone);

	bool LoadItemSet();
	bool LoadItemTemplate();
	bool LoadItemDropTable();
	bool LoadGamblingItemTable();
	bool LoadLevelData();
	bool LoadZoneChangeTable();
	bool LoadCharacterInfoTable();

	_SET_BONUS* GetSetBonus(uint32 setId);
	_ITEM_TABLE* GetItemTemplate(uint32 itemId);
	_ITEM_DROP_TABLE* GetItemDropTable(uint32 itemId);
	_GAMBLING_ITEM_TABLE* GetGamblingItemTable(uint32 id);
	_LEVEL_DATA* GetLevelData(uint32 level);
	_ZONECHANGE_DATA* GetZoneChangeData(uint8 zoneId);
	_CHARACTER_DATA* GetCharacterInfo(uint8 charTypeId);

	//std::map<uint32, CNpc*> GetNpcTable() { return m_npcTableArray.m_UserTypeMap; }
	std::map<long, _ITEM_TABLE*> GetItemTableArray() { return m_itemTableArray.m_UserTypeMap; }
	std::map<long, MAP*> GetMapArray() { return m_zoneTableArray.m_UserTypeMap; }

private:
	ObjectMgr();
	~ObjectMgr();

	SetBonusArray m_setBonusArray;
	ItemTableArray m_itemTableArray;
	ItemDropTableArray m_dropTableArray;
	GamblingItemTableArray m_gamblingItemArray;
	LevelDataArray m_levelDataArray;

	ZoneTableArray m_zoneTableArray;

	ZoneChangeTableArray m_zoneChangeArray;
	CharacterInfoArray m_characterInfoArray;
};

#define sObjMgr ObjectMgr::instance()
#endif