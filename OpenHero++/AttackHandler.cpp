//Part of the CUser class
//TODO: I don't like the splitting of classes.
#include "stdafx.h"
#include "Packets.h"

void CUser::HandleAttack(Packet& pkt)
{
	uint8 attackCounter, distance, unk2;
	uint16 attackId, unk3;
	int32 damage;

	CUser* pAttackUser = NULL;

	pkt >> attackCounter >> distance >> attackId >> unk2 >> unk3;

	if (IsDead() || GetMapInstance() == NULL || m_currentAttackRequest.IsAttacking() || m_currentAttackRequest.IsCasting())// || IsBlinking())
		return;

	Packet result(PKT_GAMESERVER_REQUEST_ATTACK);
	result << attackCounter;
	result << GetID();
	result << uint8(16);//Unk

	//ATtacking a player.
	if (attackId < NPC_START)
	{
		pAttackUser = g_main->GetUserPtr(attackId);//TODO: Allow same faction attack, but mark player as bandit(red) also check for non attack maps
		if (pAttackUser == NULL || pAttackUser->IsDead() || m_userData->m_faction == pAttackUser->m_userData->m_faction)// || pAttackUser->IsBlinking())
			return;

		result << pAttackUser->GetID();

		damage = GetDamage(pAttackUser);

		std::list<Unit*> targets;
		targets.push_back(pAttackUser);

		m_currentAttackRequest.SetAttackData(new int32[1] { -damage }, NULL, targets);

		//Hero sends my hp here for some reason, idk..
	}
	else if (attackId >= NPC_START)
	{
		CNpc* pAttackNpc = GetMapInstance()->GetNpcPtr(attackId);
		if (pAttackNpc == NULL || pAttackNpc->IsDead() || pAttackNpc->GetNpcType() == 0)//TODO: Check if were the same nation aswell.
			return;

		result << pAttackNpc->GetID();

		damage = GetDamage(pAttackNpc);
		std::list<Unit*> targets;
		targets.push_back(pAttackNpc);

		m_currentAttackRequest.SetAttackData(new int32[1]{ -damage }, NULL, targets);
	}

	result << unk2 << uint8(1);
	result << unk3;
	//SERVER_INSTANCE_CHANGE
	SendToRegion(result, NULL);
	//g_main->SendToRegion(result, GetMap(), GetRegionX(), GetRegionZ(), NULL);

	//TODO: Idk what the fuck this packet does. Also packet above is split heavily, not that easy to follow lol
	result.Initialize(0x2F);
	result << uint8(0xFE)
		<< GetID()
		<< uint32(0x000000C8);//TODO: Figure out this value, it's sent in char info stuff too, allways been same tho i think.

	//SERVER_INSTANCE_CHANGE
	SendToRegion(result, NULL);
	//g_main->SendToRegion(result, GetMap(), GetRegionX(), GetRegionZ(), NULL);
}

void CUser::HandleAoeAttack(Packet& pkt)
{
	uint8 attackCounter, distance, unk2, aoeType;
	uint16 attackId, attackIdAgain;
	int32 damage;

	CUser* pAttackUser = NULL;

	pkt >> attackCounter >> distance >> attackId >> unk2;

	if (IsDead() || GetMapInstance() == NULL || m_currentAttackRequest.IsAttacking() || m_currentAttackRequest.IsCasting())// || IsBlinking())
		return;

	Packet result(PKT_GAMESERVER_REQUEST_ATTACK_AOE);
	result << attackCounter;
	result << GetID();
	result << uint8(1);//Unk

						//ATtacking a player.
	//Note, there's no aoe when hitting players, ever. That i know of atleast.
	if (attackId < NPC_START)
	{
		pAttackUser = g_main->GetUserPtr(attackId);//TODO: Allow same faction attack, but mark player as bandit(red) also check for non attack maps
		if (pAttackUser == NULL || pAttackUser->IsDead() || m_userData->m_faction == pAttackUser->m_userData->m_faction)// || pAttackUser->IsBlinking())
			return;

		result << pAttackUser->GetID();
		result << uint16(0);
		result << uint8(1);
		result << pAttackUser->GetID();
		result << uint16(0x01D9);

		damage = GetDamage(pAttackUser);

		std::list<Unit*> targets;
		targets.push_back(pAttackUser);

		m_currentAttackRequest.SetAttackData(new int32[1]{ -damage }, NULL, targets);

		//Hero sends my hp here for some reason, idk..
	}//TODO: Revise this, not that great.
	else if (attackId >= NPC_START)
	{
		CNpc* pAttackNpc = GetMapInstance()->GetNpcPtr(attackId);
		if (pAttackNpc == NULL || pAttackNpc->IsDead() || pAttackNpc->GetNpcType() == 0)//TODO: Check if were the same nation aswell.
			return;

		std::list<Unit*> hitList = GetMapInstance()->GetListOfNpcsHit(pAttackNpc->GetX(), pAttackNpc->GetZ(), 2.0f);

		if (std::find(hitList.begin(), hitList.end(), pAttackNpc) == hitList.end())
			hitList.push_back(pAttackNpc);

		result << pAttackNpc->GetID();
		result << uint16(0);
		int16 cPos = result.wpos();
		result << uint8(0);
		result << pAttackNpc->GetID();
		result << uint16(0x01D9);

		uint8 counter = 1;

		int32* targetDamage = new int32[hitList.size()];

		foreach(itr, hitList)
		{
			Unit* temp = (*itr);
			if (temp->GetID() == pAttackNpc->GetID())
				continue;
			result << temp->GetID();
			result << uint16(0x01D9);
			counter++;
			targetDamage[counter -= 2] = -GetDamage(temp);
		}

		m_currentAttackRequest.SetAttackData(targetDamage, NULL, hitList);

		result.put(cPos, counter);
	}
	//SERVER_INSTANCE_CHANGE
	SendToRegion(result, NULL);
	//g_main->SendToRegion(result, GetMap(), GetRegionX(), GetRegionZ(), NULL);
}
