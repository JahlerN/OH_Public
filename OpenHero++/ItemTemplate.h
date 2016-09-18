#ifndef ITEMTEMPLATE_H
#define ITEMTEMPLATE_H
#include "..\SharedFiles\globals.h"

enum ItemType
{
	IT_SPEAR = 1,
	IT_ROD = 2,
	IT_SWORD = 3,
	IT_BLADE = 4,
	IT_GAUNTLET = 5,
	IT_AXE = 6,
	IT_BOW = 7,
	IT_THROWING = 8,
	IT_DUALSWORD = 9,
	IT_DUALROD = 10,
	IT_PENTACHORD = 11,
	IT_CLUB = 12,
	IT_STAFF = 13,
	IT_END
};

#define MAX_SET_PARTS 9
#define MAX_PARTIAL_SET_BONUS 7
#define MAX_EXTRA_PARTIAL_SET_BONUS 5
#define SET_BONUS_ITEM_REQ 2
#define SET_BONUS_EXTRA_ITEM_REQ 4
struct _SET_BONUS
{
	uint32 m_setId;
	uint8 m_itemCount;
	uint32 m_setPart[MAX_SET_PARTS];
	uint16 m_bonus[MAX_PARTIAL_SET_BONUS];
	uint16 m_bonusExtra[MAX_EXTRA_PARTIAL_SET_BONUS];

	//TODO: Does extra mean that you have all items, or that you have 4? idk
	__forceinline bool MeetsBonusRequirement(uint8 num) { return num >= SET_BONUS_ITEM_REQ; }
	__forceinline bool MeetsExtraBonusRequirement(uint8 num) { return num >= m_itemCount; }

	__forceinline uint32 GetSetPart(uint8 num)
	{
		if (num > MAX_SET_PARTS)
			return 0;
		else
			return m_setPart[num];
	}

	__forceinline uint16 GetPartialBonus(uint8 num)
	{
		if (num > MAX_PARTIAL_SET_BONUS)
			return 0;
		else
			return m_bonus[num];
	}

	__forceinline uint16 GetExtraPartialBonus(uint8 num)
	{
		if (num > MAX_EXTRA_PARTIAL_SET_BONUS)
			return 0;
		else
			return m_bonusExtra[num];
	}
};

struct _ITEM_TABLE
{
	uint32 m_itemId;
	std::string m_name;
	std::string m_upgradeCode;
	std::string m_upgradeDescription;
	uint32 m_itemRelation;
	uint32 m_nextItemStage;//next stage after cloth improve, for example
	uint8 m_itemType;
	uint8 m_itemRarity;// 161(0xA1) white, not upgraded, higher that that can have upgs
	uint32 m_extendedPrice;
	uint32 m_buyPrice;
	uint32 m_sellPrice;
	uint8 m_equipPos;
	uint8 m_reqClass;//TODO: Make class thing
	uint32 m_reqLevel;
	uint32 m_maxLevel;
	uint16 m_strReq;
	uint16 m_dexReq;
	uint16 m_intReq;
	uint32 m_headDefense;
	uint32 m_bodyDefense;
	uint32 m_footDefense;
	uint16 m_damageMin;
	uint16 m_damageMax;
	double m_delay;
	double m_attackRange;
	double m_attackAoE;
	uint16 m_statBonus[STAT_END];
	/*uint16 m_str;
	uint16 m_dex;
	uint16 m_int;
	uint16 m_wind;
	uint16 m_water;
	uint16 m_fire;*/
	uint8 m_poisonDamage;
	uint8 m_poisonDefense;
	uint8 m_confusionDamage;
	uint8 m_confusionDefense;
	uint8 m_paralysisDamage;
	uint8 m_paralysisDefense;
	uint32 m_maxHp;
	uint8 m_hpRecoveryPercent;
	uint32 m_maxChi;
	uint8 m_chiRecoveryPercent;
	float m_movementSpeed;
	uint32 m_minBonusDamage;
	uint32 m_maxBonusDamage;
	uint16 m_damagePercent;
	uint32 m_minSkillDamageBonus;
	uint32 m_maxSkillDamageBonus;
	uint16 m_skillDamagePercent;
	uint32 m_defense;
	uint16 m_defensePercent;
	uint32 m_artsDefense;
	uint16 m_artsDefensePercent;
	uint16 m_accuracy;
	uint16 m_dodge;
	uint16 m_hpRecovery;
	uint16 m_chiRecovery;
	uint8 m_critChance;
	uint8 m_bonusExp;
	uint16 m_itemDropBonus;
	uint16 m_bonusMineral;
	double m_duration;
	bool m_stackable;
	//effect
	//effect2
	

	_SET_BONUS* m_setBonus;
	//TODO: We need to add base stat stuff, upgrades will be in the Item class.
};

#define NUM_DROPS_IN_DROP_TABLE 20
struct _ITEM_DROP_TABLE
{
	uint32 m_dropId;
	//The highest chance value in the table.
	uint16 m_maxRoll;
	uint8 m_availableDrops;
	uint32 m_dropableId[NUM_DROPS_IN_DROP_TABLE];
	uint32 m_dropableChance[NUM_DROPS_IN_DROP_TABLE];
};

struct _GAMBLING_ITEM_TABLE
{
	uint32 m_itemId;
	uint32 m_openingCost;
	_ITEM_DROP_TABLE* m_dropTable;
};

//Used in composition
struct _MAKE_ITEM_TABLE
{
	uint32 itemId;
	uint32 reqItemId1;
	uint16 reqItemCount1;
	uint32 reqItemId2;
	uint16 reqItemCount2;
	uint32 reqItemId3;
	uint16 successRate;
	uint32 reqGold;
	uint32 resultItem;
};

struct _MAKE_ITEM_FUSION_TABLE
{
	uint32 itemId;
	uint32 reqItemId1;
	uint16 reqItemCount1;
	uint32 reqItemId2;
	uint16 reqItemCount2;
	uint32 reqItemId3;
	uint16 successRate;
	uint32 reqGold;
	uint32 resultItem;
};

#define MAX_DISMANTLE_RESULTS 4
struct _DISMANTLE_ITEM
{
	uint32 dismantleItemId;
	uint32 rewardItem[MAX_DISMANTLE_RESULTS - 1];
	uint16 rewardMaxCount[MAX_DISMANTLE_RESULTS - 1];
	uint16 chanceForReward;
	uint32 goldReq;
	uint32 specialReward;
	uint16 specialRewardChance;
};
#endif