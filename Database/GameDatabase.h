#ifndef GAMEDATABASE_H
#define GAMEDATABASE_H

#include "MySQLConnection.h"
#include "..\OpenHero++\ItemTemplate.h"
#include "..\SharedFiles\globals.h"
#include "..\OpenHero++\MAP.h"

typedef CSTLMap<_SET_BONUS> SetBonusArray;
typedef CSTLMap<_ITEM_TABLE> ItemTableArray;
typedef CSTLMap<_ITEM_DROP_TABLE> ItemDropTableArray;
typedef CSTLMap<_GAMBLING_ITEM_TABLE> GamblingItemTableArray;
typedef CSTLMap<_MAKE_ITEM_TABLE> MakeItemTableArray;
typedef CSTLMap<_MAKE_ITEM_FUSION_TABLE> MakeItemFusionTableArray;
typedef CSTLMap<_DISMANTLE_ITEM> DismantleItemTableArray;
typedef CSTLMap<_LEVEL_DATA> LevelDataArray;

typedef CSTLMap<MAP> ZoneTableArray;

typedef CSTLMap<_ZONECHANGE_DATA> ZoneChangeTableArray;
typedef CSTLMap<_ZONESTART_POSITION> ZoneStartTableArray;
typedef CSTLMap<_CHARACTER_DATA> CharacterInfoArray;
typedef CSTLMap<_NPC_DATA> NpcInfoArray;
typedef CSTLMap<_NPC_GROUP> NpcGroupArray;
typedef CSTLMap<_SKILL_DATA> SkillTableArray;
typedef CSTLMap<_SKILL_BOOK_DATA> SkillBookTableArray;

typedef CSTLMap<_NPC_GOSSIP> GossipTableArray;
typedef CSTLMap<_GOSSIP_OPTION> GossipOptionTableArray;

typedef CSTLMap<_SHOP_TABLE> ShopTableArray;
typedef CSTLMap<_SHOP_TABLE_ITEM> ShopTableItemArray;

class GameDatabaseConnection : public MySQLConnection
{
public:
	GameDatabaseConnection() : MySQLConnection() { }
};

extern GameDatabaseConnection GameDatabase;

#endif