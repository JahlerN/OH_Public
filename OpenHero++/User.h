#ifndef USER_H
#define USER_H
#include "..\SharedFiles\globals.h"
#include "../SharedFiles/Timer.h"
#include "SkillBookTemplate.h"
#include "Unit.h"
#include "ServerInstance.h"
#include "ChatHandler.h"
#include "ItemTemplate.h"
//#include "MAP.h"

#define MAX_USER_CHARACTERS 6
#define ABNORMAL_BLINKING 0x04
#define ABNORMAL_NORMAL 0x03

#define MEDITATION_TICK 3000//every 3 seconds

#define MAX_EXPANDABLE_SKILLBOOKS 7
//Only 6 of them are shown, but the last 3 might be the widen books
#define MAX_PASSIVE_BOOKS 9
enum GameState
{
	GAME_STATE_CONNECTED,
	GAME_STATE_INGAME
};

//TODO: Right now there's no way to know if the client actualy finished casting, so we just have to trust the client that they actualy did before requesting again.
struct CurrentAttackRequest
{
	CurrentAttackRequest()
	{
		m_isAttacking = false;
		m_damage = NULL;
		m_pSkillData = NULL;
	}

	bool m_isAttacking;
	int32* m_damage;
	_PLAYER_SKILL_DATA* m_pSkillData;
	std::list<Unit*> m_unitHitBySkill;

	void SetAttackData(int32* damage, _PLAYER_SKILL_DATA* pSkillData, std::list<Unit*> npcsHitBySkill)
	{
		m_damage = damage;
		m_pSkillData = pSkillData;
		if (m_pSkillData == NULL)
			m_isAttacking = true;
		m_unitHitBySkill = npcsHitBySkill;
	}

	int32 GetDamage(uint16 at) { return m_damage[at]; }
	_PLAYER_SKILL_DATA* GetSkillCast() { return m_pSkillData; }

	std::list<Unit*> GetNpcsHit() { return m_unitHitBySkill; }

	bool IsCasting() { return m_pSkillData != NULL; }
	bool IsAttacking() { return m_isAttacking; }
	void FinishCurrentAttack() 
	{ 
		if (m_damage != NULL) { delete[] m_damage; m_damage = NULL; } 
		m_pSkillData = NULL; 
		m_unitHitBySkill.clear(); 
		m_isAttacking = false; 
	}
};

struct _USER_DATA
{
	uint32 m_charUniqueId;
	char m_charId[MAX_ID_SIZE + 1];
	uint8 m_authority;//TODO: GM flag etc, not implemented at all.
	uint8 m_charCount;
	uint32 m_morphId;

	uint8 m_zone;
	float m_curPosX, m_curPosZ, m_curPosY;

	uint8 m_faction;
	uint8 m_charType;//This has 18 values, 1 for each char type + divine and darkness.
	uint8 m_class; //Common, etc..

	uint16 m_curHp;
	uint16 m_curChi;
	uint16 m_stats[STAT_END];
	uint32 m_injury;

	uint32 m_hair;
	uint32 m_face;
	uint8 m_height;

	bool m_showHelm;
	bool m_showMask;
	uint8 m_showHtItems;
	uint8 m_activeWeapon;

	//uint8 m_rank;
	//uint8 m_title;
	uint32 m_level;
	uint64 m_exp;
	uint32 m_fame;

	uint64 m_gold, m_warehouseGold;

	uint16 m_clanId;
	uint8 m_skillTabs[40];
	uint16 m_questCount;
	char m_quests[255];//TODO: Make define

	_ITEM_DATA m_itemArray[IS_END - 1];//MAX_EQUIP_SLOTS + MAX_INVENTORY_SLOTS];// *2];//*2 for premium tab TODO: Make actual definition of this
	_ITEM_DATA m_warehouseArray[MAX_WAREHOUSE_SLOTS * 2];//*2 for premium slots;
	//Contains active skill books then passive books
	_PLAYER_SKILL_BOOK_DATA m_skillBookArray[MAX_EXPANDABLE_SKILLBOOKS + MAX_PASSIVE_BOOKS];
	uint32 m_spentSkillPoints;
};

//TODO: Find a more suitable place for this
#define HP_PER_STR 10
#define DMG_PER_STR 1
#define SDMG_PER_STR 1 //also 20% gets converted so it's 1.2 in reality
#define ACC_PER_STR 0.95

#define DEF_PER_DEX 1
#define SDEF_PER_DEX 1
#define DODGE_PER_DEX 1
#define CRIT_PER_DEX 0.005f
#define ATKSPD_PER_DEX 0.1f

#define CHI_PER_INT 3
#define SDMG_PER_INT 0.36
#define SDEF_PER_INT 2

#define DMG_PER_NATURE_STAT 0.2f
#define SDMG_PER_NATURE_STAT 1
#define DEF_PER_NATURE_STAT 0.1f
#define SDEF_PER_NATURE_STAT 2
struct _CHARACTER_DATA//This is the char data table.
{
	uint32 m_typeId;
	uint8 m_nextTypeId;
	uint32 m_baseHp;
	uint32 m_baseChi;
	uint32 m_baseStr;
	uint32 m_baseDex;
	uint32 m_baseInt;
	uint8 m_hpPerLevel;
	uint8 m_chiPerLevel;
	float m_dmgPerLevel;
	float m_sdmgPerLevel;
	float m_defPerLevel;
	float m_sdefPerLevel;
};

class GameServer;
class CUser : public Unit, public OHSocket
{
public:
	_USER_DATA* m_userData;
	std::string m_accountId;
	int8 m_serverTab;
	int8 m_serverId;
	bool m_selectedCharacter;

	ServerInstance* m_curServerInstance;
	__forceinline void SetServerInstance(ServerInstance* pServerInstance) { m_curServerInstance = pServerInstance; }
	void SendToServerInstance(Packet& pkt, CUser* pExceptUser = NULL);

	std::unordered_map<uint32, uint8> m_activeSetBonuses;
	CUser* m_target;
	//MAP* m_curMap;

	uint8 m_abnormalType;//Blinking, invisible to players etc, i think. Starts as 4(blinking)

	uint32 m_maxHp, m_maxChi;
	uint16 m_minDamage, m_maxDamage;
	uint32 m_minSkillDamage, m_maxSkillDamage;
	uint32 m_def, m_pvpDef;
	uint32 m_skillDef, m_pvpSkillDef;
	uint16 m_accuracy, m_dodge;
	uint16 m_poisonAtk, m_paralysisAtk, m_confusionAtk;
	uint16 m_poisonDef, m_paralysisDef, m_confusionDef;
	uint32 m_pvpDamage, m_pvpSkillDamage;
	//This is divided by 10 on the client.
	uint32 m_attackSpeed;
	uint16 m_crit;
	float m_movementSpeed;
	//uint16 m_hpRecoverySpeed;
	//uint16 m_mpRestoreSpeed;
	//TODO: There are more values here. Like hp recovery%, chi recovery
	uint16 m_itemMinDmg, m_itemMaxDmg;
	double m_itemDelay;
	double m_itemAttackRange;
	double m_itemAttackAoE;
	uint16 m_itemStatBonus[STAT_END];
	uint16 m_itemPoisonDmg, m_itemParalysisDmg, m_itemConfusionDmg;
	uint16 m_itemPoisonDef, m_itemParalysisDef, m_itemConfusionDef;
	uint32 m_itemMaxHp, m_itemMaxChi;
	uint16 m_itemHpRecoveryPercent, m_itemChiRecoveryPercent;
	float m_itemMovementSpeed;
	uint32 m_itemMinBonusDmg, m_itemMaxBonusDmg;
	uint16 m_itemBonusDmgPercent;
	uint32 m_itemMinBonusSDmg, m_itemMaxBonusSDmg;
	uint16 m_itemBonusSDmgPercent;
	uint32 m_itemDefense, m_itemSDefense;
	uint16 m_itemDefensePercent, m_itemSDefensePercent;
	uint16 m_itemAccuracy, m_itemDodge;
	uint16 m_itemCritChance;
	uint16 m_itemBonusExp;
	uint16 m_itemDropBonus;
	uint16 m_itemBonusMineral;

	//Timers
	TimeTracker m_blinkTimer;
	int8 m_blinkCount;
	TimeTracker m_meditationTimer;
	//float m_meditationTick; 

	CNpc* m_currentGossip;


	void Update(uint32 diff);

	virtual uint16 GetID() { return GetSocketID(); }

	virtual float GetX() { return m_userData->m_curPosX; }
	virtual float GetZ() { return m_userData->m_curPosZ; }
	virtual float GetY() { return m_userData->m_curPosY; }

	bool GoToXZ(uint16 posX, uint16 posZ);
	//TODO: Make an option to stay on the same coordinates.
	void ChangeZone(_ZONECHANGE_DATA* pZone);
	void ChangeZone(_ZONESTART_POSITION* pZone);

	virtual const char* GetName() { return m_userData->m_charId; }

	void BlinkStart();
	void UpdateBlinkTime(uint32 diff);
	__forceinline bool IsBlinking() { return m_abnormalType == ABNORMAL_BLINKING; }
	__forceinline virtual bool IsDead() { return m_userData->m_curHp <= 0; }
	__forceinline GameState GetState() { return m_state; }
	__forceinline bool IsInCombateState() { return m_inCombatState; }
	__forceinline void ToggleCombatState() { m_inCombatState = !m_inCombatState; }
	__forceinline bool IsInGame() { return GetState() == GAME_STATE_INGAME; }
	__forceinline bool IsMeditating() { return m_isMeditating; }
	__forceinline void ToggleMeditation() { m_isMeditating = !m_isMeditating; }
	void HandleMeditating(uint32 diff);

	virtual void GetInOut(Packet& pkt, uint8 inOutType);
	void UserLeaveEnterRegion(uint8 type);

	uint32 GetLevel();
	std::string GetLevelString();
	uint32 GetTotalSpentStatPoints();
	uint16 GetFreeStatPoints();
	uint32 GetTotalSpentElementPoints();
	uint16 GetFreeElementPoints();
	__forceinline uint32 GetTotalSkillPoints();
	__forceinline uint32 GetFreeSkillPoints() { return GetTotalSkillPoints() - GetSpentSkillPoints(); }
	__forceinline uint32 GetSpentSkillPoints() { return m_userData->m_spentSkillPoints; }

	__forceinline uint16 GetStat(PlayerStats type) { return m_userData->m_stats[type]; }
	__forceinline void SetStat(PlayerStats type, uint16 val) { m_userData->m_stats[type] = val; }
	__forceinline uint16 GetItemStatBonus(PlayerStats type) { return m_itemStatBonus[type]; }
	__forceinline void SetItemStatBonus(PlayerStats type, uint16 val) { m_itemStatBonus[type] = val; }
	__forceinline uint32 GetStatAll(PlayerStats type) { return m_itemStatBonus[type] + m_userData->m_stats[type]; }
	//TODO: Impelement statBuffs
	virtual uint32 GetDamage(Unit* pTarget);
	uint32 GetSkillDamage(Unit* pTarget, _PLAYER_SKILL_DATA* pSkillUsed);

	void ResetItemSlotValues();
	void UpdateItemSlotValues();
	void SetItemStats(_ITEM_TABLE* pTable);
	void UpdateHpChi();
	void UpdateUserStats();

	void SendUpdateHpAndChi(Unit* pTarget);

	void SendMorphPlayer();

	void SendMyInfo();
	void SendCharacterUpdate();

	ByteBuffer GetUserInfoForRegion();

	void ExpChange(uint64 exp);
	void SendExpChange();
	void LevelChange(uint32 newLevel);

	//Skill stuff
	//void AddSkillBook();
	void SendSkillBookAdded(_PLAYER_SKILL_BOOK_DATA* pData, uint8 pos);

	virtual void OnConnect();
	virtual void Initialize();
	virtual void OnDisconnect();

	bool HandlePacket(Packet& pkt);

	void HandleVersionCheck(Packet& pkt);
	void HandleAllCharListRequest(Packet& pkt);
	void HandleCharCreation(Packet& pkt);
	void HandleDeleteCharacter(Packet& pkt);
	void HandleSelectCharacter(Packet& pkt);
	void HandleLoadingscreenUnk1(Packet& pkt);
	void HandleLoadingscreenUnk2(Packet& pkt);
	void HandleLoadingscreenUnk3(Packet& pkt);
	void HandleCharacterMovement(Packet& pkt);

	void HandleNpcGossip(Packet& pkt);
	void HandleNpcConvoTesting(Packet& pkt);

	void SendNpcGossip(CNpc* pNpc);
	void SendNpcGossip(uint32 gossipId);
	void SendOpenNpcExchange(_SHOP_TABLE* pShop);
	void SendOpenWorldMapTeleporter();

	void HandleWorldMapTeleporter(Packet& pkt);

	void RemoveOldPlayersInVision(int x, int z);
	void RemovePlayersOutsideOfVision(Region* pRegion);

	void HandleZoneChange(Packet& pkt);
	void HandleMoveItem(Packet& pkt);
	void HandleShowItemChange(Packet& pkt);
	void HandleChangeCombatState(Packet& pkt);
	void HandleMeditation(Packet& pkt);

	void HandleAddStatPt(Packet& pkt);

	void SendGlobalNotice(std::string msg);
	//void HandleSendLocalMessage(Packet& pkt);
	//void HandleCommand(Packet& pkt);
	void HandleTargetChange(Packet& pkt);

	void GoldChange(int amount, bool updateClient = true);
	virtual void HpChange(int amount, Unit* pAttacker = NULL, uint32 skillId = 0);
	virtual void ChiChange(int amount);
	virtual void OnDeath(Unit* pKiller);

#pragma region InventoryHandler

	bool HandleSwitchActiveWeapon(uint8 newActiveSlot);

	//Find the first free slot or the first stackable slot of itemId
	int FindFreeInventorySlot(uint32 itemId = 0, uint16 count = 1);
	bool HasItemEquipped(uint32 itemId);
	bool HasItemCount(uint32 itemId, uint16 count);
	bool HasItem(uint32 itemId);
	uint16 CountFreeInventorySlots();
	//returns the first pos of given itemId, -1 if no item was found
	int16 FindItemPos(uint32 itemId, uint16 startPos = 0);
	//returns the total count of given itemId
	uint32 FindItemTotalCount(uint32 itemId);

	bool GiveItem(uint32 itemId, uint16 count = 1);
	bool GiveItem(uint32 itemId, int pos, uint16 count = 1);
	//Give an already created item(Can be used in trades aswell i guess?)
	void GiveItem(_ITEM_DATA* pItem, int pos = -1);
	void GiveGambledItem(_ITEM_DATA* pItem, _ITEM_TABLE* pTable, _GAMBLING_ITEM_TABLE* pGamble, int16 slot);
	bool RemoveItem(int16 slot, uint16 count = 1);
	//Use this if you want to remove stacks of items.
	int16 RemoveItem(uint32 itemId, uint16 count = 1);

//	uint16 GetValidInventoryPos(uint16 pos);
	bool IsEquipSlot(uint16 slot);
	void HandleUseItem(_ITEM_DATA* pItem);
	void HandleGambleItem(uint32 itemId);
	bool CanEquipItem(_ITEM_TABLE* pTable);
	void HandleOpenMultiContainerItem(uint16 slot);
	void SendMoveItemStack(uint32 itemId, uint16 oldCount, uint16 newCount, uint16 oldPos, uint16 newPos);
	void SendMoveItem(uint32 itemId, uint16 oldPos, uint16 newPos);
	void SendMoveItemItem(uint32 oldItemId, uint32 newItemId, uint16 oldPos, uint16 newPos);
	void SendMoveItemSplitStack(_ITEM_TABLE* pItem, uint16 oldCount, uint16 newCount, uint16 oldPos, uint16 newPos);
	void SendBoughtItem(_ITEM_TABLE* pTable, uint16 count, uint32 duration, uint16 pos);
	void SendSoldItem(_ITEM_TABLE* pTable, uint16 pos);
	void SendAcquireItem(_ITEM_DATA* pItem, _ITEM_TABLE* pTable, uint16 pos, bool newItem = false);
	void SendUseItem(uint32 itemId);
	void SendRemoveItem(uint32 itemId, uint16 pos);
	void SendRemoveItemCount(_ITEM_DATA* pItem, uint16 slot);
	void SendUpdateItemSlot(_ITEM_TABLE* pTable, uint16 count);
	void SendItemAddedToSlot(uint32 itemId, uint8 rarity, uint16 count, int16 slot);
	void SendRemoveMultiSlotContainer(uint32 itemId, int16 slot);
	void SendUsePotion();
	void SendVisibleItemListToRegion();
	//Doesn't show up in the chat log.
	void SendUpdateGoldSilent();
	ByteBuffer GetVisibleItemList();

	void SendUpgradeItemResult(uint8 res, uint16 slot, _ITEM_TABLE* pTable);
	void SendItemModificationStatusMessage(uint8 action, uint16 status, uint8 showType = 1);
	//Doesn't show up in the chat log.
	void SendAddItemSilent(_ITEM_DATA* pItem, _ITEM_TABLE* pTable, uint16 slot);
	bool UpgradeItem(uint16 slot, uint8 upgCode);
	//bool HoleItem(uint16 slot, uint8 upgCode);
	void SendGoldUpdate();

	////////////////
	// Bank stuff //
	////////////////
	void SendOpenBank();
	void HandleBankGold(Packet& pkt);

	void OpenStrengtheningWindow();
	void OpenCompositionWindow();
	void OpenAdvancedFusionWindow();
	void OpenExtractionWindow();
	void OpenDismantleWindow();
	void RemoveBySlot(int slot);

	void HandleNpcExchange(Packet& pkt);
	void HandleItemModification(Packet& pkt);


private:
	//Returns new item
	_ITEM_DATA* AddItemToSlot(_ITEM_DATA* pNewItem, int16 slot);

public:


#pragma endregion InventoryHandler

#pragma region AttackHandler

	void HandleAttack(Packet& pkt);
	void HandleAoeAttack(Packet& pkt);

#pragma endregion AttackHandler

	void SendRegionCharacterUpdate();
	void SendUnk75();


	void SendNotice(std::string msg);
	
#pragma region Skillhandler

	void HandleSkillPackets(Packet& pkt);
	void HandleTimedCastSkillRequest(Packet& pkt);
	void HandleSkillBookPackets(Packet& pkt);
	bool RemoveSkillBook(uint32 skillBookId, uint8 slot);
	bool ChangeSkillPointOnSkill(uint8 bookSlot, uint8 skillSlot, int8 ptToAdd);
	bool AddSpecialPointToBook(uint8 bookSlot, uint8 specialPointType);

	bool AttemptCastSkillOnNpc(CNpc* pNpc, _PLAYER_SKILL_BOOK_DATA* pBookData, _PLAYER_SKILL_DATA* pSkillData);
	void FinishAttackRequest();
	CurrentAttackRequest m_currentAttackRequest;

	//Returns -1 if no skill was found.
	int16 GetSkillLevel(uint32 skillId);
	_PLAYER_SKILL_BOOK_DATA* GetSkillbookContainingSkill(uint32 skillId);

#pragma endregion SkillHandler

	CUser(uint16 socketID, SocketMgr* mgr);
	virtual ~CUser();
private:
	GameState m_state;
	bool m_inCombatState;
	bool m_isMeditating;

	ChatHandler* m_chatHandler;
};

//BASE STAT DEFINES, LATER ON WE MOVE THIS SOMEWHERE ELSE!
#endif USER_H