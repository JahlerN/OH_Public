#ifndef DBAGENT_H
#define DBAGENT_H

#include "../SharedFiles/OdbcConnection.h"
#include "MAP.h"
#include "SkillBookTemplate.h"
#include "User.h"


typedef std::vector<_USER_DATA*> UserDataArray;
//typedef CSTLMap<_CHARACTER_DATA> CharacterInfoArray;
typedef CSTLMap<_NPC_DATA> NpcInfoArray;
typedef CSTLMap<_NPC_GROUP> NpcTableArray;
typedef CSTLMap<_SKILL_DATA> SkillTableArray;
typedef CSTLMap<_SKILL_BOOK_DATA> SkillBookTableArray;
typedef CSTLMap<_ZONESTART_POSITION> ZoneStartTableArray;

//typedef CSTLMap<_SET_BONUS> SetBonusArray;
//typedef CSTLMap<_ITEM_TABLE> ItemTableArray;
//typedef CSTLMap<_ITEM_DROP_TABLE> ItemDropTableArray;
//typedef CSTLMap<_GAMBLING_ITEM_TABLE> GamblingItemTableArray;
typedef CSTLMap<_LEVEL_DATA> LevelDataArray;

class CDBAgent
{
public:
	CDBAgent() {}

	bool Connect();
	//bool LoadItemTable();

	/* LOAD SERVER TABLES*/
	//std::vector<_SERVER_TAB*> LoadServerTables();
	//bool LoadItemTable(ItemTableArray& out);
	//bool LoadItemDropTable(ItemDropTableArray& out);
	//bool LoadGamblingItemTable(GamblingItemTableArray& out);
	//bool LoadSetBonusTable(SetBonusArray& out);
	//bool LoadMapTable(ZoneTableArray& out, uint8& totalMap);
	//bool LoadZoneChangeTable(ZoneChangeTableArray& out);
	bool LoadLevelData(LevelDataArray& out);
	//bool LoadCharacterInfoTable(CharacterInfoArray& out);
	bool LoadNpcInfoTable(NpcInfoArray& out);
	bool LoadNpcTable(NpcTableArray& out);
	bool LoadSkillTable(SkillTableArray& out);
	bool LoadSkillBookTable(SkillBookTableArray& out);
	bool LoadZoneStartTable(ZoneStartTableArray& out);

	//bool LoadLevelTable(LevelTableArray out);

	//void MUserInit(uint16 uid);
	//_USER_DATA* GetUser(uint16 uid);

	int8 AccountLogin(std::string& accountId, std::string& password);
	bool GetAccountSession(CUser* pUser);
	bool GetAllCharByAccount(std::string& accountId, uint8& count, std::vector<std::string>& charIds);//std::string& charId1, std::string& chaarId2, std::string& charId3, std::string& charId4, std::string& charId5, std::string& charId6);
	void UpdateAccountFaction(std::string& accountId, uint8& faction);
	void LoadCharInfo(std::string& charId, uint8& index, ByteBuffer& result);
	int8 GetCharNameByUniqueId(std::string& accountId, uint16& id, char& out);

	int8 CreateCharacter(std::string& accountId, uint8& faction, std::string& charId, uint8 charType, uint8 height, uint32 hairType, uint32 faceType, bool updateFaction);//TODO: Consider if index is actually required here. faction?
	int8 DeleteCharacter(std::string& accountId, std::string& charId);

	bool LoadUserData(std::string& accountId, std::string& charId, _USER_DATA* pUser);
	bool LoadWarehouseData(std::string& accountId, _USER_DATA* pUser);
	bool LoadPremiumServiceUser(std::string& accountId, _USER_DATA* pUser);//TODO: I probably will just have this part as a flag or something, too much code for just the premium shit.
	bool SetLoginInfo(std::string& accountId, std::string& charId, std::string& serverIp, short servNum, std::string& clientIp);

	bool LoadWebItemMall(short uid, Packet& result);
	void SaveSkillShortcut(short uid, Packet& result);

	void RequestFriendList(short uid, std::vector<std::string>& friendList);
	//FriendAddResult AddFriend(short sid, short tid);
	//FriendRemoveResult RemoveFriend(short sid, std::string& charId);

	//bool UpdateUser(srd::string& charId, short uid, UserUpdateType t);
	//bool UpdateWarehouseData(std::string& accountId, short uid, UserUpdateType t);

	int8 CreateClan(uint16 clanId, uint8 faction, std::string& name, std::string owner, uint8 bFlag = 1);
	int UpdateClan(uint8 bType, std::string charId, uint16 clanId, uint8 bDomination);
	int DeleteClan(uint16 clanId);

	uint16 LoadAllClanMembers(uint16 clanId, Packet& result);
	void LoadClanInfo(uint16 clanId, Packet& result);
	//void LoadAllClans(uint8 faction);
	bool UpdateClanSymbol(uint16 canId, uint16 symbolSize, char* symbol);

	void AccountLogout(std::string& accountId);

	void UpdateServerUserCount(uint8 serverTab, uint8 serverId, uint16 userCount);
	void UpdateAllServerUserCount(uint16 userCount);

	void SaveUserInventory(_USER_DATA* pUser);
	void SaveUser(_USER_DATA* pUser);


	OdbcConnection m_GameDB, m_AccountDB;
private:
	UserDataArray m_userDataArray;
	//ItemTableArray m_itemTableArray;
};
//TODO: This isn't realy an optimal solution, but i don't want to bother with stored procedures right now.
//#define SQL_INS_CHARACTER "SELECT CASE @TEMP := (SELECT characterId FROM ohdb.characters WHERE characterId = ?) WHEN @TEMP=? THEN 1 ELSE (INSERT INTO ohdb.characters (accountId, characterId, charType, height, hairType, faceType) VALUES (?, ?, %d, %d, %d, %d)) END;"
//#define SQL_INS_CHARACTER "SELECT CASE WHEN (SELECT COUNT(*) FROM ohdb.characters WHERE characterId = ?) > 0 THEN 1 ELSE (INSERT INTO ohdb.characters (accountId, characterId, charType, height, hairType, faceType) VALUES (?, ?, %d, %d, %d, %d)) END;"
#define SQL_INS_CHARACTER "INSERT INTO ohdb.characters (accountId, characterId, charType, height, hairType, faceType, zone, curHp, curChi, str, dex, inte, lastPosX, lastPosZ) VALUES (?, ?, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);"

#define SQL_SEL_SERVERTABS "SELECT tabId, tabName, publicIp, port FROM ohdb.servertabs;"
#define SQL_SEL_SERVERINFO "SELECT serverId, serverName, currentPlayers, maxPlayers, serverStatus FROM ohdb.servertable WHERE tabId = %d;"
#define SQL_SEL_ALL_CHARACTERID "SELECT characterId FROM ohdb.characters WHERE accountId = ? ORDER BY entry ASC;"
#define SQL_SEL_CHARACTERID "SELECT COUNT(*) FROM ohdb.characters WHERE characterId = ?;"
#define SQL_SEL_ACCOUNT_SESSION "SELECT faction, lastIp, serverTab, serverid FROM ohdb.account WHERE accountId = ?;"
#define SQL_SEL_CHARACTER "SELECT entry, charType, height, hairType, faceType, level, zone, class, showHelm, showMask, showHtItems, inventory FROM ohdb.characters WHERE characterId = ?;"
#define SQL_SEL_CHARID_BY_UNIQUEID "SELECT characterId FROM ohdb.characters WHERE accountId = ? AND entry = %d;"
#define SQL_SEL_USER_DATA "SELECT entry, charType, height, hairType, faceType, level, exp, zone, inventory, skillBookData, class, clanId, fame, injury, curHp, curChi, str, dex, inte, wind, water, fire, skillPointsSpent, gold, lastPosX, lastPosZ, lastPosY, skillTabs, questCount, quests, showHelm, showMask, showHtItems, activeWeapon FROM ohdb.characters WHERE accountId = ? AND characterId = ?;"
#define SQL_SEL_WAREHOUSEDATA "SELECT gold, warehouseData FROM ohdb.warehouse WHERE accountId = ?;"
#define SQL_SEL_ITEMDATA "SELECT itemId, name, upgradeCode, holeText, relatedItem, nextItemStage, itemType, itemRarity, setBonusId, buyPrice, sellPrice, equipPos, reqClass, levelReq, maxLevel, strReq, dexReq, intReq, headDefense, bodyDefense, footDefense, minDamage, maxDamage, attackSpeed, attackRange, attackAoE, str, dex, inte, wind, water, fire, poisonDamage, poisonDefense, confusionDamage, confusionDefense, paralysisDamage, paralysisDefense, maxHp, hpRecoveryPercent, maxChi, chiRecoveryPercent, movementSpeed, minBonusDamage, maxBonusDamage, damagePercent, minSkillDamage, maxSkillDamage, skillDamagePercent, defense, defensePercent, artsDefense, artsDefensePercent, accuracy, dodge, hpRecovery, chiRecovery, critChance, bonusExp, itemDropBonus, bonusMineral FROM world.itemdata;"
#define SQL_SEL_ITEM_SET "SELECT setId, itemCount, setPart1, setPart2, setPart3, setPart4, setPart5, setPart6, setPart7, setPart8, setPart9, bonus1, bonus2, bonus3, bonus4, bonus5, bonus6, bonus7, bonusExtra1, bonusExtra2, bonusExtra3, bonusExtra4, bonusExtra5 FROM world.itemset;"
#define SQL_SEL_ITEMDROPDATA "SELECT dropId, dropable1, dropable2, dropable3, dropable4, dropable5, dropable6, dropable7, dropable8, dropable9, dropable10, dropable11, dropable12, dropable13, dropable14, dropable15, dropable16, dropable17, dropable18, dropable19, dropable20, dropableChance1, dropableChance2, dropableChance3, dropableChance4, dropableChance5, dropableChance6, dropableChance7, dropableChance8, dropableChance9, dropableChance10, dropableChance11, dropableChance12, dropableChance13, dropableChance14, dropableChance15, dropableChance16, dropableChance17, dropableChance18, dropableChance19, dropableChance20 FROM world.itemdropdata;"
#define SQL_SEL_GAMBLINGITEM "SELECT itemId, costToOpen, dropId FROM world.gamblingitem;"
#define SQL_SEL_ZONEDATA "SELECT zoneId, name, smd FROM world.zonedata;" //Server num was in there
#define SQL_SEL_ZONECHANGEDATA "SELECT zoneChangeId, toZoneId, posX, posY, reqLevel FROM world.zonechangedata;"
#define SQL_SEL_ZONESTARTDATA "SELECT zoneId, x, z, y, rangeX, rangeZ, rangeY FROM world.zonestartposition"
#define SQL_SEL_LEVELDATA "SELECT level, expReq, statPoint, statElement, unk2, unk3 FROM world.leveldata;"
#define SQL_SEL_CHARDATA "SELECT charTypeId, charEvolution, baseHp, baseChi, baseStr, baseDex, baseInt, hpPerLevel, chiPerLevel, dmgPerLevel, sdmgPerLevel, defPerLevel, sdefPerLevel FROM world.characterdata;"
#define SQL_SEL_NPCDATA "SELECT npcId, name, phrase, npcType, level, exp, divineExp, darknessExp, lootRolls, dropId, maxHp, maxChi, unkUse, gold, minDmg, maxDmg, minSkillDmg, maxSkillDmg FROM world.npcdata;"
#define SQL_SEL_NPC "SELECT id, zoneId, npcId, rotation, posX1, posZ1, posX2, posZ2, npcCountInField, spawnTime FROM ohdb.npc;"
#define SQL_SEL_SKILLBOOKDATA "SELECT skillBookId, name, bookType, classReq, weaponType, levelReq, skill1, skill2, skill3, skill4, skill5, skill6, skill7, skill8, skill9, skill10, skill11, skill12, skill13, skill14, skill15, skill16, skill17, skill18, skill19, skill20, skill21, skill22, skill23, skill24 FROM world.skillbookdata;"
#define SQL_SEL_SKILLDATA "SELECT skillId, name, description, skillType, statType, aoeType, pierceType, maxLevel, slot, unlockSlot1, unlockSlot2, levelForUnlock1, levelForUnlock2, duration, durationPerLevel, chiUsage, chiUsagePerLevel, chiUsagePerSecond, chiUsagePerSecondPerLevel, minDmg, minDmgPerLevel, maxDmg, maxDmgPerLevel, `range`, rangePerLevel, aoe, aoePerLevel, coolDown, baseStatBonus, statBonusPerLevel, poisonDmg, poisonDmgPerLevel, posionHeal, poisonHealPerLevel, confDmg, confDmgPerLevel, confHeal, confHealPerLevel, paralysisDmg, paralysisDmgPerLevel, paralysisHeal, paralysisHealPerLevel, minDmgBonus, minDmgBonusPerLevel, maxDmgBonus, maxDmgBonus, attackPower, attackPowerPerLevel, defensivePower, defensivePowerPerLevel, injury, injuryPerLevel, accuracy, accuracyPerLevel, dodge, dodgePerLevel, defenseBonus, defenseBonusPerLevel, movementSpeed, movementSpeedPerLevel, minHpChange, minHpChangePerLevel, maxHpChange, maxHpChangePerLevel, minChiChange, minChiChangePerLevel, maxChiChange, maxChiChangePerLevel, healPerSecond, healPerSecondPerLevel, percentHp, percentPerLevel, percentChi, percentChiPerLevel, reflectChance, reflectChancePerLevel, reflectPercent, reflectPercentPerLevel, skillBookId FROM world.skilldata;"

#define SQL_DEL_CHARACTER "DELETE FROM ohdb.characters WHERE accountId = ? AND characterId = ?;"

#define SQL_UPD_ALL_SERVER_USER_COUNT "UPDATE ohdb.servertable SET currentPlayers = %d;"
#define SQL_UPD_SERVER_USER_COUNT "UPDATE ohdb.servertable SET currentPlayers = %d WHERE tabId = %d AND serverId = %d;"
#define SQL_UPD_ACCOUNTFACTION "UPDATE ohdb.account SET faction = %d WHERE accountId = ?;" //ONLY EVER USED ON FIRST CHAR CREATION!
#define SQL_UPDATE_INVENTORY "UPDATE ohdb.characters SET inventory = ? WHERE characterId = ?;"
#define SQL_UPDATE_CHARACTER "UPDATE ohdb.characters SET charType = %d, level = %d, exp = %lld, zone = %d, skillBookData = ?, class = %d, clanId = %d, fame = %d, injury = %d, curHp = %d, curChi = %d, str = %d, dex = %d, inte = %d, wind = %d, water = %d, fire = %d, skillPointsSpent = %d, gold = %lld, lastPosX = %f, lastPosZ = %f, lastPosY = %f, showHelm = %d, showMask = %d, showHtItems = %d, activeWeapon = %d WHERE characterId = ?;"
#endif