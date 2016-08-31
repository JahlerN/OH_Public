#include "stdafx.h"
//#include "MAP.h"

bool CDBAgent::Connect()
{
	if (!m_AccountDB.Connect(g_main->m_accountDBName, g_main->m_accountDBId, g_main->m_accountDBPw))
	{
		g_main->PrintSQLError(m_AccountDB.GetError());
		return false;
	}

	if (!m_GameDB.Connect(g_main->m_gameDBName, g_main->m_gameDBId, g_main->m_gameDBPw))
	{
		g_main->PrintSQLError(m_GameDB.GetError());
		return false;
	}

	return true;
}

//std::vector<_SERVER_TAB*> CDBAgent::LoadServerTables()
//{
//	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
//	std::vector<_SERVER_TAB*> result;
//
//	if (dbCommand.get() == NULL)
//		return result;
//
//	if (!dbCommand->Execute(SQL_SEL_SERVERTABS))
//		return result;
//
//	if (!dbCommand->hasData())
//		return result;
//
//	do
//	{
//		_SERVER_TAB* pTab = new _SERVER_TAB();
//		dbCommand->FetchByte(1, pTab->m_tabId);
//		dbCommand->FetchString(2, pTab->m_tabName);
//		dbCommand->FetchString(3, pTab->m_serverIp);
//		dbCommand->FetchUInt16(4, pTab->m_port);
//
//		auto_ptr<OdbcCommand> dbCommand2(m_AccountDB.CreateCommand());
//
//		if (dbCommand2.get() == NULL)
//			return result;
//
//		if (!dbCommand2->Execute(_T(string_format(SQL_SEL_SERVERINFO, pTab->m_tabId))))
//			return result;
//
//		if (!dbCommand2->hasData())
//			return result;
//
//		do
//		{
//			int field = 1;
//			_SERVER_INFO* pInfo = new _SERVER_INFO();
//			pInfo->m_tabId = pTab->m_tabId;
//
//			dbCommand2->FetchByte(field++, pInfo->m_serverId);
//			dbCommand2->FetchString(field++, pInfo->m_serverName);
//			pInfo->m_serverName.append(string_format("\x20%d", pInfo->m_serverId + 1));
//			dbCommand2->FetchUInt16(field++, pInfo->m_curPlayers);
//			dbCommand2->FetchUInt16(field++, pInfo->m_maxPlayers);
//			pInfo->m_serverStatus = (ServerStatus)dbCommand2->FetchByte(field++);
//
//			pTab->m_serverInfoArr.push_back(pInfo);
//		} while (dbCommand2->MoveNext());
//		result.push_back(pTab);
//	} while (dbCommand->MoveNext());
//	return result;
//}

/* LOAD SERVER TABLES*/
//bool CDBAgent::LoadItemTable(ItemTableArray& out)
//{
//	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T(SQL_SEL_ITEMDATA)))
//		return false;
//
//	if (!dbCommand->hasData())
//		return false;
//
//	float start = GetFloatTime();
//	do 
//	{
//		int field = 1;
//		_ITEM_TABLE* pItem = new _ITEM_TABLE();
//		dbCommand->FetchUInt32(field++, pItem->m_itemId);
//		dbCommand->FetchString(field++, pItem->m_name);
//		dbCommand->FetchString(field++, pItem->m_upgradeCode);
//		dbCommand->FetchString(field++, pItem->m_upgradeDescription);
//		dbCommand->FetchUInt32(field++, pItem->m_itemRelation);
//		dbCommand->FetchUInt32(field++, pItem->m_nextItemStage);
//		dbCommand->FetchByte(field++, pItem->m_itemType);
//		dbCommand->FetchByte(field++, pItem->m_itemRarity);
//		pItem->m_setBonus = g_main->GetSetBonusTemplate(dbCommand->FetchUInt32(field++));
//		dbCommand->FetchUInt32(field++, pItem->m_buyPrice);
//		dbCommand->FetchUInt32(field++, pItem->m_sellPrice);
//		dbCommand->FetchByte(field++, pItem->m_equipPos);
//		dbCommand->FetchByte(field++, pItem->m_reqClass);
//		dbCommand->FetchUInt32(field++, pItem->m_reqLevel);
//		dbCommand->FetchUInt32(field++, pItem->m_maxLevel);
//		dbCommand->FetchUInt16(field++, pItem->m_strReq);
//		dbCommand->FetchUInt16(field++, pItem->m_dexReq);
//		dbCommand->FetchUInt16(field++, pItem->m_intReq);
//		dbCommand->FetchUInt32(field++, pItem->m_headDefense);
//		dbCommand->FetchUInt32(field++, pItem->m_bodyDefense);
//		dbCommand->FetchUInt32(field++, pItem->m_footDefense);
//		dbCommand->FetchUInt16(field++, pItem->m_damageMin);
//		dbCommand->FetchUInt16(field++, pItem->m_damageMax);
//		dbCommand->FetchDouble(field++, pItem->m_delay);
//		dbCommand->FetchDouble(field++, pItem->m_attackRange);
//		dbCommand->FetchDouble(field++, pItem->m_attackAoE);
//		for (int i = 0; i < STAT_END; i++)
//			dbCommand->FetchUInt16(field++, pItem->m_statBonus[i]);
//		dbCommand->FetchByte(field++, pItem->m_poisonDamage);
//		dbCommand->FetchByte(field++, pItem->m_poisonDefense);
//		dbCommand->FetchByte(field++, pItem->m_confusionDamage);
//		dbCommand->FetchByte(field++, pItem->m_confusionDefense);
//		dbCommand->FetchByte(field++, pItem->m_paralysisDamage);
//		dbCommand->FetchByte(field++, pItem->m_paralysisDefense);
//		dbCommand->FetchUInt32(field++, pItem->m_maxHp);
//		dbCommand->FetchByte(field++, pItem->m_hpRecoveryPercent);
//		dbCommand->FetchUInt32(field++, pItem->m_maxChi);
//		dbCommand->FetchByte(field++, pItem->m_chiRecoveryPercent);
//		pItem->m_movementSpeed = (float)dbCommand->FetchDouble(field++);
//		dbCommand->FetchUInt32(field++, pItem->m_minBonusDamage);
//		dbCommand->FetchUInt32(field++, pItem->m_maxBonusDamage);
//		dbCommand->FetchUInt16(field++, pItem->m_damagePercent);
//		dbCommand->FetchUInt32(field++, pItem->m_minSkillDamageBonus);
//		dbCommand->FetchUInt32(field++, pItem->m_maxSkillDamageBonus);
//		dbCommand->FetchUInt16(field++, pItem->m_skillDamagePercent);
//		dbCommand->FetchUInt32(field++, pItem->m_defense);
//		dbCommand->FetchUInt16(field++, pItem->m_defensePercent);
//		dbCommand->FetchUInt32(field++, pItem->m_artsDefense);
//		dbCommand->FetchUInt16(field++, pItem->m_artsDefensePercent);
//		dbCommand->FetchUInt16(field++, pItem->m_accuracy);
//		dbCommand->FetchUInt16(field++, pItem->m_dodge);
//		dbCommand->FetchUInt16(field++, pItem->m_hpRecovery);
//		dbCommand->FetchUInt16(field++, pItem->m_chiRecovery);
//		dbCommand->FetchByte(field++, pItem->m_critChance);
//		dbCommand->FetchByte(field++, pItem->m_bonusExp);
//		dbCommand->FetchUInt16(field++, pItem->m_itemDropBonus);
//		dbCommand->FetchUInt16(field++, pItem->m_bonusMineral);
//		pItem->m_stackable = pItem->m_itemType > 148 ? true : false; //TODO: This isn't how it's supposed to be done, it's just temporary to fix most items.
//		out.PutData(pItem->m_itemId, pItem);
//	} while (dbCommand->MoveNext());
//	printf("Loaded %d items in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);
//
//	return true;
//}

//bool CDBAgent::LoadItemDropTable(ItemDropTableArray & out)
//{
//	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T(SQL_SEL_ITEMDROPDATA)))
//		return false;
//
//	if (!dbCommand->hasData())
//		return false;
//
//	float start = GetFloatTime();
//	do
//	{
//		int field = 1;
//		_ITEM_DROP_TABLE* pItemDrop = new _ITEM_DROP_TABLE();
//		pItemDrop->m_maxRoll = 0;
//		dbCommand->FetchUInt32(field++, pItemDrop->m_dropId);
//		uint8 availableDrops = 0;
//		for (int i = 0; i < NUM_DROPS_IN_DROP_TABLE; i++)
//		{
//			dbCommand->FetchUInt32(field++, pItemDrop->m_dropableId[i]);
//			availableDrops++;
//		}
//		pItemDrop->m_availableDrops = availableDrops;
//		for (int i = 0; i < NUM_DROPS_IN_DROP_TABLE; i++)
//		{
//			dbCommand->FetchUInt32(field++, pItemDrop->m_dropableChance[i]);
//			if (pItemDrop->m_dropableChance[i] > pItemDrop->m_maxRoll)
//				pItemDrop->m_maxRoll = pItemDrop->m_dropableChance[i];
//		}
//		out.PutData(pItemDrop->m_dropId, pItemDrop);
//	} while (dbCommand->MoveNext());
//	printf("Loaded %d drop tables in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);
//	return true;
//}

//bool CDBAgent::LoadGamblingItemTable(GamblingItemTableArray & out)
//{
//	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T(SQL_SEL_GAMBLINGITEM)))
//		return false;
//
//	if (!dbCommand->hasData())
//		return false;
//
//	float start = GetFloatTime();
//	do
//	{
//		int field = 1;
//		_GAMBLING_ITEM_TABLE* pGambleItem = new _GAMBLING_ITEM_TABLE();
//		dbCommand->FetchUInt32(field++, pGambleItem->m_itemId);
//		dbCommand->FetchUInt32(field++, pGambleItem->m_openingCost);
//		pGambleItem->m_dropTable = sObjMgr->GetItemDropTable(dbCommand->FetchUInt32(field++));
//
//		if (pGambleItem->m_dropTable != NULL)
//			out.PutData(pGambleItem->m_itemId, pGambleItem);
//	} while (dbCommand->MoveNext());
//	printf("Loaded %d gambling items in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);
//	return true;
//}

//THIS HAS TO BE LOADED BEFORE ITEM TABLE!
//bool CDBAgent::LoadSetBonusTable(SetBonusArray & out)
//{
//	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T(SQL_SEL_ITEM_SET)))
//		return false;
//
//	if (!dbCommand->hasData())
//		return false;
//
//	float start = GetFloatTime();
//	do
//	{
//		int field = 1;
//		_SET_BONUS* pSetBonus = new _SET_BONUS();
//		dbCommand->FetchUInt32(field++, pSetBonus->m_setId);
//		dbCommand->FetchByte(field++, pSetBonus->m_itemCount);
//		for (int i = 0; i < MAX_SET_PARTS; i++)
//			dbCommand->FetchUInt32(field++, pSetBonus->m_setPart[i]);
//		for (int i = 0; i < MAX_PARTIAL_SET_BONUS; i++)
//			dbCommand->FetchUInt16(field++, pSetBonus->m_bonus[i]);
//		for (int i = 0; i < MAX_EXTRA_PARTIAL_SET_BONUS; i++)
//			dbCommand->FetchUInt16(field++, pSetBonus->m_bonusExtra[i]);
//		out.PutData(pSetBonus->m_setId, pSetBonus);
//	} while (dbCommand->MoveNext());
//	printf("Loaded %d item sets in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);
//
//	return true;
//}

//bool CDBAgent::LoadMapTable(ZoneTableArray & out, uint8 & totalMap)
//{
//	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T(SQL_SEL_ZONEDATA)))
//		return false;
//
//	if (!dbCommand->hasData())
//		return false;
//	float start = GetFloatTime();
//	std::map<int, _ZONE_INFO*> zoneMap;
//
//	do
//	{
//		_ZONE_INFO* pInfo = new _ZONE_INFO();
//		pInfo->m_zoneNum = dbCommand->FetchByte(1);
//		//pInfo->m_serverNum = dbCommand->FetchByte(2);
//		dbCommand->FetchBinary(2, (char*)pInfo->m_mapName, sizeof(pInfo->m_mapName));//TODO: Not sure if that conversion is ok
//		dbCommand->FetchString(3, pInfo->m_smdFile);
//		//pInfo->m_roomEvent = dbCommand->FetchByte(4);
//		
//		zoneMap.insert(std::pair<int, _ZONE_INFO*>(pInfo->m_zoneNum, pInfo));
//		//delete pInfo;//TODO: I'm not sure i should delete it.
//	} while (dbCommand->MoveNext());
//
//	foreach(itr, zoneMap)
//	{
//		//TODO: Rewrite this using ofstream etc instead of handles etc?
//		//ofstream fout;
//		HANDLE hFile;
//		std::string fullPath;
//		_ZONE_INFO* pZone = itr->second;
//
//		MAP* pMap = new MAP();
//		pMap->Initialize(pZone);
//		delete pZone;
//
//		out.PutData(pMap->m_zoneNum, pMap);
//
//		fullPath = string_format("..\\Debug\\MAP\\%s", pMap->m_smdFile.c_str());
//		//fout.open(fullPath, ios::out);
//		//FILE* fp = fopen(fullPath.c_str(), "rb");
//		hFile = CreateFile(fullPath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//
//		
//		if (hFile == INVALID_HANDLE_VALUE || !pMap->LoadMap(hFile))//!file.Open(fullPath, CFile::modeRead) || pMap->LoadMap((HANDLE)file.m_hFile))
//		{
//			printf("\nUnable to load map file: %s.\n", pMap->m_mapName.c_str());
//			out.DeleteAllData();
//			totalMap = 0;
//			return false;
//		}
//		//fout.close();
//		CloseHandle(hFile);
//
//		if (pMap->m_roomEvent > 0)
//		{
//			//TODO: Handle room events, not yet implemented
//		}
//		totalMap++;
//	}
//	printf("Loaded %d maps in %.3f second(s).", totalMap, GetFloatTime() - start);
//
//	return true;
//}

//bool CDBAgent::LoadZoneChangeTable(ZoneChangeTableArray& out)
//{
//	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T(SQL_SEL_ZONECHANGEDATA)))
//		return false;
//
//	if (!dbCommand->hasData())
//		return false;
//
//	float start = GetFloatTime();
//
//	do
//	{
//		int field = 1;
//		_ZONECHANGE_DATA* pZoneChangeData = new _ZONECHANGE_DATA();
//		dbCommand->FetchUInt32(field++, pZoneChangeData->m_zoneChangeId);
//		dbCommand->FetchByte(field++, pZoneChangeData->m_toZoneId);
//		pZoneChangeData->m_posX = (float)dbCommand->FetchDouble(field++);
//		pZoneChangeData->m_posZ = (float)dbCommand->FetchDouble(field++);
//		dbCommand->FetchUInt32(field++, pZoneChangeData->m_reqLevel);
//		out.PutData(pZoneChangeData->m_zoneChangeId, pZoneChangeData);
//	} while (dbCommand->MoveNext());
//	printf("Loaded zoneChangeData in %.3f second(s).\n", GetFloatTime() - start);
//
//	return true;
//}
//
//bool CDBAgent::LoadLevelData(LevelDataArray& out)
//{
//	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T(SQL_SEL_LEVELDATA)))
//		return false;
//
//	if (!dbCommand->hasData())
//		return false;
//
//	float start = GetFloatTime();
//	do
//	{
//		_LEVEL_DATA* pData = new _LEVEL_DATA();
//		dbCommand->FetchUInt32(1, pData->m_level);
//		dbCommand->FetchUInt64(2, pData->m_expReq);
//		dbCommand->FetchUInt16(3, pData->m_statPoint);
//		dbCommand->FetchUInt16(4, pData->m_statElement);
//		dbCommand->FetchByte(5, pData->m_unk2);
//		dbCommand->FetchByte(6, pData->m_unk3);
//		out.PutData(pData->m_level, pData);
//	} while (dbCommand->MoveNext());
//	printf("Loaded leveldata for %d levels in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);
//
//	return true;
//}

//bool CDBAgent::LoadCharacterInfoTable(CharacterInfoArray & out)
//{
//	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
//	if (dbCommand.get() == NULL)
//		return false;
//
//	if (!dbCommand->Execute(_T(SQL_SEL_CHARDATA)))
//		return false;
//
//	if (!dbCommand->hasData())
//		return false;
//
//	float start = GetFloatTime();
//	do
//	{
//		int field = 1;
//		_CHARACTER_DATA* pData = new _CHARACTER_DATA();
//		dbCommand->FetchByte(field++, pData->m_typeId);
//		dbCommand->FetchByte(field++, pData->m_nextTypeId);
//		dbCommand->FetchUInt32(field++, pData->m_baseHp);
//		dbCommand->FetchUInt32(field++, pData->m_baseChi);
//		dbCommand->FetchUInt16(field++, pData->m_baseStr);
//		dbCommand->FetchUInt16(field++, pData->m_baseDex);
//		dbCommand->FetchUInt16(field++, pData->m_baseInt);
//		dbCommand->FetchUInt16(field++, pData->m_hpPerLevel);
//		dbCommand->FetchUInt16(field++, pData->m_chiPerLevel);
//		pData->m_dmgPerLevel = (float)dbCommand->FetchDouble(field++);
//		pData->m_sdmgPerLevel = (float)dbCommand->FetchDouble(field++);
//		pData->m_defPerLevel = (float)dbCommand->FetchDouble(field++);
//		pData->m_sdefPerLevel = (float)dbCommand->FetchDouble(field++);
//		out.PutData(pData->m_typeId, pData);
//	} while (dbCommand->MoveNext());
//	printf("Loaded char info for %d char types in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);
//
//	return true;
//}

bool CDBAgent::LoadNpcInfoTable(NpcInfoArray & out)
{
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return false;

	if (!dbCommand->Execute(_T(SQL_SEL_NPCDATA)))
		return false;

	if (!dbCommand->hasData())
		return false;

	float start = GetFloatTime();
	do
	{
		int field = 1;
		_NPC_DATA* pData = new _NPC_DATA();
		dbCommand->FetchUInt32(field++, pData->m_npcId);
		dbCommand->FetchString(field++, pData->m_name);
		dbCommand->FetchString(field++, pData->m_phrase);
		dbCommand->FetchByte(field++, pData->m_npcType);
		dbCommand->FetchUInt32(field++, pData->m_level);
		dbCommand->FetchUInt32(field++, pData->m_exp);
		dbCommand->FetchUInt32(field++, pData->m_divineExp);
		dbCommand->FetchUInt32(field++, pData->m_darknessExp);
		dbCommand->FetchByte(field++, pData->m_lootRolls);
		dbCommand->FetchUInt32(field++, pData->m_dropId);
		dbCommand->FetchUInt32(field++, pData->m_maxHp);
		dbCommand->FetchUInt32(field++, pData->m_maxChi);
		dbCommand->FetchUInt16(field++, pData->m_unkUse);
		dbCommand->FetchUInt32(field++, pData->m_gold);
		dbCommand->FetchUInt32(field++, pData->m_minDmg);
		dbCommand->FetchUInt32(field++, pData->m_maxDmg);
		dbCommand->FetchUInt32(field++, pData->m_minSkillDmg);
		dbCommand->FetchUInt32(field++, pData->m_maxSkillDmg);
		out.PutData(pData->m_npcId, pData);
	} while (dbCommand->MoveNext());
	printf("Loaded information about %d Npcs in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);

	return true;
}

bool CDBAgent::LoadNpcTable(NpcTableArray & out)
{
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return false;

	if (!dbCommand->Execute(_T(SQL_SEL_NPC)))
		return false;

	if (!dbCommand->hasData())
		return false;

	float start = GetFloatTime();
	do
	{
		int field = 1;
		_NPC_GROUP* pData = new _NPC_GROUP();
		dbCommand->FetchUInt32(field++, pData->m_npcGroupId);
		dbCommand->FetchByte(field++, pData->m_zoneId);
		dbCommand->FetchUInt32(field++, pData->m_npcId);
		pData->m_rotation = (float)dbCommand->FetchDouble(field++);
		pData->m_minX = (float)dbCommand->FetchDouble(field++);
		pData->m_minZ = (float)dbCommand->FetchDouble(field++);
		pData->m_maxX = (float)dbCommand->FetchDouble(field++);
		pData->m_maxZ = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchByte(field++, pData->m_npcsInGroup);
		dbCommand->FetchUInt32(field++, pData->m_respawnTime);
		out.PutData(pData->m_npcGroupId, pData);
	} while (dbCommand->MoveNext());
	printf("Loaded %d Npc group instances in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);

	return true;
}

bool CDBAgent::LoadSkillTable(SkillTableArray& out)
{
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return false;

	if (!dbCommand->Execute(_T(SQL_SEL_SKILLDATA)))
		return false;

	if (!dbCommand->hasData())
		return false;

	float start = GetFloatTime();
	do
	{
		int field = 1;
		_SKILL_DATA* pData = new _SKILL_DATA();
		dbCommand->FetchUInt32(field++, pData->m_skillId);
		dbCommand->FetchString(field++, pData->m_name);
		dbCommand->FetchString(field++, pData->m_description);
		dbCommand->FetchByte(field++, pData->m_skillType);
		dbCommand->FetchByte(field++, pData->m_statType);
		pData->m_aoeType = (AoeType)dbCommand->FetchByte(field++);
		dbCommand->FetchByte(field++, pData->m_pierceType);
		dbCommand->FetchByte(field++, pData->m_maxLevel);
		dbCommand->FetchByte(field++, pData->m_slot);
		dbCommand->FetchByte(field++, pData->m_unlockSlot1);
		dbCommand->FetchByte(field++, pData->m_unlockSlot2);
		dbCommand->FetchByte(field++, pData->m_levelForUnlock1);
		dbCommand->FetchByte(field++, pData->m_levelForUnlock2);
		dbCommand->FetchUInt32(field++, pData->m_duration);
		dbCommand->FetchUInt32(field++, pData->m_durationPerLevel);
		dbCommand->FetchInt32(field++, pData->m_chiUsage);
		pData->m_chiUsagePerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt32(field++, pData->m_chiUsagePerSecond);
		pData->m_chiUsagePerSecondPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt32(field++, pData->m_minDmg);
		pData->m_minDmgPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt32(field++, pData->m_maxDmg);
		pData->m_maxDmgPerLevel = (float)dbCommand->FetchDouble(field++);
		pData->m_range = (float)dbCommand->FetchDouble(field++);
		pData->m_rangePerLevel = (float)dbCommand->FetchDouble(field++);
		pData->m_aoe = (float)dbCommand->FetchDouble(field++);
		pData->m_aoePerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt32(field++, pData->m_cooldown);
		dbCommand->FetchInt32(field++, pData->m_statTypeBonus);
		pData->m_statTypeBonusPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchSByte(field++, pData->m_poisonDmg);
		pData->m_poisonDmgPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchSByte(field++, pData->m_poisonHeal);
		pData->m_poisonHealPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchSByte(field++, pData->m_confusionDmg);
		pData->m_confusionDmgPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchSByte(field++, pData->m_confusionHeal);
		pData->m_confusionHealPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchSByte(field++, pData->m_paralysisDmg);
		pData->m_paralysisDmgPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchSByte(field++, pData->m_paralysisHeal);
		pData->m_paralysisHealPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt16(field++, pData->m_minDmgBonus);
		pData->m_minDmgBonusPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt16(field++, pData->m_maxDmgBonus);
		pData->m_maxDmgBonusPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt16(field++, pData->m_attackPower);
		pData->m_attackPowerPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt16(field++, pData->m_defenseivePower);
		pData->m_defensivePowerPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchUInt16(field++, pData->m_injury);
		pData->m_injuryPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt16(field++, pData->m_accuracy);
		pData->m_accuracyPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt16(field++, pData->m_dodge);
		pData->m_dodgePerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchInt16(field++, pData->m_defenseBonus);
		pData->m_defenseBonusPerLevel = (float)dbCommand->FetchDouble(field++);
		pData->m_movementSpeed = (float)dbCommand->FetchDouble(field++);
		pData->m_movementSpeedPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchUInt16(field++, pData->m_minHpChange);
		pData->m_minHpChangePerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchUInt16(field++, pData->m_maxHpChange);
		pData->m_maxHpChangePerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchUInt16(field++, pData->m_minChiChange);
		pData->m_minChiChangePerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchUInt16(field++, pData->m_maxChiChange);
		pData->m_maxChiChangePerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchUInt16(field++, pData->m_healPerSecond);
		pData->m_healPerSecondPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchByte(field++, pData->m_percentHp);
		pData->m_percentHpPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchByte(field++, pData->m_percentChi);
		pData->m_percentChiPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchUInt16(field++, pData->m_reflectChance);
		pData->m_reflectChancePerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchUInt16(field++, pData->m_reflectPercent);
		pData->m_reflectPercentPerLevel = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchUInt32(field++, pData->m_skillBookId);
		out.PutData(pData->m_skillId, pData);
	} while (dbCommand->MoveNext());
	printf("Loaded %d skills in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);

	return true;
}

bool CDBAgent::LoadSkillBookTable(SkillBookTableArray& out)
{
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return false;

	if (!dbCommand->Execute(_T(SQL_SEL_SKILLBOOKDATA)))
		return false;

	if (!dbCommand->hasData())
		return false;

	float start = GetFloatTime();
	do
	{
		int field = 1;
		_SKILL_BOOK_DATA* pData = new _SKILL_BOOK_DATA();
		dbCommand->FetchUInt32(field++, pData->m_skillBookId);
		dbCommand->FetchString(field++, pData->m_name);
		dbCommand->FetchByte(field++, pData->m_bookType);
		dbCommand->FetchByte(field++, pData->m_classReq);
		dbCommand->FetchByte(field++, pData->m_weaponType);
		dbCommand->FetchUInt32(field++, pData->m_levelReq);
		for (int i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			auto skill = g_main->GetSkillDataById(dbCommand->FetchUInt32(field++));
			if (skill == NULL)
				continue;
			pData->m_skillData[i] = skill;
		}
		out.PutData(pData->m_skillBookId, pData);
	} while (dbCommand->MoveNext());
	printf("Loaded info about %d skill books in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);
	return true;
}

bool CDBAgent::LoadZoneStartTable(ZoneStartTableArray & out)
{
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return false;

	if (!dbCommand->Execute(_T(SQL_SEL_ZONESTARTDATA)))
		return false;

	if (!dbCommand->hasData())
		return false;

	float start = GetFloatTime();
	do
	{
		int field = 1;
		_ZONESTART_POSITION* pData = new _ZONESTART_POSITION();
		pData->m_zoneId = dbCommand->FetchUInt32(field++);
		pData->m_x = (float)dbCommand->FetchDouble(field++);
		pData->m_z = (float)dbCommand->FetchDouble(field++);
		pData->m_y = (float)dbCommand->FetchDouble(field++);
		dbCommand->FetchByte(field++, pData->m_rangeX);
		dbCommand->FetchByte(field++, pData->m_rangeZ);
		dbCommand->FetchByte(field++, pData->m_rangeY);
		out.PutData(pData->m_zoneId, pData);
	} while (dbCommand->MoveNext());
	printf("Loaded default position data for %d zones in %.3f second(s).\n", out.GetSize(), GetFloatTime() - start);
	return true;
}

bool CDBAgent::GetAccountSession(CUser* pUser)
{
	pUser->m_userData->m_faction = 0;
	std::string lastIp;
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return false;

	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)pUser->m_accountId.c_str(), pUser->m_accountId.length());

	if (!dbCommand->Execute(_T(SQL_SEL_ACCOUNT_SESSION)))
	{
		g_main->PrintSQLError(m_AccountDB.GetError());
		return false;
	}

	if (dbCommand->hasData())
	{
		dbCommand->FetchByte(1, pUser->m_userData->m_faction);
		dbCommand->FetchString(2, lastIp);
		dbCommand->FetchSByte(3, pUser->m_serverTab);
		dbCommand->FetchSByte(4, pUser->m_serverId);
	}

	if (lastIp != pUser->GetRemoteIP() || pUser->m_serverTab < 0 || pUser->m_serverId < 0)
		return false;
	return true;
}

bool CDBAgent::GetAllCharByAccount(std::string& accountId, uint8& count, std::vector<std::string>& charIds)//std::string& charId1, std::string& charId2, std::string& charId3, std::string& charId4, std::string& charId5, std::string& charId6)
{
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return false;

	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)accountId.c_str(), accountId.length());

	if (!dbCommand->Execute(_T(SQL_SEL_ALL_CHARACTERID)))
	{
		g_main->PrintSQLError(m_GameDB.GetError());
		return false;
	}

	if (dbCommand->hasData())
	{
		do
		{
			std::string temp;
			dbCommand->FetchString(1, temp);
			charIds.push_back(temp);
			count++;
		} while (dbCommand->MoveNext());
	}
	else
		return false;

	return true;
}

int8 CDBAgent::GetCharNameByUniqueId(std::string& accountId, uint16& id, char& out)
{
	int8 ret = -1;
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return ret;

	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)accountId.c_str(), accountId.length());

	if (!dbCommand->Execute(string_format(_T(SQL_SEL_CHARID_BY_UNIQUEID), id)))
	{
		g_main->PrintSQLError(m_GameDB.GetError());
		return ret;
	}
	if (!dbCommand->hasData())
		return ret;
	else
	{
		if (!dbCommand->FetchString(1, &out, MAX_ID_SIZE))
			return ret;
		ret = 1;
	}

	return ret;
}

void CDBAgent::LoadCharInfo(std::string& charId, uint8& index, ByteBuffer& result)
{
	uint16 charUniqueId = 0;
	uint32 hairType = 0;
	uint32 faceType = 0;
	uint8 charType = 0;
	uint8 height = 0;
	uint16 level = 0;
	uint8 zone = 0;
	uint8 nClass = 0;
	bool showHelm;
	bool showMask;
	uint8 showHtItems; //TODO: Make this work
	char strItem[MAX_COMBINED_INV_SLOTS];
	_ITEM_DATA arr[MAX_EQUIP_SLOTS + MAX_INVENTORY_SLOTS];
	ByteBuffer itemData;

	if (charId.length() > 0)
	{
		auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());

		if (dbCommand.get() == NULL)
			return;

		dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)charId.c_str(), charId.length());

		if (!dbCommand->Execute(_T(SQL_SEL_CHARACTER)))
		{
			g_main->PrintSQLError(m_GameDB.GetError());
			return;
		}

		memset(strItem, 0x00, sizeof(strItem));
		if (dbCommand->hasData())
		{
			dbCommand->FetchUInt16(1, charUniqueId);
			dbCommand->FetchByte(2, charType);
			dbCommand->FetchByte(3, height);
			dbCommand->FetchUInt32(4, hairType);
			dbCommand->FetchUInt32(5, faceType);
			dbCommand->FetchUInt16(6, level);
			dbCommand->FetchByte(7, zone);
			dbCommand->FetchByte(8, nClass);
			showHelm = dbCommand->FetchByte(9) == 0 ? true : false;
			showMask = dbCommand->FetchByte(10) == 0 ? true : false;
			dbCommand->FetchByte(11, showHtItems);
			dbCommand->FetchBinary(12, strItem, sizeof(strItem));//TODO: Load activeWeapon and display the correct one!
		}
		else
			return;
		itemData.append(strItem, sizeof(strItem));
		//Manual uints are UNK.    //Unk, maybe unique id is 32 bit
		result << charUniqueId << uint16(0x0000) << charId << charType << nClass << uint32(level);
		result << zone;//uint8(0x0C); //Unk
		result << uint8(1) << height << hairType << faceType;
		result << uint32(0x00000000); //Unk

		for (int i = 0; i < IS_PET + 1; i++)//Only sends up to the pet.
		{
			uint32 itemId;
			uint16 count;
			uint8 upgCount;
			itemData >> itemId >> count >> upgCount;
			itemData.rpos(itemData.rpos() + 15 * 2 + 1);//Skips upgs, holeCount, holes. (15, 1, 15
			//TODO: SetID is itemType? idk why the client would need that right now but w/e.. Also fix so you see the items you should(ht items?)
								//SetID			//Unk			//slot		//unk		//upgrade			//UNK				//UNK				//Unk, serial or smth?
			result << itemId << uint8(0x00) << uint16(0x0100) << uint8(i) << uint8(0x00) << uint8(upgCount) << uint32(0x00000000) << uint32(0x00000000) << uint32(0x00000000);
		}
		//			0x000DDA01
					//Tells the game 1 more char inc
		result << uint8(index);//uint32(0x000DDA01) << uint8(0x00);//Unk
	}
}

int8 CDBAgent::CreateCharacter(std::string& accountId, uint8& faction, std::string& charId, uint8 charType, uint8 height, uint32 hairType, uint32 faceType, bool updateFaction)
{//TODO: Get back to this shit, 3 SQL calls for one action NO; NO. Working on CREATE_CHARACTER procedure, get sql error thing tho..
	int8 ret = -1;
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return ret;
	//dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)accountId.c_str(), accountId.length());
	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)charId.c_str(), charId.length());

	//if (!dbCommand->Execute(string_format(_T("{CALL CREATE_CHARACTER (?, ?, %d, %d, %d, %d, %d}"), charType, height, hairType, faceType, faction)))
	//{
	//	g_main->PrintSQLError(m_AccountDB.GetError());
	//	return ret;
	//}
	//else
	//	ret = 1;
	if (!dbCommand->Execute(_T(SQL_SEL_CHARACTERID)))
	{
		g_main->PrintSQLError(m_AccountDB.GetError());
		return ret;
	}
	else
	{
		if (!dbCommand->hasData())//TODO: Seems like select returns 0 anyways, check for that instead.
			return ret;

		dbCommand->Close();
		auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
		if (dbCommand.get() == NULL)
			return ret;

		dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)accountId.c_str(), accountId.length());
		dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)charId.c_str(), charId.length());
		_CHARACTER_DATA* pChar = sObjMgr->GetCharacterInfo(charType);

		//TODO: Add start pos definition, zone too.
		if (!dbCommand->Execute(string_format(_T(SQL_INS_CHARACTER), charType, height, hairType, faceType, 1,
			pChar->m_baseHp, pChar->m_baseChi, pChar->m_baseStr, pChar->m_baseDex, pChar->m_baseInt,
			155, 155)))
		{
			g_main->PrintSQLError(m_AccountDB.GetError());
			return ret;
		}
		ret = 1;
	}
	return ret;
}

void CDBAgent::UpdateAccountFaction(std::string& accountId, uint8& faction)
{
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return;

	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)accountId.c_str(), accountId.length());

	if (!dbCommand->Execute(string_format(_T(SQL_UPD_ACCOUNTFACTION), faction)))
	{
		g_main->PrintSQLError(m_AccountDB.GetError());
	}
}

int8 CDBAgent::DeleteCharacter(std::string& accountId, ::string& charId)
{
	int8 ret = -1;
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return ret;

	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)accountId.c_str(), accountId.length());
	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)charId.c_str(), charId.length());

	if (!dbCommand->Execute(_T(SQL_DEL_CHARACTER)))
	{
		g_main->PrintSQLError(m_AccountDB.GetError());
	}
	else
		ret = 1;
	return ret;
}

bool CDBAgent::LoadUserData(std::string& accountId, std::string& characterId, _USER_DATA* pUser)
{
	auto_ptr<OdbcCommand> dbCommand(m_AccountDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return false;

	if (pUser == NULL || characterId.length() > MAX_ID_SIZE)
		return false;

	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)accountId.c_str(), accountId.length());
	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)characterId.c_str(), characterId.length());

	if (!dbCommand->Execute(_T(SQL_SEL_USER_DATA)))
	{
		g_main->PrintSQLError(m_AccountDB.GetError());
		return false;
	}

	if (!dbCommand->hasData())
		return false;

	char items[MAX_COMBINED_INV_SLOTS];
	memset(items, 0x00, sizeof(items));
	char skillBookData[MAX_EXPANDABLE_SKILLBOOKS * sizeof(_PLAYER_SKILL_BOOK_DATA)];
	memset(skillBookData, 0x00, sizeof(skillBookData));

	int field = 1;
	dbCommand->FetchUInt32(field++, pUser->m_charUniqueId);
	dbCommand->FetchByte(field++, pUser->m_charType);
	dbCommand->FetchByte(field++, pUser->m_height);
	dbCommand->FetchUInt32(field++, pUser->m_hair);
	dbCommand->FetchUInt32(field++, pUser->m_face);
	dbCommand->FetchUInt32(field++, pUser->m_level);
	dbCommand->FetchUInt64(field++, pUser->m_exp);
	dbCommand->FetchByte(field++, pUser->m_zone);
	dbCommand->FetchBinary(field++, items, sizeof(items));
	dbCommand->FetchBinary(field++, skillBookData, sizeof(skillBookData));
	dbCommand->FetchByte(field++, pUser->m_class);
	dbCommand->FetchUInt16(field++, pUser->m_clanId);
	dbCommand->FetchUInt32(field++, pUser->m_fame);
	dbCommand->FetchUInt32(field++, pUser->m_injury);
	dbCommand->FetchUInt16(field++, pUser->m_curHp);
	dbCommand->FetchUInt16(field++, pUser->m_curChi);
	dbCommand->FetchUInt16(field++, pUser->m_stats[STAT_STR]);
	dbCommand->FetchUInt16(field++, pUser->m_stats[STAT_DEX]);
	dbCommand->FetchUInt16(field++, pUser->m_stats[STAT_INT]);
	dbCommand->FetchUInt16(field++, pUser->m_stats[STAT_WIND]);
	dbCommand->FetchUInt16(field++, pUser->m_stats[STAT_WATER]);
	dbCommand->FetchUInt16(field++, pUser->m_stats[STAT_FIRE]);
	dbCommand->FetchUInt32(field++, pUser->m_spentSkillPoints);
	dbCommand->FetchUInt64(field++, pUser->m_gold);
	pUser->m_curPosX = (float)(dbCommand->FetchDouble(field++));
	pUser->m_curPosZ = (float)(dbCommand->FetchDouble(field++));
	pUser->m_curPosY = (float)(dbCommand->FetchDouble(field++));
	dbCommand->FetchString(field++, (char*)pUser->m_skillTabs, sizeof(pUser->m_skillTabs));
	dbCommand->FetchUInt16(field++, pUser->m_questCount);
	dbCommand->FetchBinary(field++, pUser->m_quests, sizeof(pUser->m_quests));
	pUser->m_showHelm = dbCommand->FetchByte(field++) == 1 ? true : false;
	pUser->m_showMask = dbCommand->FetchByte(field++) == 1 ? true : false;
	dbCommand->FetchByte(field++, pUser->m_showHtItems);
	dbCommand->FetchByte(field++, pUser->m_activeWeapon);

	ByteBuffer itemBuf;
	itemBuf.append(items, sizeof(items));

	for (int i = 0; i <= IS_END; i++)
	{
		uint32 itemId;
		int16 count;
		uint8 upgradeAmount;
		uint8 upgs[15];
		uint8 holeCount;
		uint8 holes[15];

		itemBuf >> itemId >> count;
		itemBuf >> upgradeAmount;
		foreach_array_n(i, upgs, MAX_ITEM_UPGRADE_AMOUNT)
			itemBuf >> upgs[i];
		itemBuf >> holeCount;
		foreach_array_n(i, holes, MAX_ITEM_UPGRADE_AMOUNT)
			itemBuf >> holes[i];
		_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(itemId);

		if (pTable == NULL || count < 1)
			continue;

		if (!pTable->m_stackable && count > 1)
			count = 1;
		else if (count > MAX_ITEM_STACK)
			count = MAX_ITEM_STACK;

		pUser->m_itemArray[i].itemId = itemId;
		pUser->m_itemArray[i].count = count;
		pUser->m_itemArray[i].upgradeCount = upgradeAmount;
		memmove(pUser->m_itemArray[i].upgrades, upgs, sizeof(upgs));
		pUser->m_itemArray[i].holeCount = holeCount;
		memmove(pUser->m_itemArray[i].holes, holes, sizeof(holes));
		//pUser->m_itemArray[i].duration = duration; ????
	}

	ByteBuffer skillBookBuf;
	skillBookBuf.append(skillBookData, sizeof(skillBookData));

	for (int i = 0; i < MAX_EXPANDABLE_SKILLBOOKS; i++)
	{
		uint32 skillBookId;
		uint8 specialBookPt;
		uint8 ptsSpent;

		skillBookBuf >> skillBookId;

		_SKILL_BOOK_DATA* pData = g_main->GetSkillBookData(skillBookId);
		if (pData == NULL)
		{
			memset(&pUser->m_skillBookArray[i], 0x00, sizeof(struct _PLAYER_SKILL_BOOK_DATA));
			continue;
		}

		pUser->m_skillBookArray[i].Initialize(pData);

		for (int j = 0; j < SPECIAL_PT_TYPE_END; j++)
		{
			skillBookBuf >> specialBookPt;
			pUser->m_skillBookArray[i].m_specialBookStats[j] = specialBookPt;
		}
		for (int j = 0; j < MAX_SKILLS_IN_BOOK; j++)
		{
			skillBookBuf >> ptsSpent;
			//TODO: Add a check for skill pts in non existing skills, refund them if that's the case. Hard to do, but if we change any skill this will be fucked. 
			//We'd have to refund all skill pts if it happens. Which is what we might have to do in the end.
			pUser->m_skillBookArray[i].m_skillData[j].m_pointsSpent = ptsSpent;
		}
	}

	return true;
}

bool CDBAgent::LoadWarehouseData(std::string& accountId, _USER_DATA* pUser)
{
	auto_ptr<OdbcCommand> dbCommand(m_GameDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return false;

	if (pUser == NULL) //TODO: Add logout check thing
		return false;

	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)accountId.c_str(), accountId.length());

	if (!dbCommand->Execute(_T(SQL_SEL_WAREHOUSEDATA)))
	{
		g_main->PrintSQLError(m_GameDB.GetError());
		return false;
	}

	if (!dbCommand->hasData())
		return false;

	char items[(MAX_WAREHOUSE_SLOTS * 2) * 38];
	memset(items, 0x00, sizeof(items));

	dbCommand->FetchUInt64(1, pUser->m_warehouseGold);
	dbCommand->FetchBinary(2, items, sizeof(items));

	ByteBuffer itemBuf;
	itemBuf.append(items);

	memset(pUser->m_warehouseArray, 0x00, sizeof(pUser->m_warehouseArray));

	for (int i = 0; i < MAX_WAREHOUSE_SLOTS * 2; i++)
	{
		uint32 itemId;
		uint16 count;
		uint8 upgradeAmount;
		uint8 upgs[15];
		uint8 holeCount;
		uint8 holes[15];

		itemBuf >> itemId >> count;
		itemBuf >> upgradeAmount;
		foreach_array_n(i, upgs, MAX_ITEM_UPGRADE_AMOUNT)
			itemBuf >> upgs[i];
		itemBuf >> holeCount;
		foreach_array_n(i, holes, MAX_ITEM_UPGRADE_AMOUNT)
			itemBuf >> holes[i];
		_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(itemId);

		if (pTable == NULL || count < 1)
			continue;

		if (!pTable->m_stackable && count > 1)
			count = 1;
		else if (count > MAX_ITEM_STACK)
			count = MAX_ITEM_STACK;

		pUser->m_warehouseArray[i].itemId = itemId;
		pUser->m_warehouseArray[i].count = count;
		pUser->m_warehouseArray[i].upgradeCount = upgradeAmount;
		memmove(pUser->m_warehouseArray[i].upgrades, upgs, sizeof(upgs));
		pUser->m_warehouseArray[i].holeCount = holeCount;
		memmove(pUser->m_warehouseArray[i].holes, holes, sizeof(holes));
	}

	return true;
}

void CDBAgent::UpdateServerUserCount(uint8 serverTab, uint8 serverId, uint16 userCount)
{
	auto_ptr<OdbcCommand> dbCommand(m_GameDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return;

	if (!dbCommand->Execute(_T(string_format(SQL_UPD_SERVER_USER_COUNT, userCount, serverTab, serverId))))
	{
		g_main->PrintSQLError(m_GameDB.GetError());
		return;
	}
}

void CDBAgent::UpdateAllServerUserCount(uint16 userCount)
{
	auto_ptr<OdbcCommand> dbCommand(m_GameDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return;

	if (!dbCommand->Execute(_T(string_format(SQL_UPD_ALL_SERVER_USER_COUNT, userCount))))
	{
		g_main->PrintSQLError(m_GameDB.GetError());
		return;
	}
}

void CDBAgent::SaveUserInventory(_USER_DATA* pUser)
{
	auto_ptr<OdbcCommand> dbCommand(m_GameDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return;

	if (pUser == NULL) //TODO: Add logout check thing
		return;

	ByteBuffer itemBuf;
	for (int i = 0; i < IS_END; i++)
	{
		_ITEM_DATA* pData = &pUser->m_itemArray[i];
		itemBuf << pData->itemId << pData->count << pData->upgradeCount;
		foreach_array_n(j, pData->upgrades, MAX_ITEM_UPGRADE_AMOUNT)
			itemBuf << pData->upgrades[j];
		itemBuf << pData->holeCount;
		foreach_array_n(j, pData->holes, MAX_ITEM_UPGRADE_AMOUNT)
			itemBuf << pData->holes[j];
	}

	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)itemBuf.contents(), itemBuf.size(), SQL_BINARY);
	dbCommand->AddParameter(SQL_PARAM_INPUT, pUser->m_charId, std::string(pUser->m_charId).length());

	if (!dbCommand->Execute(_T(SQL_UPDATE_INVENTORY)))
	{
		g_main->PrintSQLError(m_GameDB.GetError());
		return;
	}
}

void CDBAgent::SaveUser(_USER_DATA* pUser)
{
	auto_ptr<OdbcCommand> dbCommand(m_GameDB.CreateCommand());
	if (dbCommand.get() == NULL)
		return;

	if (pUser == NULL)
		return;

	ByteBuffer skillBookBuf;
	for (int i = 0; i < MAX_EXPANDABLE_SKILLBOOKS; i++)
	{
		_PLAYER_SKILL_BOOK_DATA* pData = &pUser->m_skillBookArray[i];
		skillBookBuf << pData->m_skillBookId;
		for (int j = 0; j < SPECIAL_PT_TYPE_END; j++)
			skillBookBuf << pData->m_specialBookStats[j];
		for (int j = 0; j < MAX_SKILLS_IN_BOOK; j++)
			skillBookBuf << pData->m_skillData[j].m_pointsSpent;
	}

	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)skillBookBuf.contents(), skillBookBuf.size(), SQL_BINARY);
	dbCommand->AddParameter(SQL_PARAM_INPUT, pUser->m_charId, std::string(pUser->m_charId).length());

	if (!dbCommand->Execute(_T(string_format(SQL_UPDATE_CHARACTER, pUser->m_charType, pUser->m_level, pUser->m_exp, pUser->m_zone, pUser->m_class, pUser->m_clanId, pUser->m_fame,
		pUser->m_injury, pUser->m_curHp, pUser->m_curChi, pUser->m_stats[STAT_STR], pUser->m_stats[STAT_DEX], pUser->m_stats[STAT_INT], pUser->m_stats[STAT_WIND],
		pUser->m_stats[STAT_WATER], pUser->m_stats[STAT_FIRE], pUser->m_spentSkillPoints, pUser->m_gold, pUser->m_curPosX, pUser->m_curPosZ, pUser->m_curPosY, pUser->m_showHelm ? 1 : 0, pUser->m_showMask ? 1 : 0,
		pUser->m_showHtItems, pUser->m_activeWeapon))))
	{
		g_main->PrintSQLError(m_GameDB.GetError());
		return;
	}
}