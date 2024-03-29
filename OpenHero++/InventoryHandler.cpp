#include "stdafx.h"
#include "Packets.h"

//TODO: Write a class for the inventory instead.
bool CUser::CanEquipItem(_ITEM_TABLE* pTable)
{
	if (pTable == NULL || GetLevel() < pTable->m_reqLevel || GetLevel() > pTable->m_maxLevel && pTable->m_maxLevel != 0)
		return false;

	if (m_userData->m_stats[STAT_STR] < pTable->m_strReq ||
		m_userData->m_stats[STAT_DEX] < pTable->m_dexReq ||
		m_userData->m_stats[STAT_INT] < pTable->m_intReq)
		return false;

	//Anyone can wear, common class
	if (pTable->m_reqClass == 0 || pTable->m_reqClass == 1 || pTable->m_reqClass == m_userData->m_class)
		return true;

	//TODO: I still don't like this, but what do. We could add a common weaponRequirement in the charData table.
	//Special check for rod/spear, it's a shared class
	if (pTable->m_reqClass == 58)
	{
		if (m_userData->m_charType + 2 == WT_SPEAR_ROD ||
			m_userData->m_charType - 8 == WT_SPEAR_ROD ||
			m_userData->m_charType - 18 == WT_SPEAR_ROD ||
			m_userData->m_charType + 1 == WT_SPEAR_ROD ||
			m_userData->m_charType - 9 == WT_SPEAR_ROD ||
			m_userData->m_charType - 19 == WT_SPEAR_ROD)
			return true;
	}

	//Special check for sword/blade, it's a shared class
	if (pTable->m_reqClass == 55)
	{
		if (m_userData->m_charType + 2 == WT_SWORD_BLADE ||
			m_userData->m_charType - 8 == WT_SWORD_BLADE ||
			m_userData->m_charType - 18 == WT_SWORD_BLADE ||
			m_userData->m_charType + 1 == WT_SWORD_BLADE ||
			m_userData->m_charType - 9 == WT_SWORD_BLADE ||
			m_userData->m_charType - 19 == WT_SWORD_BLADE)
			return true;
	}

	if (m_userData->m_charType != pTable->m_reqClass &&
		m_userData->m_charType - 10 != pTable->m_reqClass &&
		m_userData->m_charType - 20 != pTable->m_reqClass)
		return false;

	return true;
}

uint16 CUser::CountFreeInventorySlots()
{
	uint16 freeSlots = 0;
	for (int i = 0; i < IS_END; i++)
	{
		if (IsEquipSlot(i))
			continue;

		if (m_userData->m_itemArray[i].itemId == 0)
			freeSlots++;
	}

	return freeSlots;
}

void CUser::HandleOpenMultiContainerItem(uint16 slot)
{
	std::list<_ITEM_DATA*> dropList;

	_ITEM_DATA* pItem = &m_userData->m_itemArray[slot];
	if (pItem == NULL)
		goto return_fail;
	uint32 itemId = pItem->itemId;

	_GAMBLING_ITEM_TABLE* pGamble = sObjMgr->GetGamblingItemTable(itemId);
	if (pGamble == NULL)
		goto return_fail;

	if (pGamble->m_dropTable == NULL)
		goto return_fail;

	//Hero does it like this, need to have atleast 5 slots if the item will add max 5 items
	if (pGamble->m_dropTable->m_availableDrops > CountFreeInventorySlots())
		goto return_fail;

	if (pGamble->m_openingCost > m_userData->m_gold)
		goto return_fail;

	GoldChange(-pGamble->m_openingCost, false);

	//Item didn't exist if it's < 0!
	int16 newSLot = RemoveItem(itemId);

	if (newSLot < 0)
		goto return_fail;

	_ITEM_TABLE* pTable = NULL;
	_ITEM_DATA* pItem2 = NULL;
	for (int i = 0; i < NUM_DROPS_IN_DROP_TABLE; i++)
	{
		if (pGamble->m_dropTable->m_dropableId[i] == 0)
			break;

		pTable = sObjMgr->GetItemTemplate(pGamble->m_dropTable->m_dropableId[i]);
		if (pTable == NULL)
			continue;

		pItem2 = new _ITEM_DATA();
		pItem2->itemId = pTable->m_itemId;
		pItem2->count = 1;
		pItem2->expirationTime = pTable->m_duration;

		dropList.push_back(pItem2);
	}
	uint32 newItemId = 0;
	foreach(itr, dropList)
	{
		newItemId = (*itr)->itemId;
		int16 slot = FindFreeInventorySlot(newItemId, 1);
		_ITEM_DATA* pNewItem = AddItemToSlot((*itr), slot);
		SendItemAddedToSlot(newItemId, sObjMgr->GetItemTemplate(newItemId)->m_itemRarity, pNewItem->count, slot);
	}

	SendRemoveMultiSlotContainer(itemId, slot);
	return;

return_fail:
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_OPEN_MULTI_ITEM));
	result << uint8(0xAF) << uint8(0x03);
	Send(&result);
}

void CUser::SendItemAddedToSlot(uint32 itemId, uint8 rarity, uint16 count, int16 slot)
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS);
	result << uint8(0x0A)
		<< itemId //TODO: Figure this value out, changes on some packets(Multi item opening.txt for info)
		<< uint8(0) << rarity
		<< count << slot 
		<< uint32(0);
	
	Send(&result);
}

void CUser::SendRemoveMultiSlotContainer(uint32 itemId, int16 slot)
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_OPEN_MULTI_ITEM));
	result << uint8(0x0A) << uint8(0)
		<< itemId << slot << uint16(0);

	Send(&result);
}

bool CUser::IsEquipSlot(uint16 slot)
{
	//TODO: Currently ignoring pet slots
	//TODO: Get back to this function, it's not looking great.
	if (slot < INV_1_START || slot >= CASHSHOP_EQUIP_START && slot < PET_ARMOR_START + PET_ARMOR_SLOT + 1 || slot > INV_2_END && slot < IS_END)
		return true;
	return false;
}

bool CUser::HasItemEquipped(uint32 itemId)
{
	if (itemId == 0)
		return false;

	_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(itemId);
	if (pTable == nullptr)
		return false;

	for (int i = 0; i < EQUIP_SLOT - 1; i++)//No pet can't have set bonus.
	{
		if (m_userData->m_itemArray[i].itemId == itemId)
			return true;
	}

	for (int i = CASHSHOP_EQUIP_START; i < CASHSHOP_EQUIP_START + CASHSHOP_EQUIP_SLOT; i++)//No pet can't have set bonus.
	{
		if (m_userData->m_itemArray[i].itemId == itemId)
			return true;
	}

	for (int i = SECONDARY_ACC_START; i < SECONDARY_ACC_START + SECONDARY_ACC_SLOT; i++)//No pet can't have set bonus.
	{
		if (m_userData->m_itemArray[i].itemId == itemId)
			return true;
	}

	for (int i = PET_ARMOR_START; i < PET_ARMOR_START + PET_ARMOR_SLOT; i++)//No pet can't have set bonus.
	{
		if (m_userData->m_itemArray[i].itemId == itemId)
			return true;
	}

	for (int i = FIVE_ELEMENT_START; i < FIVE_ELEMENT_START + FIVE_ELEMENT_SLOT; i++)
	{
		if (m_userData->m_itemArray[i].itemId == itemId)
			return true;
	}

	return false;
}

bool CUser::HasItemCount(uint32 itemId, uint16 count)
{
	for (int i = 0; i < IS_END; i++)
	{
		if (m_userData->m_itemArray[i].itemId == itemId && m_userData->m_itemArray[i].count >= count)
			return true;
	}
	return false;
}

bool CUser::HasItem(uint32 itemId)
{
	for (int i = 0; i < IS_END; i++)
	{
		if (m_userData->m_itemArray[i].itemId == itemId && m_userData->m_itemArray[i].count > 0)
			return true;
	}
	return false;
}

void CUser::HandleMoveItem(Packet& pkt)
{
	uint32 itemId;
	int16 oldPos, newPos;
	uint16 oldCount, newCount;
	uint16 aPos, bPos;
	uint8 subOpcode, unk;
	_ITEM_DATA* oItem, *nItem;
	pkt >> subOpcode;

	switch (subOpcode)
	{
	case MOVE_ITEM_PICKUP:
		uint16 regionDropId;
		uint8 type;
		pkt >> type >> regionDropId >> newPos;//Sends pos too, but we will not care about that at all. We check that ourselves okay.

		if (type == 1)//Item
		{
			_ZONE_ITEM* pZoneItem = GetMapInstance()->LootDroppedItemById(regionDropId, GetRegionX(), GetRegionZ());
			if (pZoneItem == NULL || pZoneItem->m_pItem == NULL)
				return;

			int freeInvSlot = FindFreeInventorySlot(pZoneItem->m_pItem->itemId, pZoneItem->m_pItem->count);
			if (freeInvSlot <= -1)//No free slots..
				return;

			if (sObjMgr->GetItemTemplate(pZoneItem->m_pItem->itemId)->m_stackable)
				GiveItem(pZoneItem->m_pItem, freeInvSlot);
			else
				GiveItem(pZoneItem->m_pItem);
			pZoneItem->m_itemDeleted = true;
			GetMapInstance()->RegionItemRemove(pZoneItem);
		}
		else if (type == 2)//Gold pile;
			return;//GoldChange(regionDropId);
		break;
	case MOVE_ITEM_REMOVE:
	{
		pkt >> unk >> itemId >> oldPos;
		if (oldPos >= IS_END)
			goto return_fail;
		oItem = &m_userData->m_itemArray[oldPos];
		if (oItem->itemId == 0 || itemId != oItem->itemId)
			goto return_fail;

		RemoveItem(oldPos, oItem->count);

		if (IsEquipSlot(oldPos))
		{
			UpdateItemSlotValues();
			UpdateUserStats();
			SendCharacterUpdate();
		}
	}
	break;
	case MOVE_ITEM_EMPTY:
	{
		pkt >> itemId >> oldPos >> newPos;
		if (oldPos >= IS_END || newPos >= IS_END)
			goto return_fail;
		oItem = &m_userData->m_itemArray[oldPos];
		if (IsEquipSlot(newPos) && !CanEquipItem(sObjMgr->GetItemTemplate(oItem->itemId)))
			goto return_fail;

		nItem = &m_userData->m_itemArray[newPos];
		if (nItem->itemId != 0 || oItem->itemId != itemId)
			goto return_fail;
		memmove(nItem, oItem, sizeof(_ITEM_DATA));
		oItem->Initialize();
		SendMoveItem(itemId, oldPos, newPos);

		UpdateItemSlotValues();
		UpdateUserStats();

		if (IsEquipSlot(newPos) || IsEquipSlot(oldPos))
		{
			SendVisibleItemListToRegion();
			SendUnk75();
			SendCharacterUpdate();
			SendRegionCharacterUpdate();
		}
	}
	break;
	case MOVE_USE_ITEM:
	{
		pkt >> itemId >> oldPos;
		if (oldPos > 1000)
			goto return_fail;
		oItem = &m_userData->m_itemArray[oldPos];
		if (oItem->itemId == 0 || itemId != oItem->itemId)
			goto return_fail;
		HandleUseItem(oItem);
	}
	break;
	case MOVE_ITEM_VISIBILITY:
	{
		if (IsMeditating())
			return;
		uint8 switchWeaponTo;
		pkt >> switchWeaponTo;
		HandleSwitchActiveWeapon(switchWeaponTo);
	}
	break;
	case MOVE_ITEM_ITEM_STACK:
	{
		pkt >> oldPos >> newPos;
		if (oldPos >= IS_END || newPos >= IS_END)
			goto return_fail;
		oItem = &m_userData->m_itemArray[oldPos];
		nItem = &m_userData->m_itemArray[newPos];

		if (oItem->itemId == nItem->itemId)
		{
			if (oItem->count + nItem->count >= MAX_ITEM_STACK)
			{
				uint16 tCount = oItem->count;
				oItem->count = nItem->count;
				nItem->count = tCount;
				oldCount = nItem->count;
				newCount = oItem->count;
			}
			else
			{
				nItem->count += oItem->count;
				oldCount = 0;
				newCount = nItem->count;
				oItem->Initialize();
			}
		}
		else
		{
			memcpy(nItem, oItem, sizeof(_ITEM_DATA));
			oItem->Initialize();
			oldCount = 0;
			newCount = nItem->count;
		}

		SendMoveItemStack(nItem->itemId, oldCount, newCount, oldPos, newPos);
	}
	break;
	case MOVE_ITEM_ITEM:
	{
		uint32 newItemId;
		pkt >> unk >> itemId >> newPos >> oldPos >> newItemId;

		if (oldPos >= IS_END || newPos >= IS_END)
			goto return_fail;
		oItem = &m_userData->m_itemArray[oldPos];
		if (IsEquipSlot(newPos) && !CanEquipItem(sObjMgr->GetItemTemplate(oItem->itemId)))
			goto return_fail;

		nItem = &m_userData->m_itemArray[newPos];
		if (IsEquipSlot(oldPos) && !CanEquipItem(sObjMgr->GetItemTemplate(nItem->itemId)))
			goto return_fail;

		if (oItem->itemId != newItemId || nItem->itemId != itemId)
			goto return_fail;

		_ITEM_DATA iTemp;
		memcpy(&iTemp, oItem, sizeof(_ITEM_DATA));
		memmove(oItem, nItem, sizeof(_ITEM_DATA));
		memmove(nItem, &iTemp, sizeof(_ITEM_DATA));

		UpdateItemSlotValues();
		UpdateUserStats();

		SendMoveItemItem(itemId, newItemId, oldPos, newPos);
		SendUnk75();
		SendVisibleItemListToRegion();
		break;
	}
	case MOVE_OPEN_GAMBLECHEST:
		pkt >> itemId;//3 unk bytes after this.
		HandleGambleItem(itemId);
		break;
	case MOVE_ITEM_SPLIT:
	{
		pkt >> oldPos >> newPos >> newCount;
		if (oldPos >= IS_END || newPos >= IS_END)
			goto return_fail;
		oItem = &m_userData->m_itemArray[oldPos];
		nItem = &m_userData->m_itemArray[newPos];

		if (nItem->itemId != 0)
			goto return_fail;

		if (newCount > oItem->count)
			return;

		oItem->count -= newCount;
		memcpy(nItem, oItem, sizeof(oItem));
		nItem->count = newCount;

		SendMoveItemSplitStack(sObjMgr->GetItemTemplate(oItem->itemId), oItem->count, newCount, oldPos, newPos);
		if (newCount == oItem->count + newCount)
			oItem->Initialize();
	}
	break;
	case MOVE_ITEM_OPEN_MULTI_ITEM:
		pkt >> oldPos;
		HandleOpenMultiContainerItem(oldPos);
	break;
	case MOVE_ITEM_SHOW:
	{
		HandleShowItemChange(pkt);
	}
	break;
	case MOVE_HAMMER_DELETE:
	{
		uint32 itemId;
		uint16 oldPos, slot;
		uint8 unk1, unk2;
		pkt >> itemId >> oldPos >> unk1 >> unk2;
		if (oldPos >= IS_END)
			goto return_fail;
		oItem = &m_userData->m_itemArray[oldPos];
		if (oItem->itemId == 0 || itemId != oItem->itemId)
			goto return_fail;

		oldCount = oItem->count;
		oItem->Initialize();
		SendRemoveItem(itemId, oldPos);
		SendUpdateItemSlot(sObjMgr->GetItemTemplate(itemId), oldCount);

		if (IsEquipSlot(oldPos))
		{
			UpdateItemSlotValues();
			UpdateUserStats();
			SendCharacterUpdate();
			SendRegionCharacterUpdate();
		}
	}
	break;
	case 11:
	case 12:
	default:
		printf("Unkown inventory packet recieved: %d", subOpcode);
		goto return_fail;//So we don't freeze client inv.
		break;
	}

	return;

return_fail:
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(subOpcode));
	result << uint8(0);
	Send(&result);
}

void CUser::HandleShowItemChange(Packet& pkt)
{
	uint8 type, show;
	pkt >> type >> show;

	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_SHOW));
	result << uint8(0x0A) << uint8(0x00);

	//Enable/disable helm 00 disable 01 enable
	if (type == 0x02)
	{
		bool sHelm = show == 0 ? false : true;
		m_userData->m_showHelm = sHelm;
		if (m_userData->m_showHtItems & 0x01) //TODO: This shounldn't be needed when 59 05 works
			result << type << 0;
		else
			result << type << show;
	}

	//Enable/disable mask 00 disable 01 enable
	if (type == 0x03)
	{
		bool sMask = show == 0 ? false : true;
		m_userData->m_showMask = sMask;
		if (m_userData->m_showHtItems & 0x02)
			result << type << 0;
		else
			result << type << show;
	}

	if (type == 0x01)
	{
		if (show >= 0x20)
			return;
		m_userData->m_showHtItems = show;
		result << type << show;
	}
	Send(&result);

	SendVisibleItemListToRegion();
}

void CUser::SendVisibleItemListToRegion()
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_VISIBILITY));
	result << uint8(0x0A) << uint8(0x00) << GetID() << m_userData->m_activeWeapon; // Last is which wep to show, (3, left 4 right)
	result.append(GetVisibleItemList());

	SendToRegion(result, NULL);
}

//Note about this function, it's shit. i hate it. But it has to be like this because hero wants only the equipped items UNLESS you have 1 HT item visible. Then it wants all slots, besides weapons(if they're empty).
ByteBuffer CUser::GetVisibleItemList()
{
	ByteBuffer result;
	uint8 counterPos = result.wpos();
	result << uint8(0);//Item count placeholder
	uint8 count = 0;
	if (m_userData->m_showHtItems > 0)
		count = 5;

	_ITEM_DATA* pItem = NULL;
	_ITEM_TABLE* pTable = NULL;
	ItemSlot is = IS_END;

	if (m_userData->m_showHtItems & SHOW_HT_HELM)
		is = IS_CSHELM;
	else if (m_userData->m_showHelm)
		is = IS_HELM;
	if (is != IS_END)
	{
		pItem = &m_userData->m_itemArray[is];
		pTable = sObjMgr->GetItemTemplate(pItem->itemId);
		result << pItem->itemId << uint16(is) << uint8(pTable != nullptr ? pTable->m_itemRarity : 0) << pItem->upgradeCount << uint32(0x00);
		is = IS_END;
	}
	else if (count >= 5)
		result << uint16(0x00) << uint8(0x00) << uint16(IS_HELM) << uint8(0x00) << uint8(0x00) << uint32(0x00);

	if (m_userData->m_showHtItems & SHOW_HT_MASK)
		is = IS_CSMASK;
	else if (m_userData->m_showMask)
		is = IS_MASK;
	if (is != IS_END)
	{
		pItem = &m_userData->m_itemArray[is];
		pTable = sObjMgr->GetItemTemplate(pItem->itemId);
		result << pItem->itemId << uint16(is) << uint8(pTable != nullptr ? pTable->m_itemRarity : 0) << pItem->upgradeCount << uint32(0x00);
		is = IS_END;
	}
	else if (count >= 5)
		result << uint16(0x00) << uint8(0x00) << uint16(IS_MASK) << uint8(0x00) << uint8(0x00) << uint32(0x00);

	if (m_userData->m_showHtItems & SHOW_HT_CLANARMOR)
		is = IS_CLANARMOR;
	else if (m_userData->m_showHtItems & SHOW_HT_ARMOR)
		is = IS_CSARMOR;
	else if (m_userData->m_itemArray[IS_ARMOR].itemId != 0)
		is = IS_ARMOR;

	if (is != IS_END)
	{
		pItem = &m_userData->m_itemArray[is];
		pTable = sObjMgr->GetItemTemplate(pItem->itemId);
		result << pItem->itemId << uint16(is) << uint8(pTable != nullptr ? pTable->m_itemRarity : 0) << pItem->upgradeCount << uint32(0x00);
		is = IS_END;
	}
	else if (count >= 5)
		result << uint16(0x00) << uint8(0x00) << uint16(IS_MASK) << uint8(0x00) << uint8(0x00) << uint32(0x00);

	pItem = &m_userData->m_itemArray[IS_WEAPON1];
	pTable = sObjMgr->GetItemTemplate(pItem->itemId);
	if (pTable != NULL)
	{
		result << pItem->itemId << uint16(IS_WEAPON1) << uint8(pTable != nullptr ? pTable->m_itemRarity : 0) << pItem->upgradeCount << uint32(0x00);
		count++;
	}

	pItem = &m_userData->m_itemArray[IS_WEAPON2];
	pTable = sObjMgr->GetItemTemplate(pItem->itemId);
	if (pTable != NULL)
	{
		result << pItem->itemId << uint16(IS_WEAPON2) << uint8(pTable != nullptr ? pTable->m_itemRarity : 0) << pItem->upgradeCount << uint32(0x00);
		count++;
	}

	if (m_userData->m_showHtItems & SHOW_HT_BOOTS)
		is = IS_CSBOOTS;
	else if (m_userData->m_itemArray[IS_BOOTS].itemId != 0)
		is = IS_BOOTS;

	if (is != IS_END)
	{
		pItem = &m_userData->m_itemArray[is];
		pTable = sObjMgr->GetItemTemplate(pItem->itemId);
		result << pItem->itemId << uint16(is) << uint8(pTable != nullptr ? pTable->m_itemRarity : 0) << pItem->upgradeCount << uint32(0x00);
		is = IS_END;
	}
	else if (count >= 5)
		result << uint16(0x00) << uint8(0x00) << uint16(IS_MASK) << uint8(0x00) << uint8(0x00) << uint32(0x00);

	pItem = &m_userData->m_itemArray[IS_PET];
	pTable = sObjMgr->GetItemTemplate(pItem->itemId);
	result << pItem->itemId << uint16(IS_PET) << uint8(pTable != nullptr ? pTable->m_itemRarity : 0) << pItem->upgradeCount << uint32(0x00);

	result.put(counterPos, uint8(count));
	return result;
}

void CUser::SendAddItemSilent(_ITEM_DATA* pItem, _ITEM_TABLE* pTable, uint16 slot)
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(0x0A));
	result << pItem->itemId
		<< uint8(0)
		<< pTable->m_itemRarity
		<< uint16(1)
		<< uint16(slot);
	//Hero just wants this if the rarity is that high.
	if (pTable->m_itemRarity > 161 || pTable->m_itemRarity == 0)
	{
		foreach_array_n(j, pItem->upgrades, MAX_ITEM_UPGRADE_AMOUNT)
			result << pItem->upgrades[j];
		result << pItem->holeCount;
		foreach_array_n(j, pItem->holes, MAX_ITEM_UPGRADE_AMOUNT - 1)
			result << pItem->holes[j];
	}
	result << uint32(0);

	Send(&result);
}

bool CUser::UpgradeItem(uint16 slot, uint8 upgCode)
{
	if (slot > IS_END - 1)
		return false;

	_ITEM_TABLE* pUpgTable = sObjMgr->GetItemTemplate(upgCode);
	if (pUpgTable == NULL)
		return false;

	_ITEM_DATA* pItem = &m_userData->m_itemArray[slot];
	if (pItem->itemId == 0)
		return false;

	_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(pItem->itemId);
	if (pTable == NULL)
		return false;

	pItem->UpgradeItem(upgCode);

	//This is what was opening a bunch of windows.
	/*Packet result(0x57, uint8(0x02));
	result << uint8(0x7c);
	result << uint8(0x2b) << uint16(0x02D3);
	result << uint32(0);
	Send(&result);*/

	SendUpgradeItemResult(2, slot, pTable);
	SendCharacterUpdate();
	SendVisibleItemListToRegion();

	return true;
}

//Find first stackable slot, or if none exists, give the first found empty slot. If no slot, return -1;
int CUser::FindFreeInventorySlot(uint32 itemId /*= 0*/, uint16 count /*= 1*/)
{
	uint16 maxSlots = MAX_INVENTORY_SLOTS;//TODO: add check for premium inventory.
	int emptySlot = -1;
	for (int i = 0; i < maxSlots; i++)
	{
		if (m_userData->m_itemArray[INV_1_START + i].itemId == itemId && m_userData->m_itemArray[INV_1_START + i].count + count <= MAX_ITEM_STACK)
			return INV_1_START + i;
		else if (m_userData->m_itemArray[INV_1_START + i].itemId == 0 && emptySlot == -1)
			emptySlot = INV_1_START + i;
	}

	return emptySlot;
}

void CUser::SendSkillBookAdded(_PLAYER_SKILL_BOOK_DATA* pData, uint8 pos)
{
	Packet result(PKT_GAMESERVER_SKILLBOOK_SKILL, uint8(1));//1 for insert
	result << pos << pData->m_skillBookId;
	for (int i = 0; i < SPECIAL_PT_TYPE_END; i++)
		result << pData->m_specialBookStats[i];

	for (int i = 0; i < MAX_SKILLS_IN_BOOK; i++)
	{
		result << pData->m_skillData[i].m_skillId;
		result << pData->m_skillData[i].m_pointsSpent;
	}
	Send(&result);
	//Pretty sure hero always sends this when adding a skill book.
	SendCharacterUpdate();
}

bool CUser::HandleSwitchActiveWeapon(uint8 newActiveSlot)
{
	if (newActiveSlot != 0x03 && newActiveSlot != 0x04)
		return false;
	m_userData->m_activeWeapon = newActiveSlot;
	UpdateItemSlotValues();
	UpdateUserStats();

	SendVisibleItemListToRegion();
	SendUnk75();
	SendRegionCharacterUpdate();
}

int16 CUser::FindItemPos(uint32 itemId, uint16 startPos)
{
	for (int i = startPos; i < IS_END; i++)
		if (m_userData->m_itemArray[i].itemId == itemId)
			return i;
	return -1;
}

uint32 CUser::FindItemTotalCount(uint32 itemId)
{
	uint32 totalCount = 0;
	int16 pos = 0;

	while (true)
	{
		pos = FindItemPos(itemId, pos);
		if (pos == -1)
			break;
		totalCount += m_userData->m_itemArray[pos++].count;
	}

	return totalCount;
}

bool CUser::GiveItem(uint32 itemId, uint16 count /* = 1 */)
{
	bool newItem = true;
	_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(itemId);
	if (pTable == NULL)
		return false;

	//TODO: Implement items auto stacking. Find slot with item, add it, check if it exceeds the max slot stack, add to new, cotninue
	int pos = -1;
	if (pTable->m_stackable)
		pos = FindFreeInventorySlot(itemId, count);
	else
		pos = FindFreeInventorySlot();

	if (pos < 0)
		return false;

	_ITEM_DATA* pItem = &m_userData->m_itemArray[pos];
	if (pItem->count > 0)
	{
		//We have the item already
		if (!pTable->m_stackable || pItem->itemId != itemId)
			return false;

		newItem = false;
	}

	if (newItem)
		pItem->Initialize();
	pItem->itemId = itemId;
	pItem->count += count;
	if (pItem->count > MAX_ITEM_STACK)
		pItem->count = MAX_ITEM_STACK;

	//pItem->expirationTime = pTable->m_duration;
	if (pTable->m_itemRarity == 255)//TODO: Hero seems to be doing this on alot of items!
		pItem->count = pTable->m_duration;

	SendAcquireItem(pItem, pTable, pos, newItem);
	SendUpdateItemSlot(pTable, pItem->count);
	//SendBoughtItem(pTable, pItem->count, 0, pos);
	//g_main->m_dbAgent.SaveUserInventory(m_userData);
	return true;
}

bool CUser::GiveItem(uint32 itemId, int pos, uint16 count /* = 1 */)
{
	bool newItem = true;
	_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(itemId);
	if (pTable == NULL)
		return false;

	_ITEM_DATA* pItem = &m_userData->m_itemArray[pos];
	if (pItem->count > 1)
	{
		//We have the item already
		if (!pTable->m_stackable || pItem->itemId != itemId)
			return false;

		newItem = false;
	}

	if (newItem)
		pItem->Initialize();
	pItem->itemId = itemId;
	pItem->count += count;
	if (pItem->count > MAX_ITEM_STACK)
		pItem->count = MAX_ITEM_STACK;

	//pItem->expirationTime = pTable->m_duration;
	if (pTable->m_itemRarity == 255)//TODO: Hero seems to be doing this on alot of items!
		pItem->count = pTable->m_duration;

	//SendAcquireItem(pItem, pTable, pos, newItem);
	//SendUpdateItemSlot(pTable, pItem->count);
	SendAddItemSilent(pItem, pTable, pos);
	//g_main->m_dbAgent.SaveUserInventory(m_userData);
	return true;
}

void CUser::GiveItem(_ITEM_DATA* pItem, int pos /*= -1*/)
{
	if (pItem == NULL)
		return;

	if (pos == -1)
	{
		pos = FindFreeInventorySlot();
		if (pos <= -1)//TODO: If we get this far, send by pigeon?
			return;
	}

	_ITEM_DATA* pNewItemPos = &m_userData->m_itemArray[pos];
	if (pNewItemPos->itemId == 0)
	{
		AddItemToSlot(pItem, pos);
	}
	else
	{
		pNewItemPos->count += pItem->count;
		pItem->Initialize();
	}

	_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(pNewItemPos->itemId);
	SendAcquireItem(pNewItemPos, pTable, pos, false);
	SendUpdateItemSlot(pTable, pNewItemPos->count);
}

bool CUser::RemoveItem(int16 slot, uint16 count)
{
	uint32 itemId = m_userData->m_itemArray[slot].itemId;
	if (itemId == 0 || count > m_userData->m_itemArray[slot].count)//TODO: Add check for unremovable items.
		return false;

	if (m_userData->m_itemArray[slot].count > count)
	{
		m_userData->m_itemArray[slot].count -= count;
		SendRemoveItemCount(&m_userData->m_itemArray[slot], slot);
	}
	else
	{
		m_userData->m_itemArray[slot].count = 0;
		//Hero still wants this packet if it's a stackable item.
		if (sObjMgr->GetItemTemplate(itemId)->m_stackable)
			SendRemoveItemCount(&m_userData->m_itemArray[slot], slot);
		else
			SendRemoveItem(itemId, slot);
		m_userData->m_itemArray[slot].Initialize();
	}
	return true;
}

int16 CUser::RemoveItem(uint32 itemId, uint16 count)
{
	int16 removedFromSlot = -1;
	uint32 totalCount = FindItemTotalCount(itemId);

	if (count > totalCount)
		return removedFromSlot;

	int16 pos = 0;
	do
	{
		pos = FindItemPos(itemId, pos);
		if (m_userData->m_itemArray[pos].count >= count)
		{
			RemoveItem(pos, count);
			count = 0;
		}
		else
		{
			uint16 tempCount = m_userData->m_itemArray[pos].count;
			RemoveItem(pos, count);
			if (tempCount > count)
				count = 0;
			else
				count -= tempCount;
		}
	} while (count > 0);

	removedFromSlot = pos;
	
	return removedFromSlot;
}

void CUser::SendMoveItem(uint32 itemId, uint16 oldPos, uint16 newPos)
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_EMPTY));
	result << uint8(0x0A) << uint8(0x00);
	result << itemId;
	result << oldPos;
	result << newPos;

	Send(&result);
}

void CUser::SendMoveItemItem(uint32 oldItemId, uint32 newItemId, uint16 oldPos, uint16 newPos)
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_ITEM));
	result << uint8(0x0A) << uint8(0x00) << uint8(0x01);
	result << oldItemId << oldPos << newPos;
	result << newItemId << newPos << oldPos;

	Send(&result);
}
void CUser::SendMoveItemStack(uint32 itemId, uint16 oldCount, uint16 newCount, uint16 oldPos, uint16 newPos)
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_ITEM_STACK)); //TODO: This one has a subopcode i think 1 = pickup new, 6 = move
	result << uint8(0x0A) << uint8(0x00); //Success.
	result << itemId;
	result << oldPos;
	result << oldCount;
	result << newPos;
	result << newCount;

	Send(&result);
}


void CUser::SendMoveItemSplitStack(_ITEM_TABLE* pItem, uint16 oldCount, uint16 newCount, uint16 oldPos, uint16 newPos)
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_SPLIT));
	result << uint8(0x0A) << uint8(0x00);
	result << pItem->m_itemId;
	result << uint8(0x00) << uint8(pItem->m_itemRarity);
	result << oldCount << oldPos;
	result << uint32(0x00000000);
	result << pItem->m_itemId;
	result << uint8(0x00) << uint8(pItem->m_itemRarity);
	result << newCount << newPos;
	result << uint32(0x00000000);

	Send(&result);
}

void CUser::SendAcquireItem(_ITEM_DATA* pItem, _ITEM_TABLE* pTable, uint16 pos, bool newItem /* = false */)
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_PICKUP));
	result << uint8(0x0A) << uint8(0x00) << uint8(0x01);
	result << pTable->m_itemId << uint8(0x00);
	result << uint8(pTable->m_itemRarity);
	result << pItem->count;
	result << pos;
	if (pTable->m_itemRarity > 0xA1)
	{
		for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT; i++)
			result << pItem->upgrades[i];
		result << pItem->holeCount;
		for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT - 1; i++)
			result << pItem->holes[i];
	}
	result << uint32(0x00000000) << uint16(0x3b48);//Unk

	Send(&result);
}

void CUser::SendBoughtItem(_ITEM_TABLE* pTable, uint16 count, uint32 duration, uint16 pos)
{
	Packet result(PKT_GAMESERVER_ITEM_STACK_CHANGE, uint8(1));//1 buy 2 sell
	result << uint8(0x0A) << uint8(0x00); //Success.
	result << pTable->m_itemId << uint8(0x00);
	result << uint8(pTable->m_itemRarity);
	result << count;
	result << pos;
	result << uint32(0x00000000); //Unk
	result << m_userData->m_gold;
	result << uint16(0x1c20) << uint16(0x0000);//Unk

	Send(&result);
}

void CUser::SendSoldItem(_ITEM_TABLE* pTable, uint16 pos)
{
	Packet result(PKT_GAMESERVER_ITEM_STACK_CHANGE, uint8(2));//1 buy 2 sell
	result << uint8(0x0A) << uint8(0x00); //Success.
	result << pTable->m_itemId;
	result << pos;
	result << m_userData->m_gold;
	result << uint16(0x1c20) << uint16(0x0000);//Unk

	Send(&result);
}

void CUser::SendUpdateGoldSilent()
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(0x0B));
	result << m_userData->m_gold;

	Send(&result);
}

void CUser::HandleUseItem(_ITEM_DATA* pItem)
{
	_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(pItem->itemId);

	if (pTable == NULL)
		return;

	//TODO: Develop the recovery item handling more. No animation yet. Also check why the item isn't removed. Also check if the player has max HP and or Chi, don't use it then.
	if (pTable->m_itemType == 151 && CanEquipItem(pTable))
	{
		if (!HasItem(pTable->m_itemId))
			return;

		SendUsePotion();

		if (!RemoveItem(pItem->itemId, 1))
			return;

		HpChange(pTable->m_hpRecovery);
		ChiChange(pTable->m_chiRecovery);
		return;
	}

	if ((pTable->m_itemType == 161 || pTable->m_itemType == 162) && CanEquipItem(pTable))
	{
		_SKILL_BOOK_DATA* pSTable = sObjMgr->GetSkillBookInfo(pItem->itemId);
		if (pSTable == NULL)
			return;

		int8 bookAdded = -1;
		for (int i = 0; i < MAX_EXPANDABLE_SKILLBOOKS; i++)
		{
			//TODO: Only 5 slots without keys? Not added yet so not sure how to get more slots.
			if (!m_userData->m_skillBookArray[i].HasBook())
			{
				m_userData->m_skillBookArray[i].Initialize(pSTable);
				bookAdded = i;
				break;
			}
		}

		if (bookAdded == -1)
		{
			goto return_fail;
		}

		RemoveItem(pItem->itemId);
		SendSkillBookAdded(&m_userData->m_skillBookArray[bookAdded], bookAdded);
	}

	return;

return_fail:
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_USE_ITEM));
	result << uint8(0);
	Send(&result);
}

void CUser::HandleGambleItem(uint32 itemId)
{
	_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(itemId);
	if (pTable == NULL)
		goto return_fail;

	_GAMBLING_ITEM_TABLE* pGamble = sObjMgr->GetGamblingItemTable(pTable->m_itemId);
	if (pGamble == NULL)
		goto return_fail;

	if (pGamble->m_dropTable == NULL)
		goto return_fail;

	if (pGamble->m_openingCost > m_userData->m_gold)
		goto return_fail;

	{
		GoldChange(-pGamble->m_openingCost, false);

		int16 slot = RemoveItem(pTable->m_itemId);

		if (slot < 0)
			goto return_fail;

		std::list<_ITEM_DATA*> dropList = g_main->GenerateItemDropList(pGamble->m_dropTable->m_dropId, 1);

		foreach(itr, dropList)
		{
			GiveGambledItem(*itr, sObjMgr->GetItemTemplate((*itr)->itemId), pGamble, slot);
		}

		if (dropList.empty())
			GiveGambledItem(NULL, NULL, pGamble, slot);
	}
	return;

return_fail:
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_OPEN_GAMBLECHEST));
	result << uint8(0xF9) << uint8(0x03);
	Send(&result);
}

//Interesting notes, this packet can change both fame and karma, idk even know what karma is.
void CUser::GiveGambledItem(_ITEM_DATA* pItem, _ITEM_TABLE* pTable, _GAMBLING_ITEM_TABLE* pGamble, int16 slot)
{
	///
	///
	//KEEP TESTING THIS, SOMETIMES INVENTORY STILL FREEZES! IDK WHY :( ALSO FUNCTION ABOVE OFC ))
	///
	///

	if (pItem != NULL)
		pItem = AddItemToSlot(pItem, slot);
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_OPEN_GAMBLECHEST));
	result << uint8(0x0A) << uint8(0x00);//Success.

	if (pItem == NULL)
		result << uint32(0);
	else
		result << pItem->itemId;
	result << uint8(0);

	if (pTable == NULL)
		result << uint8(0);
	else
		result << pTable->m_itemRarity;

	if (pItem == NULL)
		result << uint16(0);
	else
		result << pItem->count;//Count
	result << slot;
	
	if (pItem == NULL)
	{
		for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT; i++)
			result << uint8(0);
		for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT; i++)
			result << uint8(0);
	}
	else if (pItem->upgradeCount > 0)
	{
		for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT; i++)
			result << pItem->upgrades[i];
		result << pItem->holeCount;
		for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT - 1; i++)
			result << pItem->holes[i];
	}

	result << uint32(0)//Gold gain?
		<< uint64(pGamble->m_openingCost)
		<< uint32(0)//Delimeter?
		<< uint64(m_userData->m_gold)//New gold
		<< uint64(0);//Unk, probably double delimeter

	Send(&result);
	//Idk why they send this, only if it didn't create an item tho.
	if (pItem == NULL)
	{
		SendUpdateHpAndChi(NULL);
		//Packet 01 aswell
	}
}


void CUser::SendUseItem(uint32 itemId)
{

}

void CUser::SendRemoveItem(uint32 itemId, uint16 pos)
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_REMOVE));
	result << uint8(0x0A) << uint8(0x00) << uint8(0x01);
	result << itemId << pos;

	Send(&result);
}

void CUser::SendRemoveItemCount(_ITEM_DATA* pItem, uint16 slot)
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_USE_ITEM));
	result << uint8(0x0A) << uint8(0x00);
	result << pItem->itemId;
	result << slot;
	result << pItem->count;

	Send(&result);
}

//TODO: This is realy wrong. It might not even update the slot. It was sent when throwing items out.(removing a stack)
void CUser::SendUpdateItemSlot(_ITEM_TABLE* pTable, uint16 count)
{
	Packet result(PKT_GAMESERVER_UPDATE_ITEMSLOT, uint8(0x04));
	result << uint8(0x01) << pTable->m_itemId << uint8(0x00);
	result << uint8(pTable->m_itemRarity) << count << count - 1;// << uint16(0x0000);//Unks
	result << uint32(0x00000000); //Delimeter per usual?

	Send(&result);
}

void CUser::SendUsePotion()
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_USE_POTION));
	result << uint8(0x0A) << uint8(0)
		<< uint16(0x00A7) << uint16(0x0386)
		<< uint16(0);

	Send(&result);
}

void CUser::SendGoldUpdate()
{
	Packet result(PKT_GAMESERVER_MOVE_ADD_ITEM, uint8(MOVE_ITEM_PICKUP));
	result << uint8(0x0A)
		<< uint8(0)
		//TODO: Implement remove and add gold, shows correct text.
		<< uint8(2)//Loose gain? 2 get 1 loose? Might just be a relic of the past.
		<< m_userData->m_gold;

	Send(&result);
}

///TODO: Make this function delete the old item(in the item array) and just point it to the new instead! Check functions that use this so we don't double delete
/*PRIVATE*/ _ITEM_DATA* CUser::AddItemToSlot(_ITEM_DATA* pNewItem, int16 slot)
{
	_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(pNewItem->itemId);
	if (pTable != NULL && pTable->m_stackable)
	{
		//Allways set item id, it's the same item id anyways. BUt i can be 0 so we have to set it.
		m_userData->m_itemArray[slot].itemId = pNewItem->itemId;
		m_userData->m_itemArray[slot].count += pNewItem->count;
	}
	else
	{
		m_userData->m_itemArray[slot].Initialize();
		memcpy(&m_userData->m_itemArray[slot], pNewItem, sizeof(_ITEM_DATA));
	}
	
	delete pNewItem; pNewItem = NULL;
	return &m_userData->m_itemArray[slot];
}

//uint16 CUser::GetValidInventoryPos(uint16 pos)
//{
//	if (pos >= 307)
//		pos -= 240;
//	else if (pos >= 317)
//		pos -= 242;
//	if (pos >= 320)
//		return uint16(-1);
//
//	if (pos < 0 || pos > MAX_EQUIP_SLOTS + MAX_INVENTORY_SLOTS)
//		return uint16(-1);
//
//	return pos;
//}

void CUser::SendOpenBank()
{
	uint8 itemCount = 0;
	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(5));
	result << uint8(1);
	uint8 cPos = result.wpos();
	result << uint8(0); // Reserve pos for itemCount

	for (int i = BANK_1_START; i < BANK_2_START + 60; i++)
	{
		_ITEM_DATA* pItem = &m_userData->m_itemArray[i];
		if (pItem == NULL || pItem->itemId == 0)
			continue;
		result << pItem->itemId
			<< uint8(0)
			<< sObjMgr->GetItemTemplate(pItem->itemId)->m_itemRarity
			<< pItem->count
			<< i
			<< uint32(0);
	}

	Send(&result);
}

void CUser::HandleBankGold(Packet& pkt)
{
	uint8 subOpcode;
	uint64 amount;
	pkt >> subOpcode >> amount;

	//Deposit gold to bank
	if (subOpcode == 1)
	{
		if (amount > m_userData->m_gold)
			return;

		m_userData->m_gold -= amount;
		m_userData->m_warehouseGold += amount;
	}
	//Withdraw gold from bank
	else if (subOpcode == 2)
	{
		if (amount > m_userData->m_warehouseGold)
			return;

		m_userData->m_warehouseGold -= amount;
		m_userData->m_gold += amount;
	}

	Packet result(PKT_GAMESERVER_BANK_MOVE_GOLD, subOpcode);
	result << m_userData->m_gold
		<< m_userData->m_warehouseGold;

	Send(&result);
}

void CUser::OpenStrengtheningWindow()
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(8));
	result << uint8(1);

	Send(&result);
}

void CUser::OpenCompositionWindow()
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(0x0F));
	result << uint8(1);

	Send(&result);
}

void CUser::OpenAdvancedFusionWindow()
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(0x32));
	result << uint8(1);

	Send(&result);
}

void CUser::OpenExtractionWindow()
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(0x17));
	result << uint8(1);

	Send(&result);
}

void CUser::OpenDismantleWindow() 
{
	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(0x16));
	result << uint8(1);

	Send(&result);
}

void CUser::HandleNpcExchange(Packet& pkt)
{
	uint8 subOpcode;
	uint32 itemId, shopTableId, npcId;
	uint16 slot, count;
	pkt >> subOpcode;

	switch (subOpcode)
	{
	case 1://buy
	{
		pkt >> itemId >> count >> shopTableId >> slot >> npcId;

		//Idk, if they try something funky i guess.
		if (sObjMgr->GetNpcInfo(npcId)->IsMonster())
			return;

		_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(itemId);
		if (pTable == NULL)
			return;

		_SHOP_TABLE_ITEM* pShop = sObjMgr->GetShopTableItemInfo(shopTableId);
		if (pShop == NULL)
			return;

		//TODO: Make the items stack after being added, hero doesn't allow that when buying items tho.(Client shows incorrect inv data). Just move it after.
		int pos = FindFreeInventorySlot();

		//No valid slot found
		if (pos == -1)
			return;

		//Item costs only gold
		if (pTable->m_extendedPrice == 0)
		{
			if (m_userData->m_gold < pTable->m_buyPrice * count)
				return;

			if (!GiveItem(itemId, pos, count))
				return;

			GoldChange(-(pTable->m_buyPrice * count), false);
		}
		else//Purchase with item
		{
			_ITEM_TABLE* pExtended = sObjMgr->GetItemTemplate(pTable->m_extendedPrice);
			if (pExtended == NULL)
				return;

			if (!HasItemCount(pExtended->m_itemId, pTable->m_buyPrice))
				return;

			if (!GiveItem(itemId, pos, count))
				return;

			RemoveItem(pExtended->m_itemId, pTable->m_buyPrice);
		}

		_ITEM_DATA* pItem = &m_userData->m_itemArray[pos];

		SendBoughtItem(pTable, pItem->count, 0, pos);
		break;
	}
	case 2://sell
	{
		pkt >> itemId >> count >> slot;
		_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(itemId);
		if (pTable == NULL || pTable->m_extendedPrice != 0)
			return;

		_ITEM_DATA* pItem = &m_userData->m_itemArray[slot];
		if (pItem == NULL || pItem->itemId != pTable->m_itemId || pItem->count < count)
			return;

		pItem->Initialize();
		GoldChange(pTable->m_sellPrice * count, false);

		SendSoldItem(pTable, slot);
		break;
	}
	default:
		break;
	}
	int i = 0;
}

void CUser::HandleItemModification(Packet& pkt)
{
	uint8 subOpcode;
	uint8 stoneAmount;
	uint16 slot, stoneSlot[3], charm, scale;
	pkt >> subOpcode;

	switch (subOpcode)
	{
	case 2://Upg item
	{
		//TODO: Implement HT upgrading, right now it won't work like it should. Should add + depending on + on pendent, also need to check all have same +.
		 pkt >> slot >> stoneAmount;

		if (stoneAmount == 0 || stoneAmount > 3)
			return;

		if (slot >= IS_END)
			return;

		_ITEM_DATA* pItem = &m_userData->m_itemArray[slot];

		if (pItem == NULL || pItem->itemId == 0)//TODO: Check for non upgradeable items.
			return;

		for (int i = 0; i < stoneAmount; i++)
			pkt >> stoneSlot[i];

		_ITEM_DATA* pStone = &m_userData->m_itemArray[stoneSlot[0]];

		if (pStone == NULL || pStone->itemId == 0 || !HasItemCount(pStone->itemId, stoneAmount))
			return;

		uint32 goldCost = (sObjMgr->GetItemTemplate(pItem->itemId)->m_buyPrice * (pItem->upgradeCount + 1)) * pow(2, stoneAmount - 1);

		if (m_userData->m_gold < goldCost)
			return;//TODO: Send error

		pkt >> charm >> scale;

		_ITEM_TABLE* pCharm = NULL;
		_ITEM_TABLE* pScale = NULL;

		if (charm != 0 && charm < IS_END)
		{
			pCharm = sObjMgr->GetItemTemplate(m_userData->m_itemArray[charm].itemId);
			if (pCharm != NULL && pCharm->m_itemType != 164)
				pCharm = NULL;
		}

		if (scale != 0 && scale < IS_END)
		{
			pScale = sObjMgr->GetItemTemplate(m_userData->m_itemArray[scale].itemId);
			if (pScale != NULL && pScale->m_itemType != 166)
				pScale = NULL;

			if (pScale == NULL)
				return;
		}

		bool success = sItemMgr->UpgradeItem(pItem, stoneAmount, pStone->itemId, pCharm == NULL ? 0 : pCharm->m_sellPrice);

		RemoveItem(uint32(pStone->itemId), stoneAmount);
		GoldChange(-goldCost, false);

		if (success)
		{
			SendUpgradeItemResult(1, slot, sObjMgr->GetItemTemplate(pItem->itemId));
			SendMyInfo();//TODO: Replace with slot/item update.
		}
		else if (pScale == NULL)
		{
			SendUpgradeItemResult(0, slot, sObjMgr->GetItemTemplate(pItem->itemId));
			RemoveItem(pItem->itemId);
		}
		else
		{
			pItem->RemoveUpgrades(uint8(pScale->m_sellPrice));
			SendUpgradeItemResult(2, slot, sObjMgr->GetItemTemplate(pItem->itemId));
		}

		if (pCharm != NULL)
			RemoveBySlot(charm);
		if (pScale != NULL)
			RemoveBySlot(scale);

		if (IsEquipSlot(slot))
		{
			UpdateItemSlotValues();
			SendCharacterUpdate();
			SendVisibleItemListToRegion();
		}
		break;
	}
	case 4://Composition
	{
		uint8 numItems, slot2Count, slot3Count;
		uint16 slot1, slot2, slot3, slot4, resultSlot;

		pkt >> numItems >> slot1 >> slot2 >> slot2Count >> slot3 >> slot3Count >> slot4 >> resultSlot;

		_ITEM_DATA* pBook = &m_userData->m_itemArray[slot1];

		if (pBook == NULL || pBook->itemId == 0)
			return;

		_MAKE_ITEM_TABLE* pMake = sObjMgr->GetMakeItemTable(pBook->itemId);
		if (pMake == NULL)
			return;

		_ITEM_DATA* pReqItem1 = &m_userData->m_itemArray[slot2];
		_ITEM_DATA* pReqItem2 = &m_userData->m_itemArray[slot3];
		_ITEM_DATA* pReqItem3 = NULL;
		if (pReqItem1 == NULL || pReqItem2 == NULL)
			return;

		if (!HasItemCount(pReqItem1->itemId, pMake->reqItemCount1) || !HasItemCount(pReqItem2->itemId, pMake->reqItemCount2))
			return;

		_ITEM_TABLE* pHammer = NULL;

		if (pMake->reqItemId3 != 0 && !HasItemCount(pMake->reqItemId3, 1))
			return;
		else if(pMake->reqItemId3 != 0)
		{
			pReqItem3 = &m_userData->m_itemArray[slot4];
			if (pReqItem3 == NULL)
				return;
		}

		if (pReqItem3 == NULL && slot4 != 0)
		{
			pHammer = sObjMgr->GetItemTemplate(m_userData->m_itemArray[slot4].itemId);
			if (pHammer != NULL && pHammer->m_itemType != 167)
				pHammer == NULL;
		}

		if (m_userData->m_gold < pMake->reqGold)
			return;

		int slot = FindFreeInventorySlot();
		if (slot == -1)
			return;

		_ITEM_TABLE* pResItem = sObjMgr->GetItemTemplate(pMake->resultItem);
		if (pResItem == NULL)
			return;

		GoldChange(-pMake->reqGold, false);

		SendUpdateGoldSilent();

		bool success = sItemMgr->ComposeItem(pMake, pHammer == NULL ? 0 : pHammer->m_sellPrice);

		Packet result;

		if (success)
		{
			//TODO: Check for adding failure w/e
			GiveItem(pMake->resultItem, slot, 1);
			result.Initialize(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(0x0A));
			result << pMake->resultItem
				<< uint8(0)
				<< pResItem->m_itemRarity
				<< uint16(1)
				<< uint16(slot);
			//Hero just wants this if the rarity is that high.
			if (pResItem->m_itemRarity > 161 || pResItem->m_itemRarity == 0)
			{
				foreach_array_n(j, pItem->upgrades, MAX_ITEM_UPGRADE_AMOUNT)
					result << uint8(0);
				result << uint8(0);
				foreach_array_n(j, pItem->holes, MAX_ITEM_UPGRADE_AMOUNT - 1)
					result << uint8(0);
			}
			result << uint32(0);

			Send(&result);

			SendItemModificationStatusMessage(subOpcode, 4104);
		}
		else
			SendItemModificationStatusMessage(subOpcode, 4104, 0);

		
		RemoveBySlot(slot1);
		RemoveBySlot(slot2);
		RemoveBySlot(slot3);

		if (pReqItem3 != NULL)
			RemoveBySlot(slot4);

		break;
	}
	case 5://Dismantle
	{
		uint16 slot1, slot2;
		pkt >> slot1 >> slot2;//No clue what slot 2 is even used for.

		uint16 numSlotsFree = CountFreeInventorySlots();
		if (numSlotsFree < 4)
			return;

		if (slot1 == 0 || slot1 >= IS_END)
			return;

		_ITEM_DATA* pDismantleItem = &m_userData->m_itemArray[slot1];
		if (pDismantleItem->itemId == 0 || pDismantleItem->count == 0)
			return;

		_DISMANTLE_ITEM* pDismantle = sObjMgr->GetDismantleItemTable(pDismantleItem->itemId);
		if (pDismantle == NULL)
			return;

		if (m_userData->m_gold < pDismantle->goldReq)
			return;

		GoldChange(-pDismantle->goldReq, false);
		SendUpdateGoldSilent();

		//TODO: Check for 2nd job promotion.
		uint32* resultingItems = new uint32[MAX_DISMANTLE_RESULTS];
		bool success = sItemMgr->DismantleItem(pDismantle, resultingItems);

		//I'm aware that this right here, is messy as fuck.
		if (success)
		{
			Packet result(PKT_GAMESERVER_MODIFY_ITEM, uint8(subOpcode));
			result << 4200;
			int startSlot = -1;
			uint16 startSlotPos = result.wpos();
			result << uint8(1)
				<< uint64(321);//Gold reward.
			uint8 counter = 0;
			uint16 countPos = result.wpos();
			result << uint8(0);

			for (int i = 0; i < MAX_DISMANTLE_RESULTS - 1; i++)
			{
				if (resultingItems[i] == 0)
					continue;

				int pos = FindFreeInventorySlot();
				if (pos == -1)
					continue;

				if (startSlot == -1)
				{
					startSlot = i + 1;
					result.put(startSlotPos, startSlot);
				}

				counter++;

				uint16 count = sItemMgr->RollRange(1, pDismantle->rewardMaxCount[i]);
				
				result << resultingItems[i] << uint8(0) << uint8(161)
					<< count << pos << uint32(0);

				GiveItem(resultingItems[i], pos, count);
			}
			result.put(countPos, counter);

			if (resultingItems[MAX_DISMANTLE_RESULTS - 1] != 0)
			{
				int pos = FindFreeInventorySlot();
				if (pos != -1)
				{
					result << resultingItems[MAX_DISMANTLE_RESULTS - 1] << uint8(0) << uint8(161)
						<< 1 << pos << uint32(0);

					GiveItem(resultingItems[MAX_DISMANTLE_RESULTS - 1], pos, 1);
				}
			}

			SendItemModificationStatusMessage(subOpcode, 4200, 0);

			Send(&result);

			RemoveBySlot(slot1);
		}
		else
		{
			RemoveBySlot(slot1);
			SendItemModificationStatusMessage(subOpcode, 4203, 0);
		}
		break;
	}
	case 6://Extraction
	{
		uint16 slot1, slot2;
		pkt >> slot1 >> slot2;//No clue what slot 2 is even used for.

		SendItemModificationStatusMessage(subOpcode, 4303, 0);

		break;
	}
	case 9://Advanced fusion
	{
		uint8 numItems;
		uint16 slot1, slot2, slot3, hammer, unk;
		pkt >> numItems >> slot1 >> slot2 >> slot3 >> hammer >> unk;

		_ITEM_DATA* pReqItem1 = &m_userData->m_itemArray[slot1];
		_ITEM_DATA* pReqItem2 = &m_userData->m_itemArray[slot2];
		_ITEM_DATA* pReqItem3 = &m_userData->m_itemArray[slot3];

		if (pReqItem1 == NULL || pReqItem2 == NULL || pReqItem3 == NULL)
			return;

		if (pReqItem1->itemId != pReqItem2->itemId || pReqItem1->itemId != pReqItem3->itemId)
			return;

		_MAKE_ITEM_FUSION_TABLE* pMakeFusion = sObjMgr->GetMakeItemFusionTable(pReqItem1->itemId);
		if (pMakeFusion == NULL)
			return;

		_ITEM_TABLE* pHammer = NULL;
		if (hammer != 0 && hammer < IS_END)
		{
			pHammer = sObjMgr->GetItemTemplate(m_userData->m_itemArray[hammer].itemId);
			//85 = items that increase fusing success rates.
			if (pHammer != NULL && pHammer->m_itemType != 85)
				pHammer = NULL;
		}
		
		int pos = FindFreeInventorySlot();
		if (pos == -1)
			return;

		GoldChange(-pMakeFusion->reqGold, false);
		SendUpdateGoldSilent();

		bool success = sItemMgr->FuseItem(pMakeFusion, pHammer == NULL ? 0 : pHammer->m_sellPrice);

		Packet result;
		if (success)
		{
			//Gives resulting item.
			GiveItem(pMakeFusion->resultItem, pos);

			RemoveBySlot(slot1);
			RemoveBySlot(slot2);
			RemoveBySlot(slot3);

			//Success message
			SendItemModificationStatusMessage(subOpcode, 4104);
		}
		else
		{
			//Fail message
			SendItemModificationStatusMessage(subOpcode, 4105, 0);
		}

		if (pHammer != NULL)
			RemoveBySlot(hammer);

		break;
	}
	default:
		break;
	}

}

void CUser::SendItemModificationStatusMessage(uint8 action, uint16 status, uint8 success /*= 1*/)
{
	Packet result(PKT_GAMESERVER_MODIFY_ITEM, uint8(action));
	result << status
		<< success;

	Send(&result);
}

void CUser::SendUpgradeItemResult(uint8 res, uint16 slot, _ITEM_TABLE* pTable)
{
	_ITEM_DATA* pItem = &m_userData->m_itemArray[slot];

	Packet result(PKT_GAMESERVER_MODIFY_ITEM, uint8(2));

	switch (res)
	{
	case 0://fail
		result << uint8(0xA2) << uint8(0x0F) << uint8(0);
		result << pTable->m_itemRarity << slot << pItem->itemId;
		break;
	case 1://Success
		result << uint8(0xA1) << uint8(0x0F) << uint8(0x01);
		result << pItem->itemId << uint8(0) << pTable->m_itemRarity << pItem->count << slot;

		if (pItem->upgradeCount > 0 || pItem->holeCount > 0 || pTable->m_itemRarity > 161)
		{
			for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT; i++)
				result << pItem->upgrades[i];

			result << pItem->holeCount;
			for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT - 1; i++)
				result << pItem->holes[i];
		}
		else
			result << uint32(0);
		break;
	case 2://Protected by scale
		result << uint8(0xA1) << uint8(0x0F) << uint8(0x01);
		result << pItem->itemId << uint8(0) << pTable->m_itemRarity << pItem->count << slot;
		if (pItem->upgradeCount > 0 || pItem->holeCount > 0 || pTable->m_itemRarity > 161)
		{
			for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT; i++)
				result << pItem->upgrades[i];

			result << pItem->holeCount;
			for (int i = 0; i < MAX_ITEM_UPGRADE_AMOUNT - 1; i++)
				result << pItem->holes[i];
		}
		else
			result << uint32(0);
		break;
	default:
		break;
	}

	Send(&result);
}

void CUser::RemoveBySlot(int slot) 
{
	m_userData->m_itemArray[slot].Initialize();

	Packet result(PKT_GAMESERVER_ADD_ITEM_CONTINUOS, uint8(0x0A));
	result << uint32(0)
		<< uint32(0)
		<< uint16(slot)
		<< uint16(0)
		<< uint32(0)
		<< uint32(0)
		<< uint32(0)
		<< uint32(0)
		<< uint32(0)
		<< uint32(0)
		<< uint32(0)
		<< uint32(0);

	Send(&result);
}