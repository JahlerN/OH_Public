#ifndef SKILLBOOKTEMPLATE_H
#define SKILLBOOKTEMPLATE_H
#include "..\SharedFiles\globals.h"

//NOTES ABOUT SKILL BOOKS
//Skill book itemId links to skillbookdata ID
//

//There's a separate table about passives.. Jeezus shinkungData, exported it as SQL already.
enum SpecialPointType
{
	STATS = 0,
	LEVEL = 1,
	FORCE = 2,
	SPECIAL_PT_TYPE_END = 3,
};

enum SkillType
{
	SELF_BUFF = 0x00,
	ACTIVE_SKILL = 0x01,
	ACTIVE_BUFF = 0x02,
	PASSIVE_BUFF = 0x03,
};

enum AoeType
{
	SINGLE_TARGET = 0x00,
	SINGLE_TARGET_PIERCE = 0x01,
	RANGE_CENTERED_ON_TARGET = 0x02,
	RANGE_CENTERED_ON_PLAYER = 0x04,
	CHAINING_FROM_TARGET = 0x05,
	TELEPORT = 0x07,
	BUFF_AOE = 0x08,
};

//TODO: Make  this a class instead. Make functions like get total chi usage for level
struct _SKILL_DATA
{
	uint32 m_skillId;
	std::string m_name;
	std::string m_description;
	uint8 m_skillType;
	uint8 m_statType;
	AoeType m_aoeType;
	uint8 m_pierceType;
	uint8 m_maxLevel;
	uint8 m_slot;
	uint8 m_unlockSlot1;
	uint8 m_unlockSlot2;
	uint8 m_levelForUnlock1;
	uint8 m_levelForUnlock2;
	uint32 m_duration;
	uint32 m_durationPerLevel;
	int32 m_chiUsage;
	float m_chiUsagePerLevel;
	int32 m_chiUsagePerSecond;
	float m_chiUsagePerSecondPerLevel;
	int32 m_minDmg;
	float m_minDmgPerLevel;
	int32 m_maxDmg;
	float m_maxDmgPerLevel;
	float m_range;
	float m_rangePerLevel;
	float m_aoe;
	float m_aoePerLevel;
	int32 m_cooldown;
	int32 m_statTypeBonus;
	float m_statTypeBonusPerLevel;
	int8 m_poisonDmg;
	float m_poisonDmgPerLevel;
	int8 m_poisonHeal;
	float m_poisonHealPerLevel;
	int8 m_confusionDmg;
	float m_confusionDmgPerLevel;
	int8 m_confusionHeal;
	float m_confusionHealPerLevel;
	int8 m_paralysisDmg;
	float m_paralysisDmgPerLevel;
	int8 m_paralysisHeal;
	float m_paralysisHealPerLevel;
	int16 m_minDmgBonus;
	float m_minDmgBonusPerLevel;
	int16 m_maxDmgBonus;
	float m_maxDmgBonusPerLevel;
	//Not exactly sure what attack power gives, but i think it's flat to both dmg and sdmg.
	int16 m_attackPower;
	float m_attackPowerPerLevel;
	int16 m_defenseivePower;
	float m_defensivePowerPerLevel;
	uint16 m_injury;
	float m_injuryPerLevel;
	int16 m_accuracy;
	float m_accuracyPerLevel;
	int16 m_dodge;
	float m_dodgePerLevel;
	int16 m_defenseBonus;
	float m_defenseBonusPerLevel;
	float m_movementSpeed;
	float m_movementSpeedPerLevel;
	uint16 m_minHpChange;
	float m_minHpChangePerLevel;
	uint16 m_maxHpChange;
	float m_maxHpChangePerLevel;
	uint16 m_minChiChange;
	float m_minChiChangePerLevel;
	uint16 m_maxChiChange;
	float m_maxChiChangePerLevel;
	uint16 m_healPerSecond;
	float m_healPerSecondPerLevel;
	//Only used in the assassin skill that deals %HP
	uint8 m_percentHp;
	float m_percentHpPerLevel;
	uint8 m_percentChi;
	float m_percentChiPerLevel;
	uint16 m_reflectChance;
	float m_reflectChancePerLevel;
	uint16 m_reflectPercent;
	float m_reflectPercentPerLevel;
	//Proably pretty redundant, it's not even correct all the time, sometimes this values says it's in a book, but it isn't when you look at the book.
	uint32 m_skillBookId;
};

#define MAX_SKILLS_IN_BOOK 24
struct _SKILL_BOOK_DATA
{
	uint32 m_skillBookId;
	std::string m_name;
	uint8 m_bookType;
	uint8 m_classReq;//weaponType in table.
	uint8 m_weaponType;
	uint32 m_levelReq;
	_SKILL_DATA* m_skillData[MAX_SKILLS_IN_BOOK];

	__forceinline bool IsPassiveBook() { return m_bookType == 162; }
};

struct _PLAYER_SKILL_DATA
{
	uint32 m_skillId;
	uint8 m_pointsSpent;
	_SKILL_DATA* m_pData;
};

struct _PLAYER_SKILL_BOOK_DATA
{
	void Initialize(_SKILL_BOOK_DATA* pData)
	{
		m_skillBookId = 0;
		for (int i = 0; i < SPECIAL_PT_TYPE_END; i++)
			m_specialBookStats[i] = 0;
		memset(&m_skillData, 0x00, sizeof m_skillData);
		if (pData == NULL)
			return;

		m_skillBookId = pData->m_skillBookId;
		for (int i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			_SKILL_DATA* pSData = pData->m_skillData[i];
			if (pSData == NULL)
				continue;

			m_skillData[i].m_skillId = pSData->m_skillId;
			m_skillData[i].m_pData = pSData;
		}
	}

	__forceinline bool HasBook() { return m_skillBookId != 0; }

	__forceinline bool HasUnlockedSkill(uint32 skillId)
	{
		for (uint8 i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			if (m_skillData[i].m_skillId == skillId)
				return HasUnlockedSkill(i);
		}
		return false;
	}

	//0 index skillSlot
	__forceinline bool HasUnlockedSkill(uint8 skillSlot)
	{
		skillSlot++;
		bool ret = true;
		for (int i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			if (m_skillData[i].m_skillId == 0 || m_skillData->m_pData == NULL)
				continue;

			//ALL WRONG, theres more skills that unlock one skill, that's what i need to look for...
			if (m_skillData[i].m_pData->m_unlockSlot1 == skillSlot)
				if (m_skillData[i].m_pointsSpent >= m_skillData[i].m_pData->m_levelForUnlock1
					|| m_skillData[i].m_pData->m_levelForUnlock1 == 0)
					ret = true;
				else
					return false;

			if (m_skillData[i].m_pData->m_unlockSlot2 == skillSlot)
				if (m_skillData[i].m_pointsSpent >= m_skillData[i].m_pData->m_levelForUnlock2
					|| m_skillData[i].m_pData->m_levelForUnlock2 == 0)
					ret = true;
				else
					return false;
		}

		return ret;
	}

	__forceinline _PLAYER_SKILL_DATA* GetSkillBySkillId(uint32 skillId)
	{
		for (int i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			_PLAYER_SKILL_DATA* pSkillData = &m_skillData[i];
			if (pSkillData->m_skillId == 0)
				continue;

			if (pSkillData->m_skillId == skillId)
				return pSkillData;
		}
		return NULL;
	}

	__forceinline _PLAYER_SKILL_DATA* GetSkillBySlot(uint8 skillSlot)
	{
		return &m_skillData[skillSlot];
	}

	__forceinline uint8 GetSkillLevelBySlot(uint8 skillSlot)
	{
		int8 pts = -1;
		if (m_skillData[skillSlot].m_skillId != 0)
				pts = m_skillData[skillSlot].m_pointsSpent;
		return pts;
	}

	__forceinline uint8 GetSkillLevelBySkillId(uint32 skillId)
	{
		for (int i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			_PLAYER_SKILL_DATA* pSkillData = &m_skillData[i];
			if (pSkillData->m_skillId == 0)
				continue;

			if (pSkillData->m_skillId == skillId)
				return pSkillData->m_pointsSpent;
		}
	}

	__forceinline int16 GetTotalPointsInSkills()
	{
		int16 pts = -1;
		for (uint8 i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			int8 temp = GetSkillLevelBySlot(i);
			if (temp != -1)
				pts += temp;
		}
		return pts;
	}

	__forceinline uint8 GetTotalSpecialPoints()
	{
		uint8 total = 0;
		for (int i = 0; i < SPECIAL_PT_TYPE_END; i++)
			total += m_specialBookStats[i];
		return total;
	}

	__forceinline bool AddPointToSkill(uint8 slot, uint8 ptsToAdd)
	{
		if (m_skillData[slot].m_skillId == 0)
			return false;

		m_skillData[slot].m_pointsSpent += ptsToAdd;
		return true;
	}

	__forceinline bool RemovePointFromSkill(uint8 slot, uint8 ptsToRemove)
	{
		if (m_skillData[slot].m_skillId == 0)
			return false;

		m_skillData[slot].m_pointsSpent -= ptsToRemove;
		return true;
	}

	uint32 m_skillBookId;
	_PLAYER_SKILL_DATA m_skillData[MAX_SKILLS_IN_BOOK];

	//Stats on divine books.
	//TODO: Add safety on these so points can't be spent on them unless they're the correct book
	uint8 m_specialBookStats[SPECIAL_PT_TYPE_END];
};
#endif