#ifndef NPC_H
#define NPC_H
#include "Unit.h"
#include "Define.h"
#define NPC_UNIQUEID_START 20000
#define NPC_AGGO_RANGE 40

enum NpcState
{
	NS_ALIVE,
	NS_STANDING,
	NS_RANDOM_MOVEMENT,
	NS_RETURN_TO_POS_BEFORE_ATTACK_STATE,
	NS_TRACING_PLAYER,
	NS_ATTACKING_PLAYER,
	NS_DEAD,
};

enum DeathState
{
	DS_ALIVE = 0,
	DS_DEAD = 1,
};

struct _PositionBeforeAttackingState
{
	float x;
	float z;
	float y;
};

struct _Target
{
	uint16 tId;
	float tX;
	float tZ;
	float tY;
};

class CNpc : public Unit
{
public:
	CNpc(_NPC_DATA* npcData, _NPC_GROUP* npcGroup, MapInstance* mapInstance);
	~CNpc();

	void Initialize();
	void Revive();

	void UpdateAI(uint32 diff);

	virtual uint16 GetID() { return m_uniqueId; }
	uint16 m_uniqueId;
	__forceinline uint32 GetNpcID() { return m_npcGroup->m_npcId; }

	virtual float GetX() { return m_curX; }
	virtual float GetZ() { return m_curZ; }
	virtual float GetY() { return m_curY; }

	//TODO: This isn't the actual definiton, but it's the best i have right now. Figure out the DB values to fix this. Look at npcType, 12:2, 13:2 those might give it with the right combination
	__forceinline bool IsMonster() { return m_npcData->IsMonster(); }

	virtual std::string GetName() { return m_npcData->m_name; }
	virtual uint32 GetLevel() { return m_npcData->m_level; }
	__forceinline uint32 GetType() { return m_npcData->m_npcType; }
	__forceinline uint32 GetExp() { return m_npcData->m_exp; }
	__forceinline uint32 GetGoldDrop() { return m_npcData->m_gold; }
	__forceinline uint32 GetDropId() { return m_npcData->m_dropId; }

	__forceinline uint8 GetZoneID() { return m_npcGroup->m_zoneId; }
	__forceinline uint32 GetMaxHP() { return m_npcData->m_maxHp; }
	__forceinline uint32 GetMaxChi() { return m_npcData->m_maxChi; }
	__forceinline uint16 GetUnkUse() { return m_npcData->m_unkUse; }
	__forceinline uint32 GetMinDmg() { return m_npcData->m_minDmg; }
	__forceinline uint32 GetMaxDmg() { return m_npcData->m_maxDmg; }
	__forceinline uint32 GetMinSkillDmg() { return m_npcData->m_minSkillDmg; }
	__forceinline uint32 GetMaxSkillDmg() { return m_npcData->m_maxSkillDmg; }
	
	__forceinline uint32 GetRespawnTime() { return m_npcGroup->m_respawnTime; }
	__forceinline uint32 GetNpcType() { return m_npcData->m_npcType; }
	TimeTracker m_killTimeTracker;
	DeathState m_deathState;
	float m_curX, m_curZ, m_curY;
	float m_rotation;
	float m_movingToX, m_movingToZ, m_movingToY;
	uint32 m_curHp, m_curChi;

	virtual void GetInOut(Packet& result, uint8 type);
	void SendInOut(uint8 type);
	void GetNpcInfo(Packet& pkt);

	//TODO: In dark maps stats don't count.
	void HpChange(int amount, Unit* pAttacker = NULL, uint32 skillId = 0);
	void ChiChange(int amount);
	virtual uint32 GetDamage(Unit* pTarget);
	virtual uint32 GetSkillDamage(Unit* pTarget, _SKILL_DATA* pSkillUsed);
	virtual void OnDeath(Unit* pKiller, uint32 skillId);
	virtual bool IsDead() { return m_curHp <= 0 || m_deathState == DS_DEAD; }

	//Movement.
	void HandleMovement();
	void SendMoveTo();

	void SendUpdateHpAndChi(Unit* pAttacker, uint32 skillId = 0);

	//AI Stuff
	std::map<uint16, CUser*> m_visiblePlayers;
	_Target m_target;
	_PositionBeforeAttackingState m_posBeforeAttackState;
	NpcState m_npcState;
	uint32 m_lastAttackTime;
	void ScanForPlayerInAggroRange();
	void HandleAttackingState();
	void AttackUser(CUser* pTarget);
	void HandleReturnState();
	float DistanceToTarget(Unit* target);
	float DistanceToPos(float x, float z);

	void UpdateVisibleUserList();

	//Respawn range?
	//flot X, flot Z, float X2, float Z2
private:
	_NPC_DATA* m_npcData;//Info about the npc.
	_NPC_GROUP* m_npcGroup;
	//_DROP_DATA* m_dropData;
};
#endif