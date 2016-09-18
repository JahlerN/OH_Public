#include "stdafx.h"
#include "Packets.h"

//TODO: Give the player feedback for what he's done after using a command.

CommandHandler::CommandHandler()
{
}

CommandHandler::~CommandHandler()
{
	free_command_table(s_commandTable);
}

bool CommandHandler::InitCommands()
{//TODO: Implement subcommand tables, do it by passing another commandTable, if the function parameter is null, use the commandTable listed.
	static Command<CommandHandler> commandTable[] =
	{
		{ "chat",			&CommandHandler::HandleGlobalChat,			"Sends a message cross server." },
		{ "help",			&CommandHandler::HandleHelp,				"Gives the player a list of all available commands and their explanation." },
		{ "test",			&CommandHandler::HandleTest,				"Dev command, do not use if you don't have access to the code." },
		//ITEM
		{ "additem",		&CommandHandler::HandleGivePlayerItem,		"Gives a player item: ItemID [, count]."},
		{ "upgitem",		&CommandHandler::HandleUpgradeItem,			"Upgrades an item: slot upgId(0-255)." },
		{ "holeitem",		&CommandHandler::HandleMakeHole,			"Adds a hole to item: slot holeId(0-255)." },
		{ "finditem",		&CommandHandler::HandleFindItem,			"Sends a list with items containing the substring: string." },
		//ZONES
		{ "zone",			&CommandHandler::HandleZoneChange,			"Moves the player to zone: ZoneID"},
		{ "move",			&CommandHandler::HandleTeleport,			"Moves the player to pos X Z: posX posZ" },
		{ "start",			&CommandHandler::HandleHome,				"Teleports you to a zones home position if it exists." },
		//CHARACTER
		{ "morph",			&CommandHandler::HandleMorph,				"Morphs the player into: npcID." },
		{ "demorph",		&CommandHandler::HandleDemorph,				"Removes the current morph." },
		{ "setlevel",		&CommandHandler::HandleSetLevel,			"Changes player level to: levelNum." },
		{ "modspeed",		&CommandHandler::HandleModSpeed,			"Changes the player movement speed to: speedNum." },
		{ "resetstats",		&CommandHandler::HandleResetStats,			"Removes all alocated stats." },
		{ "heal",			&CommandHandler::HandleHeal,				"Heals target to full or to specified amount. HP [, Chi]" },
		{ "addgold",		&CommandHandler::HandleAddGold,				"Gives user gold. GOLD" },
		//GM STUFF
		{ "announce",		&CommandHandler::HandleGMAnnounce,			"Sends a message to all players, displayed at the top of the screen." },
		{ "reload",			&CommandHandler::HandleReload,				"Reloads db tables, only for Administrators/Devs!" },
	};

	init_command_table(CommandHandler, commandTable, s_commandTable)

	return true;
}

bool CommandHandler::HandleChatCommand(Packet& pkt, CUser* pSender)
{
	std::string message;
	pkt >> message;

	if (message.size() <= 1 || message[0] != '/')
		return false;

	CommandArgs vArgs = StrSplit(message, " ");
	std::string command = vArgs.front();
	vArgs.pop_front();

	STRTOLOWER(command);

	CommandTable::iterator itr = s_commandTable.find(command.c_str() + 1);
	if (itr == s_commandTable.end())
		return false;

	//TODO: Check permission for command. Add it in the commandTable. PERMISSION::GM or w/e.

	return(this->*(itr->second->handler))(vArgs, message.c_str() + command.size() + 1, itr->second->help, pSender);
}

bool CommandHandler::HandleGlobalChat(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	Packet result(PKT_GAMESERVER_MESSAGE, uint8(MT_LOCAL));
	result << pSendUser->GetID();
	result << pSendUser->m_userData->m_charId;
	result.DByte();
	result << args;

	g_main->SendToAll(result);

	return true;
}

bool CommandHandler::HandleHelp(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (vArgs.size() == 1)
	{
		CommandTable::iterator itr = s_commandTable.find(vArgs.front().c_str());
		if (itr != s_commandTable.end())
			pSendUser->SendNotice(string_format("%s Explanation: %s", itr->first.c_str(), itr->second->help));
		else
			return true;//TODO: Send error message.
	}
	else
		for (auto c : s_commandTable)
		{
			std::string msg = string_format("Command: %s", c.first.c_str());
			pSendUser->SendNotice(msg);
		}
	return true;
}

bool CommandHandler::HandleTest(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_TO_GROUND);

	if (vArgs.size() < 1)
		return false;

	switch (atoi(vArgs.back().c_str()))
	{
	case 2:
		result << uint8(2);
		result << uint16(1); //Item drop id, 0 doesn't seem to work.
		result << uint8(1) //0 idk seems to be nothing, 1 item, 2 gold
			<< uint8(1);//Idk
		result << pSendUser->GetX() << pSendUser->GetY() << pSendUser->GetZ();
		result << uint32(99100243);//Master drac knife.
		result << uint8(0) << uint8(164);//Item rarity.
		result << uint16(1);//count.
		result << uint32(0) << uint16(0) << uint16(pSendUser->GetID()) << uint16(0);
		break;
	case 3:
		result << uint8(3);
		result << uint16(1) << uint32(-1);
		break;
	case 4:
		result << uint8(4);
		result << uint16(1);
		break;
	}

	pSendUser->Send(&result);
	return true;
}

bool CommandHandler::HandleHome(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	//TODO: Temporary way of handling the /home command, do / home to make it work. For some reason the client doesn't send the command request at all when doing /home.
	//if (vArgs.empty())
		//return false;

	//if (vArgs.front() != "home")
		//return false;

	uint8 zone = pSendUser->m_userData->m_zone;

	_ZONESTART_POSITION* pData = sObjMgr->GetZoneStartPosition(zone);
	if (pData == NULL)
		return false;

	pSendUser->GoToXZ(pData->m_x, pData->m_z);
	return true;
}

bool CommandHandler::HandleGivePlayerItem(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser)
{
	if (vArgs.size() < 1)
		return false;

	uint32 itemId = atoi(vArgs.front().c_str());
	vArgs.pop_front();

	if (pSendUser == NULL)
		return false;

	_ITEM_TABLE* pItem = sObjMgr->GetItemTemplate(itemId);
	if (pItem == NULL)
		return false;

	uint16 count = 1;
	if (!vArgs.empty())
		count = atoi(vArgs.front().c_str());

	//TODO: Send error message if unsuccessfull.(Make it bool)
	if (pSendUser->m_target->GetID() < NPC_START)
		pSendUser->m_target->GiveItem(itemId, count);

	return true;
}

bool CommandHandler::HandleUpgradeItem(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (vArgs.size() != 2)
		return false;

	uint16 slot = atoi(vArgs.front().c_str());
	vArgs.pop_front();
	uint8 upgCode = atoi(vArgs.front().c_str());


	return pSendUser->UpgradeItem(slot, upgCode);
}

bool CommandHandler::HandleMakeHole(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser)
{
	if (vArgs.size() != 2)
		return false;

	uint16 slot = atoi(vArgs.front().c_str());
	vArgs.pop_front();
	uint8 holeCode = atoi(vArgs.front().c_str());

	if (slot > IS_END - 1)
		return false;

	_ITEM_TABLE* pUpgTable = sObjMgr->GetItemTemplate(holeCode);
	if (pUpgTable == NULL)
		return false;

	_ITEM_DATA* pItem = &pSendUser->m_userData->m_itemArray[slot];
	if (pItem->itemId == 0)
		return false;

	_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(pItem->itemId);
	if (pTable == NULL)
		return false;

	pItem->HoleItem(holeCode);

	Packet result(0x57, uint8(0x02));
	result << uint8(0x7c);
	result << uint8(0x2b) << uint16(0x02D3);
	result << uint32(0);
	pSendUser->Send(&result);
	result.Initialize(0x54, uint8(0x02));
	result << uint8(0xA1) << uint8(0x0F) << uint8(0x01);
	result << pItem->itemId << uint8(0) << uint8(pTable->m_itemRarity) << pItem->count << slot;
	for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT; i++)
		result << pItem->upgrades[i];

	result << pItem->holeCount;
	for (int i = 0; i < 14; i++)
		result << pItem->holes[i];

	pSendUser->Send(&result);
	if (pSendUser->IsEquipSlot(slot))
	{
		pSendUser->UpdateItemSlotValues();
		pSendUser->UpdateUserStats();
	}
	pSendUser->SendCharacterUpdate();
	return true;
}

bool CommandHandler::HandleFindItem(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	uint16 maxItemCount = 99, count = 0;
	std::string subStr = args;
	STRTOLOWER(subStr);
	auto itemArr = sObjMgr->GetItemTableArray();
	foreach(itr, itemArr)
	{
		std::string name = itr->second->m_name;
		if (name.empty())
			continue;
		STRTOLOWER(name);
		if (HasSubString(name, subStr))
		{
			pSendUser->SendNotice(string_format("ID: %d Name: %s", itr->second->m_itemId, itr->second->m_name.c_str()));
			if (count++ == maxItemCount)
				break;
		}
	}
	return true;
}

//CLEAR INV 
//for (int i = INV_1_START; i < INV_1_START + MAX_INVENTORY_SLOTS; i++)
//{
//	memset(&m_userData->m_itemArray[i], 0x00, sizeof(_ITEM_DATA));
//	g_main->m_dbAgent.SaveUserInventory(m_userData);
//}
//SendMyInfo();

bool CommandHandler::HandleZoneChange(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (vArgs.size() != 1)// || pSendUser->IsDead())
		return false;

	uint32 zoneChangeId;
	zoneChangeId = atoi(vArgs.front().c_str());

	//For now we always go to default position when warping to a new zone.
	_ZONECHANGE_DATA* pZoneChange = new _ZONECHANGE_DATA();
	_ZONESTART_POSITION* pZoneStart = sObjMgr->GetZoneStartPosition(zoneChangeId);
	if (pZoneStart == NULL)
	{
		pZoneChange->m_posX = 0;
		pZoneChange->m_posZ = 0;
		pZoneChange->m_toZoneId = zoneChangeId;
	}
	else
	{
		pZoneChange->m_posX = pZoneStart->m_x;
		pZoneChange->m_posZ = pZoneStart->m_z;
		pZoneChange->m_toZoneId = zoneChangeId;
	}

	pSendUser->ChangeZone(pZoneChange);
	delete pZoneChange;
	return true;
}

bool CommandHandler::HandleTeleport(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (vArgs.size() != 2)// || pSendUser->IsDead())
		return false;

	//TODO: Check for valid pos.
	uint16 x, z;
	x = atoi(vArgs.front().c_str());
	vArgs.pop_front();
	z = atoi(vArgs.front().c_str());

	return pSendUser->GoToXZ(x, z);
}

bool CommandHandler::HandleGMAnnounce(CommandArgs& vArgs, const char* args, const char* description, CUser* pSendUser)
{
	Packet result(PKT_GAMESERVER_MESSAGE, uint8(MT_GM_ANNOUNCE));
	result.SByte();
	result << args;

	g_main->SendToAll(result);

	return true;
}

bool CommandHandler::HandleMorph(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (vArgs.size() != 1)
		return false;

	if (pSendUser == NULL)
		return false;

	if (pSendUser->m_target->GetID() < NPC_START)
	{
		pSendUser->m_target->m_userData->m_morphId = atoi(vArgs.front().c_str());
		pSendUser->m_target->SendMorphPlayer();
	}
	return true;
}

bool CommandHandler::HandleDemorph(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (pSendUser == NULL)
		return false;

	if (pSendUser->m_target->GetID() < NPC_START)
	{
		pSendUser->m_target->m_userData->m_morphId = 0;
		pSendUser->m_target->SendMorphPlayer();
	}

	return true;
}

bool CommandHandler::HandleSetLevel(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (vArgs.size() > 1)
		return false;

	uint32 nLevel = atoi(vArgs.front().c_str());
	if (nLevel > MAX_PLAYER_LEVEL)
		nLevel = MAX_PLAYER_LEVEL;

	if (pSendUser == NULL)
		return false;

	if (pSendUser->m_target->m_userData->m_level == nLevel)
		return true;

	pSendUser->m_target->m_userData->m_level = nLevel;
	uint8 cType = pSendUser->m_target->m_userData->m_charType;
	if (pSendUser->m_target->m_userData->m_level > 100 && cType < 60)
		pSendUser->m_target->m_userData->m_charType += 10;
	if (pSendUser->m_target->m_userData->m_level > 200 && cType < 70)
		pSendUser->m_target->m_userData->m_charType += 10;

	if (pSendUser->m_target->m_userData->m_level < 201 && cType > 70)
		pSendUser->m_target->m_userData->m_charType -= 10;
	if (pSendUser->m_target->m_userData->m_level < 101 && cType > 60)
		pSendUser->m_target->m_userData->m_charType -= 10;

	uint32 level = pSendUser->m_target->GetLevel();
	//Set the player exp to last level requirement.
	if (level == 1)
		pSendUser->m_target->m_userData->m_exp = 0;
	else
		pSendUser->m_target->m_userData->m_exp = g_main->GetExpReqByLevel(nLevel - 1);

	//To make sure we don't count stats for higher than char level items
	pSendUser->m_target->UpdateItemSlotValues();
	pSendUser->m_target->UpdateUserStats();
	pSendUser->m_target->SendMyInfo();
	pSendUser->m_target->SendCharacterUpdate();
	pSendUser->m_target->UserLeaveEnterRegion(1);

	return true;
}

bool CommandHandler::HandleModSpeed(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (vArgs.size() != 1)
		return false;

	float mSpeed = (float)atoi(vArgs.front().c_str());

	if (mSpeed < 0 || mSpeed > 59)
		return false;

	pSendUser->m_target->m_movementSpeed = mSpeed;
	pSendUser->m_target->SendCharacterUpdate();
	return true;
}

bool CommandHandler::HandleResetStats(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	_CHARACTER_DATA* pData = sObjMgr->GetCharacterInfo(pSendUser->m_userData->m_charType);

	pSendUser->m_userData->m_stats[STAT_STR] = pData->m_baseStr;
	pSendUser->m_userData->m_stats[STAT_DEX] = pData->m_baseDex;
	pSendUser->m_userData->m_stats[STAT_INT] = pData->m_baseInt;
	for (int i = STAT_WIND; i < STAT_END; i++)
		pSendUser->m_userData->m_stats[i] = 0;

	pSendUser->UpdateItemSlotValues();
	pSendUser->UpdateUserStats();
	pSendUser->SendCharacterUpdate();
	pSendUser->SendRegionCharacterUpdate();
	return true;
}

bool CommandHandler::HandleHeal(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (pSendUser == NULL || vArgs.size() > 2)
		return false;
	CUser* pTarget = pSendUser->m_target;
	if (pTarget == NULL)
		pTarget = pSendUser;

	int healAmount = 0, chiRecoveryAmount = 0;
	if (vArgs.size() > 0)
	{
		healAmount = atoi(vArgs.front().c_str());
		vArgs.pop_front();
		if (vArgs.size() > 0)
			chiRecoveryAmount = atoi(vArgs.front().c_str());
	}
	else
	{
		healAmount = pTarget->m_maxHp;
		chiRecoveryAmount = pTarget->m_maxChi;
	}


	pTarget->HpChange(healAmount, pSendUser);
	pTarget->ChiChange(chiRecoveryAmount);
	return true;
}

bool CommandHandler::HandleAddGold(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	if (pSendUser == NULL || vArgs.size() != 1)
		return false;

	CUser* pTarget = pSendUser->m_target;
	if (pTarget == NULL)
		pTarget = pSendUser;

	int goldAmount = 0;
	goldAmount = atoi(vArgs.front().c_str());

	pTarget->GoldChange(goldAmount);

	return true;
}

bool CommandHandler::HandleReload(CommandArgs & vArgs, const char * args, const char * description, CUser * pSendUser)
{
	//sObjMgr->LoadItemSet();
	//sObjMgr->LoadItemTemplate();
	//sObjMgr->LoadItemDropTable();
	//sObjMgr->LoadGamblingItemTable();
	//sObjMgr->LoadLevelData();
	//sObjMgr->LoadZoneChangeTable();
	//sObjMgr->LoadZoneStartPositionTable();
	//sObjMgr->LoadCharacterInfoTable();
	sObjMgr->LoadNpcGossipTable();
	sObjMgr->LoadGossipOptionTable();
	//sObjMgr->LoadNpcInfoTable();
	//sObjMgr->LoadNpcGroupTable();
	//sObjMgr->LoadSkillTable();
	//sObjMgr->LoadSkillBookTable();
	//sObjMgr->LoadShopTable();
	//sObjMgr->LoadShopItemTable();

	return true;
}