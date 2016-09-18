#include "stdafx.h"
#include "Packets.h"

CNpc::CNpc(_NPC_DATA* npcData, _NPC_GROUP* npcGroup, MapInstance* mapInstance)
{
	if (npcData == NULL || npcGroup == NULL)
	{
		printf("Unable to load npc data! Not good!.");
		ASSERT(0);
	}
	m_npcData = npcData;
	m_npcGroup = npcGroup;
	m_npcId = npcGroup->m_npcId;
	m_curMapInstance = mapInstance;
	Initialize();

	m_curHp = m_npcData->m_maxHp;
	m_curChi = m_npcData->m_maxChi;
	m_rotation = 0.0f;
	m_lastAttackTime = 0;
	memset(&m_target, 0x00, sizeof(_Target));

	m_npcState = NS_STANDING;
	m_deathState = DS_ALIVE;
}

CNpc::~CNpc()
{

}

void CNpc::Initialize()
{
	std::random_device r;
	std::mt19937 rng(r());
	std::uniform_real_distribution<> disX(m_npcGroup->m_minX, m_npcGroup->m_maxX);
	m_curX = disX(rng);

	//Shits real stupid, but so are hero devs.(Sometimes min is bigger than max)
	if (m_npcGroup->m_minZ > m_npcGroup->m_maxZ)
	{
		std::uniform_real_distribution<> disZ(m_npcGroup->m_maxZ, m_npcGroup->m_minZ);
		m_curZ = disZ(rng);
	}
	else
	{
		std::uniform_real_distribution<> disZ(m_npcGroup->m_minZ, m_npcGroup->m_maxZ);
		m_curZ = disZ(rng);
	}
	m_curY = 0;
	m_movingToX = m_curX;
	m_movingToZ = m_curZ;
	m_movingToY = m_curY;

	SetRegion(GetNewRegionX(), GetNewRegionZ());
}

void CNpc::UpdateVisibleUserList()
{
	std::map<uint16, CUser*> tempUserMap;
	//Get all players in region.
	foreach_region(x, z)
	{
		Region* pRegion = GetMapInstance()->GetRegion(GetRegionX() + x, GetRegionZ() + z);
		if (pRegion == NULL || pRegion->m_regionUserMap.IsEmpty())
			continue;

		auto m = pRegion->m_regionUserMap.m_UserTypeMap;
		for (auto user = m.begin(); user != m.end();)// foreach(user, m)//TODO: This might fuck up if a user changes region while were updating this.
		{
			CUser* pUser = g_main->GetUserPtr((*user).first);
			user++;
			if (pUser == NULL || !pUser->IsInGame())
				continue;

			tempUserMap.insert(make_pair(pUser->GetID(), pUser));
		}
	}

	foreach(user, m_visiblePlayers)
	{
		//Player is still within visibilty.
		if (tempUserMap.find(user->first) != tempUserMap.end())
			continue;
		else//He's too far away and shouldn't see me right now, remove me from his client.
		{
			CUser* pUser = user->second;
			if (pUser == NULL || !pUser->IsInGame())
				continue;
			Packet result;
			GetInOut(result, 2);
			pUser->Send(&result);
		}
	}

	m_visiblePlayers.clear();
	m_visiblePlayers.insert(tempUserMap.begin(), tempUserMap.end());
}

void CNpc::UpdateAI(uint32 diff)
{
	if (!IsMonster())
	{
		UpdateVisibleUserList();
		return;
	}

	if (IsDead())
	{
		m_killTimeTracker.Update(diff);
		if (m_killTimeTracker.Passed())
			Revive();
		else
			return;
	}

	//Update the list of users we can possibly kill, and hide from them when they're realy far away.
	UpdateVisibleUserList();

	return;//TODO: When i get back to ai stuff. just make them move to the player if he's in aggro range. Then make them attack him if in aggro range, then make them move to him if he moves. We need to make this in steps otherwise it's gonna be  impossible to make and understand, i'm not good at AI :(
	switch (m_npcState)
	{
	case NS_STANDING:
		ScanForPlayerInAggroRange();
		break;
	case NS_RANDOM_MOVEMENT:
		break;
	case NS_ATTACKING_PLAYER:
		HandleAttackingState();
		break;
	case NS_RETURN_TO_POS_BEFORE_ATTACK_STATE:
		HandleReturnState();
		break;
	default:
		break;
	}
}

void CNpc::Revive()
{
	//They have already been initialized to a new spot and added to the region, but they're still considered dead so not shown to clients
	m_curHp = m_npcData->m_maxHp;
	m_curChi = m_npcData->m_maxChi;
	m_deathState = DS_ALIVE;
	SendInOut(1);
}

void CNpc::GetInOut(Packet & result, uint8 type)
{
	result.Initialize(PKT_GAMESERVER_REGION_CHANGE_NPC, uint8(type));
	//result << GetID();
	if (type != 2)//Not out
		GetNpcInfo(result);
	else
	{
		result << GetID()
			<< GetRegionX()
			<< GetRegionZ()
			<< GetX()
			<< GetZ();
	}
}

void CNpc::SendInOut(uint8 type)
{
	if (GetRegion() == NULL)
	{
		SetRegion(GetNewRegionX(), GetNewRegionZ());
		if (GetRegion() == NULL)
			return;
	}

	if (type == 2)
	{
		GetRegion()->Remove(this);
	}
	else
	{
		GetRegion()->Add(this);
	}

	Packet result;
	GetInOut(result, type);
	//SERVER_INSTANCE_CHANGE
	SendToRegion(result, NULL);
	//g_main->SendToRegion(result, GetMap(), GetRegionX(), GetRegionZ(), NULL);
}

void CNpc::GetNpcInfo(Packet & pkt)
{
	pkt << GetID()
		<< GetNpcID()
		<< GetLevel()
		//TODO: I'm pretty sure exp fucks some monsters up, can't say for sure tho. All of the below is trying to find the stuff that makes npcs
		//look like npcs. Worth checking is advanced guy in DC, scarecrow, consigment. These npcs have the same values pretty much but scarecrow are 
		//considered monsters in real hero.
		<< uint32(IsMonster() ? -1 : 0) //was FFFFFFFF on viper lord, 0 on escort warrior
		<< GetName()
		<< m_curHp
		<< m_curChi
		<< GetMaxHP()
		<< GetMaxChi()
		<< uint8(1)//1 1 on npcs, 0x12 1 on viper lord
		<< uint8(1)
		<< m_curX
		<< m_curZ
		<< m_curY
		<< m_movingToX
		<< m_movingToZ
		<< m_movingToY
		<< float(2)//Moving speed,
		<< uint16(0xFFFF)//100% unk
		<< uint8(0)
		<< GetUnkUse()
		<< uint16(0)
		<< uint16(0)
		<< uint16(m_npcData->m_npcType == 0 ? 1 : 0);//Not right probably
}

void CNpc::HpChange(int amount, Unit* pAttacker, uint32 skillId /*= 0*/)
{
	if (IsDead())
		return;

	if (amount < 0 && -amount > m_curHp)
	{
		m_curHp = 0;
		OnDeath(pAttacker, skillId);
		return;
	}
	else if (amount >= 0 && m_curHp > GetMaxHP())
		m_curHp = GetMaxHP();
	else
		m_curHp += amount;

	SendUpdateHpAndChi(pAttacker, skillId);
}

void CNpc::ChiChange(int amount)
{
	if (amount < 0 && -amount > m_curChi)
		m_curChi = 0;
	else if (amount >= 0 && m_curChi > GetMaxChi())
		m_curChi = GetMaxChi();
	else
		m_curChi += amount;

	SendUpdateHpAndChi(NULL);
}

void CNpc::SendUpdateHpAndChi(Unit* pAttacker, uint32 skillId /*= 0*/)
{
	if (skillId == 0)
		skillId = -1;
	Packet result(PKT_GAMESERVER_CHARACTER_UPDATE_CURHPCHI);
	result << GetID();
	if (pAttacker != NULL)
		result << pAttacker->GetID();
	else
		result << GetID();
	result << m_curHp
		<< m_curChi
		<< skillId//Making this 0 made it show no color at all. Maybe everything here is just color values? -1 is gray, think only 0 is white
		<< uint32(0)
		<< uint8(0)//Skill id hit by! Determines the color of the hit.
		<< uint8(1)
		<< uint8(0);

	//SERVER_INSTANCE_CHANGE
	SendToRegion(result, NULL);
	//g_main->SendToRegion(result, GetMap(), GetRegionX(), GetRegionZ(), NULL);
}

void CNpc::OnDeath(Unit* pKiller, uint32 skillId)
{
	m_killTimeTracker.Reset(GetRespawnTime() * 1000);
	m_deathState = DS_DEAD;

	CUser* pUserKiller = dynamic_cast<CUser*>(pKiller);
	if (pUserKiller != NULL)
	{
		if (pUserKiller->GetLevel() > 200)
			pUserKiller->ExpChange(m_npcData->m_darknessExp);
		else if (pUserKiller->GetLevel() > 100)
			pUserKiller->ExpChange(m_npcData->m_divineExp);
		else
			pUserKiller->ExpChange(GetExp());

		pUserKiller->GoldChange(GetGoldDrop());

		auto drops = sItemMgr->GenerateItemDropList(m_npcData);//g_main->GenerateItemDropList(m_npcData->m_dropId, m_npcData->m_lootRolls);

		std::list<_ZONE_ITEM*> itemList;
		if (drops.size() > 0)
		{
			uint16 startX = GetX() - drops.size() / 2, startZ = GetZ() - drops.size() / 2;
			uint16 offsetX = 0, offsetZ = 0;
			//TODO: Make each item not drop on the same spot so like, first drops on monster pos, next drops around that, then around that etc..
			foreach(itr, drops)
			{
				_ITEM_DATA* pItem = (*itr);
				if (pItem == NULL)
					continue;
				
				_ZONE_ITEM* pZoneItem = new _ZONE_ITEM();
				//m_regionItemId is set at the add item to region function.
				pZoneItem->m_pItem = pItem;
				pZoneItem->m_x = startX + offsetX++;
				pZoneItem->m_z = startZ + offsetZ;
				pZoneItem->m_dropTracker.Reset(ITEM_DROP_EXPIRATION_TIME);
				pZoneItem->m_expirationTracker.Reset(ITEM_DROP_EXPIRATION_TIME);
				pZoneItem->m_reservedForuserId = pKiller->GetID();
				itemList.push_back(pZoneItem);

				if (offsetX >= drops.size() / 4)//+1
				{
					offsetX = 0;
					offsetZ++;
				}
			}
		}

		GetMapInstance()->RegionItemAdd(itemList);
	}
	SendUpdateHpAndChi(pKiller, skillId);

	GetMapInstance()->RegionNpcRemove(GetRegionX(), GetRegionZ(), GetID());
	Initialize();
	GetMapInstance()->RegionNpcAdd(GetRegionX(), GetRegionZ(), GetID());
}

uint32 CNpc::GetDamage(Unit* pTarget)
{
	return 0;
}

uint32 CNpc::GetSkillDamage(Unit * pTarget, _SKILL_DATA * pSkillUsed)
{
	return uint32();
}

void CNpc::HandleMovement()
{
	if (m_curX < m_movingToX)
		m_curX += 6 * 0.2;
	else
		m_curX -= 6 * 0.2;
	if (m_curZ < m_movingToZ)
		m_curZ += 6 * 0.2;
	else
		m_curZ -= 6 * 0.2;
}

void CNpc::SendMoveTo()
{
	Packet result(PKT_GAMESERVER_NPC_MOVE);
	result << GetID()
		<< uint8(1)
		<< m_curX
		<< m_curZ
		<< m_curY
		<< m_movingToX
		<< m_movingToZ
		<< m_movingToY
		<< float(1) //rotation.
		<< uint8(0);

	SendToRegion(result, NULL);
}

//AI STUFF
void CNpc::ScanForPlayerInAggroRange()
{
	//Already following someone.
	if (m_target.tId != 0)
		return;
	foreach(user, m_visiblePlayers)
	{
		CUser* pUser = g_main->GetUserPtr((*user).first);
		if (pUser == NULL || !pUser->IsInGame())
			continue;

		//Calc distance to player.
		if (DistanceToTarget(pUser) <= NPC_AGGO_RANGE)
		{
			//Is withing range.
			m_target.tId = pUser->GetID();
			m_target.tX = pUser->GetX();
			m_target.tZ = pUser->GetZ();
			m_target.tY = pUser->GetY();
			m_npcState = NS_ATTACKING_PLAYER;
			m_posBeforeAttackState.x = GetX();
			m_posBeforeAttackState.z = GetZ();
			m_posBeforeAttackState.y = GetY();
			return;
		}
	}
}

void CNpc::HandleAttackingState()
{
	CUser* pTarget = g_main->GetUserPtr(m_target.tId);
	if (pTarget == NULL)
	{
		m_npcState = NS_RETURN_TO_POS_BEFORE_ATTACK_STATE;
		memset(&m_target, 0x00, sizeof(_Target));
		return;
	}

	int distanceToOrigin = DistanceToPos(m_posBeforeAttackState.x, m_posBeforeAttackState.z);
	if (distanceToOrigin > 40)//Max tracking range
	{
		memset(&m_target, 0x00, sizeof(_Target));
		m_npcState = NS_RETURN_TO_POS_BEFORE_ATTACK_STATE;
		return;
	}

	//If were within attack range, attack him
	if (DistanceToTarget(pTarget) < 8)
	{
		if (m_lastAttackTime + 1000 < GetMSTime())
			AttackUser(pTarget);
		return;
	}
	//Outside attack range but still following.
	else
	{
		//Don't change movement if play is standing still.
		if (m_movingToX == pTarget->GetX() &&
			m_movingToZ == pTarget->GetZ() &&
			m_movingToY == pTarget->GetY())
			return;
		m_movingToX = pTarget->GetX();
		m_movingToZ = pTarget->GetZ();
		m_movingToY = pTarget->GetY();
		HandleMovement();
		return;
	}
}

void CNpc::AttackUser(CUser* pTarget)
{
	pTarget->HpChange(-1, this, -1);
	m_lastAttackTime = GetMSTime();
	if (pTarget->IsDead())
	{
		memset(&m_target, 0x00, sizeof(_Target));
		m_npcState = NS_RETURN_TO_POS_BEFORE_ATTACK_STATE;
	}
}

void CNpc::HandleReturnState()
{
	if (m_curX == m_posBeforeAttackState.x + 10.0f ||
		m_curX == m_posBeforeAttackState.x - 10.0f &&
		m_curZ == m_posBeforeAttackState.z + 10.0f ||
		m_curZ == m_posBeforeAttackState.z - 10.0f)
	{
		m_movingToX = m_curX;
		m_movingToZ = m_curZ;
		m_movingToY = m_curY;
		HandleMovement();
		m_npcState = NS_STANDING;
		return;
	}

	m_movingToX = m_posBeforeAttackState.x;
	m_movingToZ = m_posBeforeAttackState.z;
	m_movingToY = m_posBeforeAttackState.y;
	HandleMovement();
}

float CNpc::DistanceToPos(float x, float z)
{
	return sqrt(pow(GetX() - x, 2) + pow(GetZ() - z, 2));
}

float CNpc::DistanceToTarget(Unit* target)
{
	return DistanceToPos(target->GetX(), target->GetZ());
}