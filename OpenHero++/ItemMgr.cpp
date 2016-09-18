#include "stdafx.h"
#include "ItemMgr.h"

ItemMgr* ItemMgr::instance()
{
	static ItemMgr instance;
	return &instance;
}

std::list<_ITEM_DATA*> ItemMgr::GenerateItemDropList(_NPC_DATA* pNpc)
{
	_ITEM_DROP_TABLE* pDropTable = sObjMgr->GetItemDropTable(pNpc->m_dropId);
	std::list<_ITEM_DATA*> dropList;
	if (pDropTable == NULL)
		return dropList;

	//TODO: Randomize 
	for (int i = 0; i < pNpc->m_lootRolls; i++)
	{
		_ITEM_DATA* pItem = GenerateItem(pDropTable);
		if (pItem != NULL)
			dropList.push_back(pItem);
	}
	return dropList;
}

_ITEM_DATA* ItemMgr::GenerateItem(_ITEM_DROP_TABLE* pDropitem)
{
	std::uniform_int_distribution<> gen(0, 1000);
	uint16 roll = gen(rng);

	if (roll > pDropitem->m_maxRoll)
		return NULL;

	for (int i = 0; i < NUM_DROPS_IN_DROP_TABLE; i++)
	{
		if (roll > pDropitem->m_dropableChance[i])
			continue;

		_ITEM_DROP_TABLE* pDrop = sObjMgr->GetItemDropTable(pDropitem->m_dropableId[i]);
		//If it doesn't point to another drop table, generate the item.
		if (pDrop == NULL)
		{
			_ITEM_TABLE* pTable = sObjMgr->GetItemTemplate(pDropitem->m_dropableId[i]);
			if (pTable == NULL)
			{
				printf("Tried to generate item drop, but the item table was null.");
				return NULL;
			}

			//TODO: Check if the player has active quest, set a flag to add instantly or just add it here and move on.
			if (pTable->m_itemType == 202)//Quest item
				return NULL;

			//TODO: Randomize if it should be upgraded or not, also stats.
			_ITEM_DATA* pItem = new _ITEM_DATA();
			pItem->itemId = pTable->m_itemId;
			pItem->count = 1;

			return pItem;
		}
		else
		{
			return GenerateItem(pDrop);
		}
	}
}

bool ItemMgr::UpgradeItem(_ITEM_DATA* pItem, uint8 numStones, uint32 stoneType, uint16 bonusRate)
{
	bonusRate = bonusRate + 100;
	std::uniform_int_distribution<> gen(0, 10000);

	uint16 roll = gen(rng);

	//So the question is how much each stone impacts upgrading, does it just give one more attempt for each stone? Does it add a % increase ?
	//Upgrade success!
	if (roll < m_upgradeRates.at(pItem->upgradeCount + 1) * (bonusRate / 100) + (1.75 * (numStones - 1) * 100))
	{
		pItem->UpgradeItem(stoneType);
		return true;
	}
	else
		return false;
}

bool ItemMgr::ComposeItem(_MAKE_ITEM_TABLE* pMake, uint16 bonusRate)
{
	bonusRate = bonusRate + 100;
	std::uniform_int_distribution<> gen(0, 1000);

	uint16 roll = gen(rng);

	if (roll < pMake->successRate * (bonusRate / 100))
		return true;
	else
		return false;
}

bool ItemMgr::FuseItem(_MAKE_ITEM_FUSION_TABLE* pMakeFusion, uint16 bonusRate)
{
	bonusRate = bonusRate + 100;
	std::uniform_int_distribution<> gen(0, 1000);

	uint32 roll = gen(rng);

	if (roll < pMakeFusion->successRate * (bonusRate / 100))
		return true;
	return false;
}

bool ItemMgr::DismantleItem(_DISMANTLE_ITEM* pDismantle, uint32* resultingItems)
{
	std::uniform_int_distribution<> gen(0, 1000);
	uint8 numResItems = 0;

	//MAX_DISMANTLE_RESULTS -1 saving 1 slot for special item.
	for (int i = 0; i < MAX_DISMANTLE_RESULTS - 1; i++)
	{
		uint32 roll = gen(rng);

		if (roll < pDismantle->chanceForReward)
		{
			numResItems++;
			resultingItems[i] = pDismantle->rewardItem[i];
		}
		else
			resultingItems[i] = 0;
	}

	uint32 roll = gen(rng);
	
	if (roll < pDismantle->specialRewardChance)
	{
		resultingItems[MAX_DISMANTLE_RESULTS - 1] = pDismantle->specialReward;
		numResItems++;
	}

	return numResItems != 0;
}

ItemMgr::ItemMgr()
{
	std::seed_seq seed{ r(), r(), r(), r(), r(), r(), r(), r() };
	rng = std::mt19937(seed);

	//For now, static upgrade rates in here
	m_upgradeRates.insert(make_pair(1, 850));
	m_upgradeRates.insert(make_pair(2, 750));
	m_upgradeRates.insert(make_pair(3, 650));
	m_upgradeRates.insert(make_pair(4, 550));
	m_upgradeRates.insert(make_pair(5, 450));
	m_upgradeRates.insert(make_pair(6, 200));
	m_upgradeRates.insert(make_pair(7, 80));
	m_upgradeRates.insert(make_pair(8, 120));
	m_upgradeRates.insert(make_pair(9, 100));
	m_upgradeRates.insert(make_pair(10, 90));
	m_upgradeRates.insert(make_pair(11, 80));
	m_upgradeRates.insert(make_pair(12, 70));
	m_upgradeRates.insert(make_pair(13, 50));
	m_upgradeRates.insert(make_pair(14, 40));
	m_upgradeRates.insert(make_pair(15, 30));
}

ItemMgr::~ItemMgr()
{

}