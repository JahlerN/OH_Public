#include "stdafx.h"
//#include "ObjectMgr.h"
//#include "DBAgent.h"

ObjectMgr* ObjectMgr::instance()
{
	static ObjectMgr instance;
	return &instance;
}

std::vector<_SERVER_TAB*> ObjectMgr::LoadServerTables()
{
	std::vector<_SERVER_TAB*> serverVec;
	QueryResult res = AccountDatabase.ExecuteQuery(SQL_SEL_SERVERTABS);

	if (res.IsEmpty())
		return serverVec;

	uint32 startTime = GetMSTime();

	_SERVER_TAB* pTab = NULL;
	_SERVER_INFO* pInfo = NULL;

	do
	{
		pTab = new _SERVER_TAB();

		pTab->m_tabId = res.GetUint8();
		pTab->m_tabName = res.GetString();
		pTab->m_serverIp = res.GetString();
		pTab->m_port = res.GetUint16();

		QueryResult res2 = AccountDatabase.ExecuteQuery(string_format(SQL_SEL_SERVERINFO, pTab->m_tabId).c_str());

		if (res2.IsEmpty())
			return serverVec;

		do
		{
			pInfo = new _SERVER_INFO();

			pInfo->m_tabId = pTab->m_tabId;
			pInfo->m_serverId = res2.GetUint8();
			pInfo->m_serverName = res2.GetString();
			pInfo->m_serverName.append(string_format("\x20%d", pInfo->m_serverId + 1));
			pInfo->m_curPlayers = res2.GetUint16();
			pInfo->m_maxPlayers = res2.GetUint16();
			pInfo->m_serverStatus = (ServerStatus)res2.GetUint8();

			pTab->m_serverInfoArr.push_back(pInfo);
		} while (res2.NextRow());
		serverVec.push_back(pTab);
	} while (res.NextRow());

	return serverVec;
}

bool ObjectMgr::LoadMapTables()
{
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_ZONEDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_ZONE_INFO* pZone = NULL;
	MAP* pMap = NULL;

	do
	{
		pZone = new _ZONE_INFO();

		pZone->m_zoneNum = res.GetUint32();
		pZone->m_mapName = res.GetString();
		pZone->m_smdFile = res.GetString();

		pMap = ReadMapFile(pZone);
		if (pMap == NULL)
		{
			m_zoneTableArray.DeleteAllData();
			return false;
		}
		m_zoneTableArray.PutData(pMap->m_zoneNum, pMap);
	} while (res.NextRow());

	printf("Loaded %d maps in %ums\n", m_zoneTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

MAP* ObjectMgr::ReadMapFile(_ZONE_INFO* pZone)
{
	HANDLE hFile;
	std::string fullPath;

	MAP* pMap = new MAP();
	pMap->Initialize(pZone);
	delete pZone;

	fullPath = string_format("..\\Debug\\MAP\\%s", pMap->m_smdFile.c_str());
	//fout.open(fullPath, ios::out);
	//FILE* fp = fopen(fullPath.c_str(), "rb");
	hFile = CreateFile(fullPath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE || !pMap->LoadMap(hFile))//!file.Open(fullPath, CFile::modeRead) || pMap->LoadMap((HANDLE)file.m_hFile))
	{
		printf("\nUnable to load map file: %s.\n", pMap->m_mapName.c_str());
		return NULL;
	}
	//fout.close();
	CloseHandle(hFile);

	if (pMap->m_roomEvent > 0)
	{
		//TODO: Handle room events, not yet implemented
	}

	return pMap;
}

bool ObjectMgr::LoadItemSet()
{
	m_setBonusArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_ITEM_SET);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_SET_BONUS* pSet = NULL;

	do
	{
		pSet = new _SET_BONUS();

		pSet->m_setId = res.GetUint32();
		pSet->m_itemCount = res.GetUint8();
		for (int i = 0; i < MAX_SET_PARTS; i++)
			pSet->m_setPart[i] = res.GetUint32();
		for (int i = 0; i < MAX_PARTIAL_SET_BONUS; i++)
			pSet->m_bonus[i] = res.GetUint16();
		for (int i = 0; i < MAX_EXTRA_PARTIAL_SET_BONUS; i++)
			pSet->m_bonusExtra[i] = res.GetUint16();
		m_setBonusArray.PutData(pSet->m_setId, pSet);
	} while (res.NextRow());

	printf("\nLoaded %u item sets in %ums\n", m_setBonusArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadItemTemplate()
{
	m_itemTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_ITEMDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_ITEM_TABLE* pTable = NULL;

	do
	{
		pTable = new _ITEM_TABLE();

		pTable->m_itemId = res.GetUint32();
		pTable->m_name = res.GetString();
		pTable->m_upgradeCode = res.GetString();
		pTable->m_upgradeDescription = res.GetString();
		pTable->m_itemRelation = res.GetUint32();
		pTable->m_nextItemStage = res.GetUint32();
		pTable->m_itemType = res.GetUint8();
		pTable->m_itemRarity = res.GetUint8();
		pTable->m_setBonus = GetSetBonus(res.GetUint32());
		pTable->m_extendedPrice = res.GetUint32();
		pTable->m_buyPrice = res.GetUint32();
		pTable->m_sellPrice = res.GetUint32();
		pTable->m_equipPos = res.GetUint16();
		pTable->m_reqClass = res.GetUint8();
		pTable->m_reqLevel = res.GetUint32();
		pTable->m_maxLevel = res.GetUint32();
		pTable->m_strReq = res.GetUint16();
		pTable->m_dexReq = res.GetUint16();
		pTable->m_intReq = res.GetUint16();
		pTable->m_headDefense = res.GetUint32();
		pTable->m_bodyDefense = res.GetUint32();
		pTable->m_footDefense = res.GetUint32();
		pTable->m_damageMin = res.GetUint16();
		pTable->m_damageMax = res.GetUint16();
		pTable->m_delay = res.GetUint16() / 100;
		pTable->m_attackRange = res.GetFloat();
		pTable->m_attackAoE = res.GetFloat();
		for (int i = 0; i < STAT_END; i++)
			pTable->m_statBonus[i] = res.GetUint16();
		pTable->m_poisonDamage = res.GetUint8();
		pTable->m_poisonDefense = res.GetUint8();
		pTable->m_confusionDamage = res.GetUint8();
		pTable->m_confusionDefense = res.GetUint8();
		pTable->m_paralysisDamage = res.GetUint8();
		pTable->m_paralysisDefense = res.GetUint8();
		pTable->m_maxHp = res.GetUint16();
		pTable->m_hpRecoveryPercent = res.GetUint16();
		pTable->m_maxChi = res.GetUint16();
		pTable->m_chiRecoveryPercent = res.GetUint16();
		pTable->m_movementSpeed = res.GetFloat();
		pTable->m_minBonusDamage = res.GetUint16();
		pTable->m_maxBonusDamage = res.GetUint16();
		pTable->m_damagePercent = res.GetUint8();
		pTable->m_minSkillDamageBonus = res.GetUint16();
		pTable->m_maxSkillDamageBonus = res.GetUint16();
		pTable->m_skillDamagePercent = res.GetUint8();
		pTable->m_defense = res.GetUint32();
		pTable->m_defensePercent = res.GetUint8();
		pTable->m_artsDefense = res.GetUint32();
		pTable->m_artsDefensePercent = res.GetUint8();
		pTable->m_accuracy = res.GetUint16();
		pTable->m_dodge = res.GetUint16();
		pTable->m_hpRecovery = res.GetInt16();
		pTable->m_chiRecovery = res.GetInt16();
		pTable->m_critChance = res.GetUint8();
		pTable->m_bonusExp = res.GetUint8();
		pTable->m_itemDropBonus = res.GetUint16();
		pTable->m_bonusMineral = res.GetUint16();
		pTable->m_stackable = pTable->m_itemType > 148 ? true : false; //TODO: This isn't how it's supposed to be done, it's just temporary to fix most items.
		m_itemTableArray.PutData(pTable->m_itemId, pTable);
	} while (res.NextRow());

	printf("Loaded %u item tables in %ums\n", m_itemTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadItemDropTable()
{
	m_dropTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_ITEMDROPDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_ITEM_DROP_TABLE* pTable = NULL;

	do
	{
		pTable = new _ITEM_DROP_TABLE();

		pTable->m_maxRoll = 0;
		pTable->m_dropId = res.GetUint32();
		uint8 availableDrops = 0;
		for (; availableDrops < NUM_DROPS_IN_DROP_TABLE; availableDrops++)
			pTable->m_dropableId[availableDrops] = res.GetUint32();
		for (int i = 0; i < NUM_DROPS_IN_DROP_TABLE; i++)
		{
			pTable->m_dropableChance[i] = res.GetUint16();
			if (pTable->m_dropableChance[i] > pTable->m_maxRoll)
				pTable->m_maxRoll = pTable->m_dropableChance[i];
		}
		m_dropTableArray.PutData(pTable->m_dropId, pTable);
	} while (res.NextRow());

	printf("Loaded %u item drop tables in %ums\n", m_dropTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadGamblingItemTable()
{
	m_gamblingItemArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_GAMBLINGITEM);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_GAMBLING_ITEM_TABLE* pTable = NULL;

	do
	{
		pTable = new _GAMBLING_ITEM_TABLE();

		pTable->m_itemId = res.GetUint32();
		pTable->m_openingCost = res.GetUint32();
		pTable->m_dropTable = sObjMgr->GetItemDropTable(res.GetUint32());

		if (pTable->m_dropTable != NULL)
			m_gamblingItemArray.PutData(pTable->m_itemId, pTable);
	} while (res.NextRow());

	printf("Loaded %u gambling item tables in %ums\n", m_gamblingItemArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadMakeItemTable()
{
	m_makeItemTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_MAKEITEMTABLE);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_MAKE_ITEM_TABLE* pTable = NULL;

	do
	{
		pTable = new _MAKE_ITEM_TABLE();

		pTable->itemId = res.GetUint32();
		pTable->reqItemId1 = res.GetUint32();
		pTable->reqItemCount1 = res.GetUint16();
		pTable->reqItemId2 = res.GetUint32();
		pTable->reqItemCount2 = res.GetUint16();
		pTable->reqItemId3 = res.GetUint32();
		pTable->successRate = res.GetUint16();
		pTable->reqGold = res.GetUint32();
		pTable->resultItem = res.GetUint32();

		m_makeItemTableArray.PutData(pTable->itemId, pTable);
	} while (res.NextRow());

	printf("Loaded %u recipes for item making in %ums\n", m_makeItemTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadMakeItemFusionTable()
{
	m_makeItemTableFusionArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_MAKEITEMFUSIONTABLE);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_MAKE_ITEM_FUSION_TABLE* pTable = NULL;

	do
	{
		pTable = new _MAKE_ITEM_FUSION_TABLE();

		pTable->itemId = res.GetUint32();
		pTable->reqItemId1 = res.GetUint32();
		pTable->reqItemCount1 = res.GetUint16();
		pTable->reqItemId2 = res.GetUint32();
		pTable->reqItemCount2 = res.GetUint16();
		pTable->reqItemId3 = res.GetUint32();
		pTable->successRate = res.GetUint16();
		pTable->reqGold = res.GetUint32();
		pTable->resultItem = res.GetUint32();

		m_makeItemTableFusionArray.PutData(pTable->itemId, pTable);
	} while (res.NextRow());

	printf("Loaded %u recipes for item fusion in %ums\n", m_makeItemTableFusionArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadDismantleItemTable()
{
	m_dismantleitemTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_DISMANTLE_ITEM_TABLE);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_DISMANTLE_ITEM* pTable = NULL;

	do
	{
		pTable = new _DISMANTLE_ITEM();

		pTable->dismantleItemId = res.GetUint32();
		for (int i = 0; i < MAX_DISMANTLE_RESULTS - 1; i++)
			pTable->rewardItem[i] = res.GetUint32();
		for (int i = 0; i < MAX_DISMANTLE_RESULTS - 1; i++)
			pTable->rewardMaxCount[i] = res.GetUint16();

		pTable->chanceForReward = res.GetUint16();
		pTable->goldReq = res.GetUint32();
		pTable->specialReward = res.GetUint32();
		pTable->specialRewardChance = res.GetUint16();

		m_dismantleitemTableArray.PutData(pTable->dismantleItemId, pTable);
	} while (res.NextRow());

	printf("Loaded %u results for item dismantling in %ums\n", m_dismantleitemTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadLevelData()
{
	m_levelDataArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_LEVELDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_LEVEL_DATA* pTable = NULL;

	do
	{
		pTable = new _LEVEL_DATA();

		pTable->m_level = res.GetUint32();
		pTable->m_expReq = res.GetUint64();
		pTable->m_statPoint = res.GetUint16();
		pTable->m_statElement = res.GetUint16();
		pTable->m_unk2 = res.GetUint8();
		pTable->m_unk3 = res.GetUint8();

		m_levelDataArray.PutData(pTable->m_level, pTable);
	} while (res.NextRow());

	printf("Loaded level data for %u levels in %ums\n", m_levelDataArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadZoneChangeTable()
{
	m_zoneChangeArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_ZONECHANGEDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_ZONECHANGE_DATA* pTable = NULL;

	do
	{
		pTable = new _ZONECHANGE_DATA();

		pTable->m_zoneChangeId = res.GetUint32();
		pTable->m_toZoneId = res.GetUint8();
		pTable->m_posX = res.GetFloat();
		pTable->m_posZ = res.GetFloat();
		pTable->m_reqLevel = res.GetUint32();

		m_zoneChangeArray.PutData(pTable->m_toZoneId, pTable);
	} while (res.NextRow());

	printf("Loaded [NOTE: Some doesn't exist and some are wrong] %d rows of zone change data in %ums\n", m_zoneChangeArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadZoneStartPositionTable()
{
	m_zoneStartTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_ZONESTARTDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_ZONESTART_POSITION* pTable = NULL;

	do
	{
		pTable = new _ZONESTART_POSITION();

		pTable->m_zoneId = res.GetUint32();
		pTable->m_x = res.GetFloat();
		pTable->m_z = res.GetFloat();
		pTable->m_y = res.GetFloat();
		pTable->m_rangeX = res.GetUint8();
		pTable->m_rangeZ = res.GetUint8();
		pTable->m_rangeY = res.GetUint8();

		m_zoneStartTableArray.PutData(pTable->m_zoneId, pTable);
	} while (res.NextRow());

	printf("Loaded zone start data for %d zones in %ums\n", m_zoneStartTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadCharacterInfoTable()
{
	m_characterInfoArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_CHARDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_CHARACTER_DATA* pTable = NULL;

	do
	{
		pTable = new _CHARACTER_DATA();

		pTable->m_typeId = res.GetUint32();
		pTable->m_nextTypeId = res.GetUint8();
		pTable->m_baseHp = res.GetUint32();
		pTable->m_baseChi = res.GetUint32();
		pTable->m_baseStr = res.GetUint32();
		pTable->m_baseDex = res.GetUint32();
		pTable->m_baseInt = res.GetUint32();
		pTable->m_hpPerLevel= res.GetUint8();
		pTable->m_chiPerLevel = res.GetUint8();
		pTable->m_dmgPerLevel = res.GetFloat();
		pTable->m_sdmgPerLevel = res.GetFloat();
		pTable->m_defPerLevel = res.GetFloat();
		pTable->m_sdefPerLevel = res.GetFloat();

		m_characterInfoArray.PutData(pTable->m_typeId, pTable);
	} while (res.NextRow());

	printf("Loaded base info about %d character types in %ums\n.", m_characterInfoArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadNpcGossipTable()
{
	m_gossipTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_NPCGOSSIP);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_NPC_GOSSIP* pTable = NULL;

	do
	{
		pTable = new _NPC_GOSSIP();

		pTable->gossipId = res.GetUint32();
		pTable->npcSayId = res.GetUint32();
		pTable->npcId = res.GetUint32();
		pTable->optionCount = 0;
		for (; pTable->optionCount < MAX_GOSSIP_OPTIONS; pTable->optionCount++)
		{
			pTable->option[pTable->optionCount] = res.GetUint32();
			pTable->response[pTable->optionCount] = res.GetUint32();
			if (pTable->option[pTable->optionCount] == 0)
				break;
		}

		m_gossipTableArray.PutData(pTable->gossipId, pTable);
	} while (res.NextRow());

	printf("Loaded %d gossip tables in %ums\n", m_gossipTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadGossipOptionTable()
{
	m_gossipOptionTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_GOSSIP_RESPONSE_LINK);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_GOSSIP_OPTION* pTable = NULL;

	do
	{
		pTable = new _GOSSIP_OPTION();

		pTable->optionId = res.GetUint32();
		pTable->gossipFlag = (GossipFlag)res.GetUint8();
		pTable->goldCost = res.GetUint32();
		pTable->toZone = res.GetUint8();

		m_gossipOptionTableArray.PutData(pTable->optionId, pTable);
	} while (res.NextRow());

	printf("Loaded %d gossip options in %ums\n", m_gossipOptionTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadNpcInfoTable()
{
	m_npcInfoArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_NPCDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_NPC_DATA* pTable = NULL;

	do 
	{
		pTable = new _NPC_DATA();

		pTable->m_npcId = res.GetUint32();
		pTable->m_name = res.GetString();
		pTable->m_phrase = res.GetString();
		pTable->m_npcType = res.GetUint8();
		pTable->m_level = res.GetUint32();
		pTable->m_exp = res.GetUint32();
		pTable->m_divineExp = res.GetUint32();
		pTable->m_darknessExp = res.GetUint32();
		pTable->m_lootRolls = res.GetUint8();
		pTable->m_dropId = res.GetUint32();
		pTable->m_maxHp = res.GetUint32();
		pTable->m_maxChi = res.GetUint32();
		pTable->m_unkUse = res.GetUint16();
		pTable->m_gold = res.GetUint32();
		pTable->m_gossipId = res.GetUint32();
		pTable->m_minDmg = res.GetUint32();
		pTable->m_maxDmg = res.GetUint32();
		pTable->m_minSkillDmg = res.GetUint32();
		pTable->m_maxSkillDmg = res.GetUint32();

		m_npcInfoArray.PutData(pTable->m_npcId, pTable);
	} while (res.NextRow());

	printf("Loaded info about %d npc types in %ums\n", m_npcInfoArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadNpcGroupTable()
{
	m_npcGroupArray.DeleteAllData();
	QueryResult res = AccountDatabase.ExecuteQuery(SQL_SEL_NPC);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_NPC_GROUP* pTable = NULL;

	do
	{
		pTable = new _NPC_GROUP();

		pTable->m_npcGroupId = res.GetUint32();
		pTable->m_zoneId = res.GetUint32();//Idk why it's this big in the DB, never over 255 tho
		pTable->m_npcId = res.GetUint32();
		pTable->m_rotation = res.GetFloat();
		pTable->m_minX = res.GetFloat();
		pTable->m_minZ = res.GetFloat();
		pTable->m_maxX = res.GetFloat();
		pTable->m_maxZ = res.GetFloat();
		pTable->m_npcsInGroup = res.GetUint8();
		pTable->m_respawnTime = res.GetUint32();

		m_npcGroupArray.PutData(pTable->m_npcGroupId, pTable);
	} while (res.NextRow());

	printf("Loaded info about %d npc groups in %ums\n", m_npcGroupArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadSkillTable()
{
	m_skillTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_SKILLDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_SKILL_DATA* pTable = NULL;

	do
	{
		pTable = new _SKILL_DATA();

		pTable->m_skillId = res.GetUint32();
		pTable->m_name = res.GetString();
		pTable->m_description = res.GetString();
		pTable->m_skillType = res.GetUint8();
		pTable->m_statType = res.GetUint8();
		pTable->m_aoeType = (AoeType)res.GetUint8();
		pTable->m_pierceType = res.GetUint8();
		pTable->m_maxLevel = res.GetUint8();
		pTable->m_slot = res.GetUint8();
		pTable->m_levelForUnlock1 = res.GetUint8();
		pTable->m_levelForUnlock2 = res.GetUint8();
		pTable->m_levelForUnlock1 = res.GetUint8();
		pTable->m_levelForUnlock2 = res.GetUint8();
		pTable->m_duration = res.GetUint32();
		pTable->m_durationPerLevel = res.GetUint32();
		pTable->m_chiUsage = res.GetUint16();
		pTable->m_chiUsagePerLevel = res.GetFloat();
		pTable->m_chiUsagePerSecond = res.GetUint16();
		pTable->m_chiUsagePerSecondPerLevel = res.GetFloat();
		pTable->m_minDmg = res.GetUint16();
		pTable->m_minDmgPerLevel = res.GetFloat();
		pTable->m_maxDmg = res.GetUint16();
		pTable->m_maxDmgPerLevel = res.GetFloat();
		pTable->m_range = res.GetFloat();
		pTable->m_rangePerLevel = res.GetFloat();
		pTable->m_aoe = res.GetFloat();
		pTable->m_aoePerLevel = res.GetFloat();
		pTable->m_cooldown = res.GetUint16();
		pTable->m_statTypeBonus = res.GetUint32();
		pTable->m_statTypeBonusPerLevel = res.GetFloat();
		pTable->m_poisonDmg = res.GetUint8();
		pTable->m_poisonDmgPerLevel = res.GetFloat();
		pTable->m_poisonHeal = res.GetUint8();
		pTable->m_poisonHealPerLevel = res.GetFloat();
		pTable->m_confusionDmg = res.GetUint8();
		pTable->m_confusionDmgPerLevel = res.GetFloat();
		pTable->m_confusionHeal = res.GetUint8();
		pTable->m_confusionHealPerLevel = res.GetFloat();
		pTable->m_paralysisDmg = res.GetUint8();
		pTable->m_paralysisDmgPerLevel = res.GetFloat();
		pTable->m_paralysisHeal = res.GetUint8();
		pTable->m_paralysisHealPerLevel = res.GetFloat();
		pTable->m_minDmgBonus = res.GetInt16();
		pTable->m_minDmgBonusPerLevel = res.GetFloat();
		pTable->m_maxDmgBonus = res.GetInt16();
		pTable->m_maxDmgBonusPerLevel = res.GetFloat();
		pTable->m_attackPower = res.GetInt16();
		pTable->m_attackPowerPerLevel = res.GetFloat();
		pTable->m_defenseivePower = res.GetInt16();
		pTable->m_defensivePowerPerLevel = res.GetFloat();
		pTable->m_injury = res.GetUint16();
		pTable->m_injuryPerLevel = res.GetFloat();
		pTable->m_accuracy = res.GetUint16();
		pTable->m_accuracyPerLevel = res.GetFloat();
		pTable->m_dodge = res.GetUint16();
		pTable->m_dodgePerLevel = res.GetFloat();
		pTable->m_defenseBonus = res.GetInt16();
		pTable->m_defenseBonusPerLevel = res.GetFloat();
		pTable->m_movementSpeed = res.GetFloat();
		pTable->m_movementSpeedPerLevel = res.GetFloat();
		pTable->m_minHpChange = res.GetUint16();
		pTable->m_minHpChangePerLevel = res.GetFloat();
		pTable->m_maxHpChange = res.GetUint16();
		pTable->m_maxHpChangePerLevel = res.GetFloat();
		pTable->m_minChiChange = res.GetUint16();
		pTable->m_minChiChangePerLevel = res.GetFloat();
		pTable->m_maxChiChange = res.GetUint16();
		pTable->m_maxChiChangePerLevel = res.GetFloat();
		pTable->m_healPerSecond = res.GetUint16();
		pTable->m_healPerSecondPerLevel = res.GetFloat();
		pTable->m_percentHp = res.GetUint8();
		pTable->m_percentHpPerLevel = res.GetFloat();
		pTable->m_percentChi = res.GetUint8();
		pTable->m_percentChiPerLevel = res.GetFloat();
		pTable->m_reflectChance = res.GetUint16();
		pTable->m_reflectChancePerLevel = res.GetFloat();
		pTable->m_reflectPercent = res.GetUint16();
		pTable->m_reflectPercentPerLevel = res.GetFloat();
		pTable->m_skillBookId = res.GetUint32();

		m_skillTableArray.PutData(pTable->m_skillId, pTable);
	} while (res.NextRow());

	printf("Loaded %d skills in %ums\n", m_skillTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadSkillBookTable()
{
	m_skillBookTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_SKILLBOOKDATA);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_SKILL_BOOK_DATA* pTable = NULL;

	do
	{
		pTable = new _SKILL_BOOK_DATA();

		pTable->m_skillBookId = res.GetUint32();
		pTable->m_name = res.GetString();
		pTable->m_bookType = res.GetUint8();
		pTable->m_classReq = res.GetUint8();
		pTable->m_weaponType = res.GetUint8();
		pTable->m_levelReq = res.GetUint32();

		for (int i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			auto skill = sObjMgr->GetSkillInfo(res.GetUint32());
			pTable->m_skillData[i] = skill;
		}

		m_skillBookTableArray.PutData(pTable->m_skillBookId, pTable);
	} while (res.NextRow());

	printf("Loaded %d skill books in %ums\n", m_skillBookTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadShopTable()
{
	m_shopTableArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_SHOPTABLE);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_SHOP_TABLE* pTable = NULL;

	do
	{
		pTable = new _SHOP_TABLE();

		pTable->shopId = res.GetUint32();
		pTable->shopName = res.GetString();
		for (int i = 0; i < MAX_SHOP_ITEM_TABLES; i++)
			pTable->shopItemTable[i] = res.GetUint32();
		
		m_shopTableArray.PutData(pTable->shopId, pTable);
	} while (res.NextRow());

	printf("Loaded %d shop tables in %ums\n", m_shopTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadShopItemTable()
{
	m_shopTableItemArray.DeleteAllData();
	QueryResult res = GameDatabase.ExecuteQuery(SQL_SEL_SHOPTABLEITEM);

	if (res.IsEmpty())
		return false;

	uint32 startTime = GetMSTime();

	_SHOP_TABLE_ITEM* pTable = NULL;

	do
	{
		pTable = new _SHOP_TABLE_ITEM();

		pTable->shopItemId = res.GetUint32();
		for (int i = 0; i < MAX_SHOP_TABLE_ITEMS; i++)
		{
			pTable->itemIds[i] = res.GetUint32();
			pTable->slot[i] = res.GetUint16();
		}

		m_shopTableItemArray.PutData(pTable->shopItemId, pTable);
	} while (res.NextRow());

	printf("Loaded %d shop item tables in %ums\n", m_shopTableItemArray.GetSize(), GetMSTime() - startTime);
	return true;
}

_SET_BONUS* ObjectMgr::GetSetBonus(uint32 setId)
{
	return m_setBonusArray.GetData(setId);
}

_ITEM_TABLE* ObjectMgr::GetItemTemplate(uint32 itemId)
{
	return m_itemTableArray.GetData(itemId);
}

_ITEM_DROP_TABLE* ObjectMgr::GetItemDropTable(uint32 itemId)
{
	return m_dropTableArray.GetData(itemId);
}

_GAMBLING_ITEM_TABLE* ObjectMgr::GetGamblingItemTable(uint32 id)
{
	return m_gamblingItemArray.GetData(id);
}

_MAKE_ITEM_TABLE* ObjectMgr::GetMakeItemTable(uint32 itemId)
{
	return m_makeItemTableArray.GetData(itemId);
}

_MAKE_ITEM_FUSION_TABLE* ObjectMgr::GetMakeItemFusionTable(uint32 itemId)
{
	return m_makeItemTableFusionArray.GetData(itemId);
}

_DISMANTLE_ITEM* ObjectMgr::GetDismantleItemTable(uint32 itemId)
{
	return m_dismantleitemTableArray.GetData(itemId);
}

_LEVEL_DATA* ObjectMgr::GetLevelData(uint32 level)
{
	return m_levelDataArray.GetData(level);
}

//By zone ID
_ZONECHANGE_DATA* ObjectMgr::GetZoneChangeData(uint8 zoneId)
{
	return m_zoneChangeArray.GetData(zoneId);
}

_ZONESTART_POSITION* ObjectMgr::GetZoneStartPosition(uint8 zoneId)
{
	return m_zoneStartTableArray.GetData(zoneId);
}

_CHARACTER_DATA* ObjectMgr::GetCharacterInfo(uint8 charTypeId)
{
	return m_characterInfoArray.GetData(charTypeId);
}

_NPC_GOSSIP* ObjectMgr::GetNpcGossipInfo(uint32 gossipId)
{
	return m_gossipTableArray.GetData(gossipId);
}

_GOSSIP_OPTION* ObjectMgr::GetGossipOptionInfo(uint32 optionId)
{
	return m_gossipOptionTableArray.GetData(optionId);
}

_NPC_DATA* ObjectMgr::GetNpcInfo(uint32 npcId)
{
	return m_npcInfoArray.GetData(npcId);
}

_NPC_GROUP* ObjectMgr::GetNpcGroupInfo(uint32 npcId)
{
	return m_npcGroupArray.GetData(npcId);
}

_SKILL_DATA* ObjectMgr::GetSkillInfo(uint32 skillId)
{
	return m_skillTableArray.GetData(skillId);
}

_SKILL_BOOK_DATA* ObjectMgr::GetSkillBookInfo(uint32 skillBookId)
{
	return m_skillBookTableArray.GetData(skillBookId);
}

_SHOP_TABLE* ObjectMgr::GetShopTableInfo(uint32 shopId)
{
	return m_shopTableArray.GetData(shopId);
}

_SHOP_TABLE_ITEM* ObjectMgr::GetShopTableItemInfo(uint32 shopItemId)
{
	return m_shopTableItemArray.GetData(shopItemId);
}

ObjectMgr::ObjectMgr()
{

}

ObjectMgr::~ObjectMgr()
{
	m_setBonusArray.DeleteAllData();
	m_itemTableArray.DeleteAllData();
	m_dropTableArray.DeleteAllData();
	m_gamblingItemArray.DeleteAllData();
	m_levelDataArray.DeleteAllData();

	m_zoneChangeArray.DeleteAllData();

	m_zoneChangeArray.DeleteAllData();
	m_characterInfoArray.DeleteAllData();
	m_npcInfoArray.DeleteAllData();
	m_npcGroupArray.DeleteAllData();
}
