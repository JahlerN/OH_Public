//Part of the CUser class
#include "stdafx.h"
#include "Packets.h"

//SkillId, CooldownTime.
typedef std::map<uint32, time_t> SkillCooldownList;

enum SkillPacketSub
{
	INSERT_SKILLBOOK = 0x01,
	ADD_SKILL_POINT_TO_BOOK_SKILL = 0x02,
	REMOVE_SKILL_POINT_FROM_BOOK_SKILL = 0x03,
	ADD_SPECIAL_POINT_TO_BOOK = 0x05,
	REMOVE_SKILLBOOK = 0x06,
};

void CUser::HandleSkillPackets(Packet& pkt)
{
	uint8 useWepSlot, wepType, unk3;
	uint32 skillId;
	float targetX, targetZ, targetY;
	uint16 target, target2;

	pkt >> useWepSlot >> wepType >> skillId >> targetX >> targetZ >> targetY >> target >> unk3 >> target2;

	if (m_currentAttackRequest.IsAttacking() || m_currentAttackRequest.IsCasting())
		m_currentAttackRequest.FinishCurrentAttack();

	//TODO: We can probably refactor this so we don't need to duplicate the checks for the monk cast reuqest and the other chars requests.
	_PLAYER_SKILL_BOOK_DATA* pBookData = GetSkillbookContainingSkill(skillId);

	if (pBookData == NULL)
		return;

	_PLAYER_SKILL_DATA* pSkillData = pBookData->GetSkillBySkillId(skillId);
	_SKILL_DATA* pSTable = pSkillData->m_pData;
	if (pSTable == NULL)
		return;

	if (pSkillData->m_pointsSpent == 0)
		return;

	//TODO: Figure out chi usage, it seems to increase by 36/37 all the time.
	if (pSTable->m_chiUsage + pSTable->m_chiUsagePerLevel > m_userData->m_curChi)//pSTable->m_chiUsagePerLevel * pSkillData->m_pointsSpent > m_userData->m_curChi)
		return;

	//TODO: This only works sometimes, for some unknown reason.
	if (useWepSlot != m_userData->m_activeWeapon)
		if (!HandleSwitchActiveWeapon(useWepSlot))
			return;

	//player.
	if (target < NPC_START)
	{

	}
	//npc
	else
	{
		CNpc* npc = GetMapInstance()->GetNpcPtr(target);
		//TODO: Check for range to NPC.

		if (!AttemptCastSkillOnNpc(npc, pBookData, pSkillData))
			return;//TODO: Give error to player

		ChiChange(-(pSTable->m_chiUsage + pSTable->m_chiUsagePerLevel * pSkillData->m_pointsSpent));
	}
}

void CUser::HandleTimedCastSkillRequest(Packet& pkt)
{
	uint8 unk;
	uint16 targetId;
	uint32 skillId;
	float tX, tZ, tY;
	
	pkt >> unk >> skillId >> tX >> tZ >> tY >> targetId;

	if (m_currentAttackRequest.IsAttacking() || m_currentAttackRequest.IsCasting())
		m_currentAttackRequest.FinishCurrentAttack();

	_PLAYER_SKILL_BOOK_DATA* pBook = GetSkillbookContainingSkill(skillId);

	if (pBook == NULL || !pBook->HasUnlockedSkill(skillId))
		return;

	_PLAYER_SKILL_DATA* pSkill = pBook->GetSkillBySkillId(skillId);
	_SKILL_DATA* pSTable = pSkill->m_pData;
	if (pSTable->m_chiUsage + pSTable->m_chiUsagePerLevel > m_userData->m_curChi)
		return;

	CNpc* pNpc = GetMapInstance()->GetNpcPtr(targetId);
	if (pNpc == NULL)
		return;

	Packet result(PKT_GAMESERVER_REQUEST_CAST_TIMED_SKILL, uint8(0x10));
	result << GetID()
		<< pNpc->GetX() << pNpc->GetZ() << tY
		<< targetId
		<< skillId;

	SendToRegion(result);

	//if (!AttemptCastSkillOnNpc(pNpc, pBook, pSkill))
		//return;//TODO: Give error to player

	//ChiChange(-(pSTable->m_chiUsage + pSTable->m_chiUsagePerLevel * pSkill->m_pointsSpent));
}

bool CUser::AttemptCastSkillOnNpc(CNpc* pNpc, _PLAYER_SKILL_BOOK_DATA* pBookData, _PLAYER_SKILL_DATA* pSkillData)
{
	_SKILL_DATA* pSTable = pSkillData->m_pData;
	//TODO: Figure out exactly what determines if it's an npc.(It's not only npctype)
	if (pNpc == NULL || pNpc->IsDead() || !pNpc->IsMonster()) // Don't allow attacking npcs
		return false;
	//TODO: Check for range to NPC.

	Packet result(PKT_GAMESERVER_REQUEST_CAST_SKILL);
	result << uint8(0x0A) << uint8(0);
	result << GetID();
	result << uint8(pSTable->m_aoeType);
	result << pSTable->m_skillId;
	result << pNpc->GetX() << pNpc->GetZ() << pNpc->GetY();
	result << uint8(1) << pNpc->GetID();

	int16 cPos = result.wpos();
	result << uint8(0);

	uint8 hitCount = 1;

	result << pNpc->GetID() << uint8(1);

	//TODO: Don't hit npcs.. lol
	//Check for additional targets hit.
	switch (pSTable->m_aoeType)
	{
	case SINGLE_TARGET:
	{
		//Already checked if it's a npc. npc added to packet aswell. So we just need to deal damage.

		//pNpc->HpChange(-GetSkillDamage(pNpc, pSkillData), this, pSkillData->m_skillId);
		std::list<Unit*> npcHit;
		npcHit.push_back(pNpc);
		int32 damage = -GetSkillDamage(npcHit.front(), pSkillData);
		m_currentAttackRequest.SetAttackData(new int32[1]{ damage }, pSkillData, npcHit);
	}
	break;
	case SINGLE_TARGET_PIERCE:
	case RANGE_CENTERED_ON_PLAYER:
	case RANGE_CENTERED_ON_TARGET:
	{
		std::list<Unit*> npcsHit;

		if (pSTable->m_aoeType == RANGE_CENTERED_ON_PLAYER)
			npcsHit = GetMapInstance()->GetListOfNpcsHit(GetX(), GetZ(), (pSTable->m_aoe + pSTable->m_aoePerLevel * pSkillData->m_pointsSpent) * (1 + pBookData->m_specialBookStats[FORCE] / 10));
		else
			npcsHit = GetMapInstance()->GetListOfNpcsHit(pNpc->GetX(), pNpc->GetZ(), (pSTable->m_aoe + pSTable->m_aoePerLevel * pSkillData->m_pointsSpent) * (1 + pBookData->m_specialBookStats[FORCE] / 10));

		//Allways hit target even if outside aoe. But don't add him twice, that's a bad idea.
		if (std::find(npcsHit.begin(), npcsHit.end(), pNpc) == npcsHit.end())
			npcsHit.push_back(pNpc);

		uint16 index = 0;
		int32* damageForNpc = new int32[npcsHit.size()];
		foreach(itr, npcsHit)
		{
			if ((*itr)->GetID() != pNpc->GetID())
				result << (*itr)->GetID() << uint8(1);
			damageForNpc[index++] = -GetSkillDamage((*itr), pSkillData);
		}
		result.put(cPos, uint8(npcsHit.size()));
		m_currentAttackRequest.SetAttackData(damageForNpc, pSkillData, npcsHit);
	}
	break;
	//TODO: Implement hits behind a target. For now it's just a normal aoe
	//case SINGLE_TARGET_PIERCE:
	//{
	//	std::list<CNpc*> npcsHit;
	//	//We have to move the center of the hit to aoe/2 behind the target.
	//	npcsHit = GetMap()->GetListOfNpcsHit();
	//}
	//break;
	default:
		break;
	}
	SendToRegion(result, NULL);
}

void CUser::HandleSkillBookPackets(Packet& pkt)
{
	uint8 subCommand, skillBookSlot, skillSlot, specialPointType;
	uint32 skillBookId;
	pkt >> subCommand;

	switch (subCommand)
	{
	case ADD_SKILL_POINT_TO_BOOK_SKILL:
		pkt >> skillBookSlot >> skillSlot;
		ChangeSkillPointOnSkill(skillBookSlot, skillSlot, 1);
		break;
	case REMOVE_SKILL_POINT_FROM_BOOK_SKILL:
		pkt >> skillBookSlot >> skillSlot;
		ChangeSkillPointOnSkill(skillBookSlot, skillSlot, -1);
		break;
	case ADD_SPECIAL_POINT_TO_BOOK:
		pkt >> skillBookSlot >> specialPointType >> skillBookId;
		AddSpecialPointToBook(skillBookSlot, specialPointType);
		break;
	case REMOVE_SKILLBOOK:
		pkt >> skillBookSlot >> skillBookId;

		if (!RemoveSkillBook(skillBookId, skillBookSlot))
			goto return_fail;
		break;
	}

	return;

return_fail:
	Packet result(PKT_GAMESERVER_SKILLBOOK_SKILL, uint8(subCommand));
	result << uint8(0);
	Send(&result);
}

bool CUser::RemoveSkillBook(uint32 skillBookId, uint8 slot)
{
	if (m_userData->m_skillBookArray[slot].m_skillBookId == skillBookId)
	{
		int16 freedPoints = m_userData->m_skillBookArray[slot].GetTotalPointsInSkills();
		if (freedPoints != -1)
			m_userData->m_spentSkillPoints += -freedPoints;
		m_userData->m_skillBookArray[slot].Initialize(NULL);
	}
	else
		return false;

	Packet result(PKT_GAMESERVER_SKILLBOOK_SKILL, uint8(REMOVE_SKILLBOOK));
	result << uint8(0x0A) << uint8(0);//Success
	result << slot << skillBookId;

	Send(&result);
	SendCharacterUpdate();//If we gained skill points

	return true;
}

bool CUser::ChangeSkillPointOnSkill(uint8 bookSlot, uint8 skillSlot, int8 ptToAdd)
{
	_PLAYER_SKILL_BOOK_DATA* pBData = &m_userData->m_skillBookArray[bookSlot];
	if (pBData->m_skillBookId == 0)
		return false;

	if (pBData->m_skillData[skillSlot].m_skillId == 0)
		return false;

	if (!pBData->HasUnlockedSkill(skillSlot))
		return false;
	
	if (ptToAdd + pBData->m_skillData[skillSlot].m_pointsSpent > pBData->m_skillData[skillSlot].m_pData->m_maxLevel)
		return false;

	if (ptToAdd + pBData->m_skillData[skillSlot].m_pointsSpent < 0)
		return false;

	//If the player has points in a skill after the one he wants to remove pts from, don't allow it.
	if (ptToAdd < 0)
	{
		_SKILL_DATA* pSkillData = pBData->GetSkillBySlot(skillSlot)->m_pData;
		if (pSkillData->m_unlockSlot1 != 0 && pBData->GetSkillBySlot(pSkillData->m_unlockSlot1)->m_pointsSpent > 0 ||
			pSkillData->m_unlockSlot2 != 0 && pBData->GetSkillBySlot(pSkillData->m_unlockSlot2)->m_pointsSpent > 0)
			return false;
	}

	pBData->AddPointToSkill(skillSlot, ptToAdd);
	m_userData->m_spentSkillPoints += ptToAdd;

	Packet result(PKT_GAMESERVER_SKILLBOOK_SKILL, uint8(ADD_SKILL_POINT_TO_BOOK_SKILL));
	result << uint8(0x0A) << uint8(0);
	result << bookSlot << skillSlot;
	result << pBData->m_skillData[skillSlot].m_skillId;
	//TODO: Right now it's just 1 skill pt for 1 level, no max level either. All that has to be fixed but idk where they store stuff like that.
	result << pBData->m_skillData[skillSlot].m_pointsSpent;

	Send(&result);
	SendCharacterUpdate();//Display gain/loss in skill pts.

	return true;
}

//TODO: These points don't affect anything yet, however, they should. To see what they do, add a book and hover over the stats(stats, level, force).
//Added aoe bonus for force tho, not sure if it's correct but i tried.(still need range).
bool CUser::AddSpecialPointToBook(uint8 bookSlot, uint8 specialPointType)
{
	//81 05 0A 00 04 02 43 56 F6 05 01
	_PLAYER_SKILL_BOOK_DATA* pBData = &m_userData->m_skillBookArray[bookSlot];
	if (pBData->m_skillBookId == 0)
		return false;

	if (specialPointType >= SPECIAL_PT_TYPE_END)
		return false;

	if (pBData->GetTotalSpecialPoints() >= 10)
		return false;

	pBData->m_specialBookStats[specialPointType]++;

	Packet result(PKT_GAMESERVER_SKILLBOOK_SKILL, uint8(ADD_SPECIAL_POINT_TO_BOOK));
	result << uint8(0x0A) << uint8(0);
	result << bookSlot << specialPointType;
	result << pBData->m_skillBookId;
	result << pBData->m_specialBookStats[specialPointType];
	Send(&result);
	//SendCharacterUpdate(); Hero sends this, but nothing's changing in that packet when we do this, so...

	return true;
}

int16 CUser::GetSkillLevel(uint32 skillId)
{
	for (int i = 0; i < MAX_EXPANDABLE_SKILLBOOKS; i++)
	{
		_PLAYER_SKILL_BOOK_DATA* pBookData = &m_userData->m_skillBookArray[i];
		if (!pBookData->HasBook())
			continue;

		for (int i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			_PLAYER_SKILL_DATA* pSkillData = &pBookData->m_skillData[i];
			if (pSkillData->m_skillId == 0)
				continue;

			if (pSkillData->m_skillId == skillId)
				return pSkillData->m_pointsSpent;
		}
	}
	return -1;
}

_PLAYER_SKILL_BOOK_DATA* CUser::GetSkillbookContainingSkill(uint32 skillId)
{
	for (int i = 0; i < MAX_EXPANDABLE_SKILLBOOKS; i++)
	{
		_PLAYER_SKILL_BOOK_DATA* pBookData = &m_userData->m_skillBookArray[i];
		if (!pBookData->HasBook())
			continue;

		for (int i = 0; i < MAX_SKILLS_IN_BOOK; i++)
		{
			_PLAYER_SKILL_DATA* pSkillData = &pBookData->m_skillData[i];
			if (pSkillData->m_skillId == 0)
				continue;

			if (pSkillData->m_skillId == skillId)
				return pBookData;
		}
	}

	return NULL;
}

void CUser::FinishAttackRequest()
{
	if (!m_currentAttackRequest.IsCasting() && !m_currentAttackRequest.IsAttacking())
		return;
	uint16 index = 0;
	auto npcsHit = m_currentAttackRequest.GetNpcsHit();
	foreach(itr, npcsHit)
	{
		Unit* pNpc = (*itr);

		if (m_currentAttackRequest.IsAttacking())
			pNpc->HpChange(m_currentAttackRequest.GetDamage(index++), this, -3);
		else
			pNpc->HpChange(m_currentAttackRequest.GetDamage(index++), this, m_currentAttackRequest.GetSkillCast()->m_skillId);
	}

	m_currentAttackRequest.FinishCurrentAttack();
}