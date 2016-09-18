#ifndef DEFINE_H
#define DEFINE_H
#define VIEW_DIST 32//48

struct _OBJECT_EVENT
{
	int m_belong;
	int m_index;
	int m_type;
	int m_controlNpcId;
	int m_status;
	float m_posX;
	float m_posZ;
	float m_posY;
};

struct _NPC_DATA
{
	uint32 m_npcId;
	std::string m_name;
	std::string m_phrase;
	uint8 m_npcType;
	uint32 m_level;
	uint32 m_exp;
	uint32 m_divineExp;
	uint32 m_darknessExp;
	uint8 m_lootRolls;
	uint32 m_dropId;
	uint32 m_gold;
	uint32 m_gossipId;
	uint32 m_maxHp;
	uint32 m_maxChi;
	//Is sent in packet to client, no clue what it does.
	uint16 m_unkUse;
	uint32 m_minDmg;
	uint32 m_maxDmg;
	uint32 m_minSkillDmg;
	uint32 m_maxSkillDmg;

	__forceinline bool IsMonster() { return m_gossipId == 0; }//{ return m_npcType == 0 || m_dropId == 0 || m_exp == 0 && m_level < 100 || m_divineExp == 0 && m_level < 200 && m_level > 100 || m_darknessExp == 0 && m_level > 200 ? false : true; }
};

struct _NPC_GROUP
{
	uint32 m_npcGroupId;
	uint8 m_zoneId;
	uint32 m_npcId;
	float m_rotation;
	float m_minX, m_minZ, m_maxX, m_maxZ;
	uint8 m_npcsInGroup;
	uint32 m_respawnTime;//In seconds.
};

#define MAX_SHOP_ITEM_TABLES 5

struct _SHOP_TABLE
{
	uint32 shopId;
	std::string shopName;
	uint32 shopItemTable[MAX_SHOP_ITEM_TABLES];
};

#define MAX_SHOP_TABLE_ITEMS 20

struct _SHOP_TABLE_ITEM
{
	uint32 shopItemId;
	uint32 itemIds[MAX_SHOP_TABLE_ITEMS];
	uint32 slot[MAX_SHOP_TABLE_ITEMS];
};

//Just a player or a GM?
enum UserAccess
{
	PLAYER = 0,
	GAME_ASSISTANT = 1,
	GAME_MASTER = 2,
	ADMINISTRATOR = 3,
	CONSOLE = 4,
};

//SkillId = yellow
enum DamageTextColors
{
	DTC_GRAY = 0xFFFFFFFE,
	DTC_PURPLE = 0xFFFFFFFD,
};
#endif