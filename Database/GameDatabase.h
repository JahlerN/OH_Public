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
typedef CSTLMap<_LEVEL_DATA> LevelDataArray;

typedef CSTLMap<MAP> ZoneTableArray;

typedef CSTLMap<_ZONECHANGE_DATA> ZoneChangeTableArray;
typedef CSTLMap<_CHARACTER_DATA> CharacterInfoArray;

class GameDatabaseConnection : public MySQLConnection
{
public:
	GameDatabaseConnection() : MySQLConnection() { }
};

extern GameDatabaseConnection GameDatabase;

#endif