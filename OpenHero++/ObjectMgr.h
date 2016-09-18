#ifndef OBJECTMGR_H
#define OBJECTMGR_h

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
	bool LoadMakeItemTable();
	bool LoadMakeItemFusionTable();
	bool LoadDismantleItemTable();
	bool LoadLevelData();
	bool LoadZoneChangeTable();
	bool LoadZoneStartPositionTable();
	bool LoadCharacterInfoTable();
	bool LoadNpcGossipTable();
	bool LoadGossipOptionTable();
	bool LoadNpcInfoTable();
	bool LoadNpcGroupTable();
	bool LoadSkillTable();
	bool LoadSkillBookTable();

	bool LoadShopTable();
	bool LoadShopItemTable();

	_SET_BONUS* GetSetBonus(uint32 setId);
	_ITEM_TABLE* GetItemTemplate(uint32 itemId);
	_ITEM_DROP_TABLE* GetItemDropTable(uint32 itemId);
	_GAMBLING_ITEM_TABLE* GetGamblingItemTable(uint32 id);
	_MAKE_ITEM_TABLE* GetMakeItemTable(uint32 itemId);
	_MAKE_ITEM_FUSION_TABLE* GetMakeItemFusionTable(uint32 itemId);
	_DISMANTLE_ITEM* GetDismantleItemTable(uint32 itemId);
	_LEVEL_DATA* GetLevelData(uint32 level);
	_ZONECHANGE_DATA* GetZoneChangeData(uint8 zoneId);
	_ZONESTART_POSITION* GetZoneStartPosition(uint8 zoneId);
	_CHARACTER_DATA* GetCharacterInfo(uint8 charTypeId);
	_NPC_GOSSIP* GetNpcGossipInfo(uint32 gossipId);
	_GOSSIP_OPTION* GetGossipOptionInfo(uint32 optionId);
	_NPC_DATA* GetNpcInfo(uint32 npcId);
	_NPC_GROUP* GetNpcGroupInfo(uint32 npcId);
	_SKILL_DATA* GetSkillInfo(uint32 skillId);
	_SKILL_BOOK_DATA* GetSkillBookInfo(uint32 skillBookId);
	_SHOP_TABLE* GetShopTableInfo(uint32 shopId);
	_SHOP_TABLE_ITEM* GetShopTableItemInfo(uint32 shopItemId);
	
	//std::map<uint32, CNpc*> GetNpcTable() { return m_npcTableArray.m_UserTypeMap; }
	std::map<long, _ITEM_TABLE*> GetItemTableArray() { return m_itemTableArray.m_UserTypeMap; }
	std::map<long, MAP*> GetMapArray() { return m_zoneTableArray.m_UserTypeMap; }
	std::map<long, _NPC_GROUP*> GetNpcInfoArray() { return m_npcGroupArray.m_UserTypeMap; }

private:
	ObjectMgr();
	~ObjectMgr();

	SetBonusArray m_setBonusArray;
	ItemTableArray m_itemTableArray;
	ItemDropTableArray m_dropTableArray;
	GamblingItemTableArray m_gamblingItemArray;
	MakeItemTableArray m_makeItemTableArray;
	MakeItemFusionTableArray m_makeItemTableFusionArray;
	DismantleItemTableArray m_dismantleitemTableArray;
	LevelDataArray m_levelDataArray;

	ZoneTableArray m_zoneTableArray;

	ZoneChangeTableArray m_zoneChangeArray;
	ZoneStartTableArray m_zoneStartTableArray;
	CharacterInfoArray m_characterInfoArray;
	NpcInfoArray m_npcInfoArray;
	NpcGroupArray m_npcGroupArray;
	SkillTableArray m_skillTableArray;
	SkillBookTableArray m_skillBookTableArray;

	GossipTableArray m_gossipTableArray;
	GossipOptionTableArray m_gossipOptionTableArray;

	ShopTableArray m_shopTableArray;
	ShopTableItemArray m_shopTableItemArray;
};

#define sObjMgr ObjectMgr::instance()
#endif