#include "stdafx.h"
//#include "ObjectMgr.h"

ObjectMgr* ObjectMgr::instance()
{
	static ObjectMgr instance;
	return &instance;
}

std::vector<_SERVER_TAB*> ObjectMgr::LoadServerTables()
{
	std::vector<_SERVER_TAB*> serverVec;
	QueryResult* pRes = AccountDatabase.ExecuteQuery(SQL_SEL_SERVERTABS);
	QueryResult* pRes2;

	if (pRes == NULL)
		return serverVec;

	uint32 startTime = GetMSTime();

	_SERVER_TAB* pTab = NULL;
	_SERVER_INFO* pInfo = NULL;

	do
	{
		pTab = new _SERVER_TAB();

		pTab->m_tabId = pRes->GetUint8();
		pTab->m_tabName = pRes->GetString();
		pTab->m_serverIp = pRes->GetString();
		pTab->m_port = pRes->GetUint16();

		pRes2 = AccountDatabase.ExecuteQuery(string_format(SQL_SEL_SERVERINFO, pTab->m_tabId).c_str());

		if (pRes2 == NULL)
			return serverVec;

		do
		{
			pInfo = new _SERVER_INFO();

			pInfo->m_tabId = pTab->m_tabId;
			pInfo->m_serverId = pRes2->GetUint8();
			pInfo->m_serverName = pRes2->GetString();
			pInfo->m_serverName.append(string_format("\x20%d", pInfo->m_serverId + 1));
			pInfo->m_curPlayers = pRes2->GetUint16();
			pInfo->m_maxPlayers = pRes2->GetUint16();
			pInfo->m_serverStatus = (ServerStatus)pRes2->GetUint8();

			pTab->m_serverInfoArr.push_back(pInfo);
		} while (pRes2->NextRow());
		serverVec.push_back(pTab);
	} while (pRes->NextRow());

	return serverVec;
}

bool ObjectMgr::LoadMapTables()
{
	QueryResult* pRes = GameDatabase.ExecuteQuery(SQL_SEL_ZONEDATA);

	if (pRes == NULL)
		return false;

	uint32 startTime = GetMSTime();

	_ZONE_INFO* pZone = NULL;
	MAP* pMap = NULL;

	do
	{
		pZone = new _ZONE_INFO();

		pZone->m_zoneNum = pRes->GetUint32();
		pZone->m_mapName = pRes->GetString();
		pZone->m_smdFile = pRes->GetString();

		pMap = ReadMapFile(pZone);
		if (pMap == NULL)
		{
			m_zoneTableArray.DeleteAllData();
			return false;
		}
		m_zoneTableArray.PutData(pMap->m_zoneNum, pMap);
	} while (pRes->NextRow());

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
	QueryResult* pRes = GameDatabase.ExecuteQuery(SQL_SEL_ITEM_SET);

	if (pRes == NULL)
		return false;

	uint32 startTime = GetMSTime();

	_SET_BONUS* pSet = NULL;

	do
	{
		pSet = new _SET_BONUS();

		pSet->m_setId = pRes->GetUint32();
		pSet->m_itemCount = pRes->GetUint8();
		for (int i = 0; i < MAX_SET_PARTS; i++)
			pSet->m_setPart[i] = pRes->GetUint32();
		for (int i = 0; i < MAX_PARTIAL_SET_BONUS; i++)
			pSet->m_bonus[i] = pRes->GetUint16();
		for (int i = 0; i < MAX_EXTRA_PARTIAL_SET_BONUS; i++)
			pSet->m_bonusExtra[i] = pRes->GetUint16();
		m_setBonusArray.PutData(pSet->m_setId, pSet);
	} while (pRes->NextRow());

	printf("\nLoaded %u item sets in %ums\n", m_setBonusArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadItemTemplate()
{
	QueryResult* pRes = GameDatabase.ExecuteQuery(SQL_SEL_ITEMDATA);

	if (pRes == NULL)
		return false;

	uint32 startTime = GetMSTime();

	_ITEM_TABLE* pTable = NULL;

	do
	{
		pTable = new _ITEM_TABLE();

		pTable->m_itemId = pRes->GetUint32();
		pTable->m_name = pRes->GetString();
		pTable->m_upgradeCode = pRes->GetString();
		pTable->m_upgradeDescription = pRes->GetString();
		pTable->m_itemRelation = pRes->GetUint32();
		pTable->m_nextItemStage = pRes->GetUint32();
		pTable->m_itemType = pRes->GetUint8();
		pTable->m_itemRarity = pRes->GetUint8();
		pTable->m_setBonus = GetSetBonus(pRes->GetUint32());
		pTable->m_buyPrice = pRes->GetUint32();
		pTable->m_sellPrice = pRes->GetUint32();
		pTable->m_equipPos = pRes->GetUint16();
		pTable->m_reqClass = pRes->GetUint8();
		pTable->m_reqLevel = pRes->GetUint32();
		pTable->m_maxLevel = pRes->GetUint32();
		pTable->m_strReq = pRes->GetUint16();
		pTable->m_dexReq = pRes->GetUint16();
		pTable->m_intReq = pRes->GetUint16();
		pTable->m_headDefense = pRes->GetUint32();
		pTable->m_bodyDefense = pRes->GetUint32();
		pTable->m_footDefense = pRes->GetUint32();
		pTable->m_damageMin = pRes->GetUint16();
		pTable->m_damageMax = pRes->GetUint16();
		pTable->m_delay = pRes->GetUint16() / 100;
		pTable->m_attackRange = pRes->GetFloat();
		pTable->m_attackAoE = pRes->GetFloat();
		for (int i = 0; i < STAT_END; i++)
			pTable->m_statBonus[i] = pRes->GetUint16();
		pTable->m_poisonDamage = pRes->GetUint8();
		pTable->m_poisonDefense = pRes->GetUint8();
		pTable->m_confusionDamage = pRes->GetUint8();
		pTable->m_confusionDefense = pRes->GetUint8();
		pTable->m_paralysisDamage = pRes->GetUint8();
		pTable->m_paralysisDefense = pRes->GetUint8();
		pTable->m_maxHp = pRes->GetUint16();
		pTable->m_hpRecoveryPercent = pRes->GetUint16();
		pTable->m_maxChi = pRes->GetUint16();
		pTable->m_chiRecoveryPercent = pRes->GetUint16();
		pTable->m_movementSpeed = pRes->GetFloat();
		pTable->m_minBonusDamage = pRes->GetUint16();
		pTable->m_maxBonusDamage = pRes->GetUint16();
		pTable->m_damagePercent = pRes->GetUint8();
		pTable->m_minSkillDamageBonus = pRes->GetUint16();
		pTable->m_maxSkillDamageBonus = pRes->GetUint16();
		pTable->m_skillDamagePercent = pRes->GetUint8();
		pTable->m_defense = pRes->GetUint32();
		pTable->m_defensePercent = pRes->GetUint8();
		pTable->m_artsDefense = pRes->GetUint32();
		pTable->m_artsDefensePercent = pRes->GetUint8();
		pTable->m_accuracy = pRes->GetUint16();
		pTable->m_dodge = pRes->GetUint16();
		pTable->m_hpRecovery = pRes->GetInt16();
		pTable->m_chiRecovery = pRes->GetInt16();
		pTable->m_critChance = pRes->GetUint8();
		pTable->m_bonusExp = pRes->GetUint8();
		pTable->m_itemDropBonus = pRes->GetUint16();
		pTable->m_bonusMineral = pRes->GetUint16();
		pTable->m_stackable = pTable->m_itemType > 148 ? true : false; //TODO: This isn't how it's supposed to be done, it's just temporary to fix most items.
		m_itemTableArray.PutData(pTable->m_itemId, pTable);
	} while (pRes->NextRow());

	printf("Loaded %u item tables in %ums\n", m_itemTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadItemDropTable()
{
	QueryResult* pRes = GameDatabase.ExecuteQuery(SQL_SEL_ITEMDROPDATA);

	if (pRes == NULL)
		return false;

	uint32 startTime = GetMSTime();

	_ITEM_DROP_TABLE* pTable = NULL;

	do
	{
		pTable = new _ITEM_DROP_TABLE();

		pTable->m_maxRoll = 0;
		pTable->m_dropId = pRes->GetUint32();
		uint8 availableDrops = 0;
		for (; availableDrops < NUM_DROPS_IN_DROP_TABLE; availableDrops++)
			pTable->m_dropableId[availableDrops] = pRes->GetUint32();
		for (int i = 0; i < NUM_DROPS_IN_DROP_TABLE; i++)
		{
			pTable->m_dropableChance[i] = pRes->GetUint16();
			if (pTable->m_dropableChance[i] > pTable->m_maxRoll)
				pTable->m_maxRoll = pTable->m_dropableChance[i];
		}
		m_dropTableArray.PutData(pTable->m_dropId, pTable);
	} while (pRes->NextRow());

	printf("Loaded %u item drop tables in %ums\n", m_dropTableArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadGamblingItemTable()
{
	QueryResult* pRes = GameDatabase.ExecuteQuery(SQL_SEL_GAMBLINGITEM);

	if (pRes == NULL)
		return false;

	uint32 startTime = GetMSTime();

	_GAMBLING_ITEM_TABLE* pTable = NULL;

	do
	{
		pTable = new _GAMBLING_ITEM_TABLE();

		pTable->m_itemId = pRes->GetUint32();
		pTable->m_openingCost = pRes->GetUint32();
		pTable->m_dropTable = sObjMgr->GetItemDropTable(pRes->GetUint32());

		if (pTable->m_dropTable != NULL)
			m_gamblingItemArray.PutData(pTable->m_itemId, pTable);
	} while (pRes->NextRow());

	printf("Loaded %u gambling item tables in %ums\n", m_gamblingItemArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadLevelData()
{
	QueryResult* pRes = GameDatabase.ExecuteQuery(SQL_SEL_LEVELDATA);

	if (pRes == NULL)
		return false;

	uint32 startTime = GetMSTime();

	_LEVEL_DATA* pTable = NULL;

	do
	{
		pTable = new _LEVEL_DATA();

		pTable->m_level = pRes->GetUint32();
		pTable->m_expReq = pRes->GetUint64();
		pTable->m_statPoint = pRes->GetUint16();
		pTable->m_statElement = pRes->GetUint16();
		pTable->m_unk2 = pRes->GetUint8();
		pTable->m_unk3 = pRes->GetUint8();

		m_levelDataArray.PutData(pTable->m_level, pTable);
	} while (pRes->NextRow());

	printf("Loaded level data for %u levels in %ums\n", m_levelDataArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadZoneChangeTable()
{
	QueryResult* pRes = GameDatabase.ExecuteQuery(SQL_SEL_ZONECHANGEDATA);

	if (pRes == NULL)
		return false;

	uint32 startTime = GetMSTime();

	_ZONECHANGE_DATA* pTable = NULL;

	do
	{
		pTable = new _ZONECHANGE_DATA();

		pTable->m_zoneChangeId = pRes->GetUint32();
		pTable->m_toZoneId = pRes->GetUint8();
		pTable->m_posX = pRes->GetFloat();
		pTable->m_posZ = pRes->GetFloat();
		pTable->m_reqLevel = pRes->GetUint32();

		m_zoneChangeArray.PutData(pTable->m_zoneChangeId, pTable);
	} while (pRes->NextRow());

	printf("Loaded [NOTE: Some doesn't exist and some are wrong] %d rows of zone change data in %ums\n", m_zoneChangeArray.GetSize(), GetMSTime() - startTime);
	return true;
}

bool ObjectMgr::LoadCharacterInfoTable()
{
	QueryResult* pRes = GameDatabase.ExecuteQuery(SQL_SEL_CHARDATA);

	if (pRes == NULL)
		return false;

	uint32 startTime = GetMSTime();

	_CHARACTER_DATA* pTable = NULL;

	do
	{
		pTable = new _CHARACTER_DATA();

		pTable->m_typeId = pRes->GetUint32();
		pTable->m_nextTypeId = pRes->GetUint8();
		pTable->m_baseHp = pRes->GetUint32();
		pTable->m_baseChi = pRes->GetUint32();
		pTable->m_baseStr = pRes->GetUint32();
		pTable->m_baseDex = pRes->GetUint32();
		pTable->m_baseInt = pRes->GetUint32();
		pTable->m_hpPerLevel= pRes->GetUint8();
		pTable->m_chiPerLevel = pRes->GetUint8();
		pTable->m_dmgPerLevel = pRes->GetFloat();
		pTable->m_sdmgPerLevel = pRes->GetFloat();
		pTable->m_defPerLevel = pRes->GetFloat();
		pTable->m_sdefPerLevel = pRes->GetFloat();

		m_characterInfoArray.PutData(pTable->m_typeId, pTable);
	} while (pRes->NextRow());

	printf("Loaded base info about %d character types in %ums\n.", m_characterInfoArray.GetSize(), GetMSTime() - startTime);
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

_LEVEL_DATA* ObjectMgr::GetLevelData(uint32 level)
{
	return m_levelDataArray.GetData(level);
}

_ZONECHANGE_DATA* ObjectMgr::GetZoneChangeData(uint8 zoneId)
{
	return m_zoneChangeArray.GetData(zoneId);
}
_CHARACTER_DATA * ObjectMgr::GetCharacterInfo(uint8 charTypeId)
{
	return m_characterInfoArray.GetData(charTypeId);
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
}
