#ifndef ITEMMGR_H
#define ITEMMGR_H

//TODO: This class is supposed to handle item generation/modifying. All the code that's doing that currently is to be moved in here.

class ItemMgr
{
public:
	static ItemMgr* instance();

	std::list<_ITEM_DATA*> GenerateItemDropList(_NPC_DATA* pNpc);
	bool UpgradeItem(_ITEM_DATA* pItem, uint8 numStones, uint32 stoneType, uint16 bonusRate);
	bool ComposeItem(_MAKE_ITEM_TABLE* pMake, uint16 bonusRate);
	bool FuseItem(_MAKE_ITEM_FUSION_TABLE* pMakeFusion, uint16 bonusRate);
	bool DismantleItem(_DISMANTLE_ITEM* pDismantle, uint32* resultingItems);

	uint32 RollRange(int32 min, int32 max) 
	{ 
		std::uniform_int_distribution<> gen(min, max);
		return gen(rng);
	}

private:
	ItemMgr();
	~ItemMgr();

	std::map<uint8, uint16> m_upgradeRates;
	_ITEM_DATA* GenerateItem(_ITEM_DROP_TABLE* pDropitem);
	std::random_device r;
	std::mt19937 rng;
};

#define sItemMgr ItemMgr::instance()

#endif