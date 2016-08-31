#pragma once
#include <iostream>
#define GAMESERVER_VERSION 0.10f
#define LOGINSERVER_VERSION 0.10f
#define MAX_ID_SIZE 20

#define NPC_START 20001

#define MAX_INVENTORY_SLOTS 56
#define MAX_WAREHOUSE_SLOTS 120 //*2 for premium slots /2 for 1 page

#define MAX_ITEM_STACK 999
#define MAX_GOLD 9999999999999

#define MAX_PLAYER_LEVEL 250

enum ServerStatus
{
	SERVER_STATUS_MAINTENANCE = 0,
	SERVER_STATUS_OPEN = 1,
};

struct _SERVER_INFO
{
	uint8 m_tabId;
	uint8 m_serverId;
	std::string m_serverName;
	uint16 m_curPlayers;
	uint16 m_maxPlayers;
	ServerStatus m_serverStatus;
};

struct _SERVER_TAB
{
	uint8 m_tabId;
	std::string m_tabName;
	std::string m_serverIp;
	uint16 m_port;
	std::vector<_SERVER_INFO*> m_serverInfoArr;
};

enum PlayerStats
{
	STAT_STR,
	STAT_DEX,
	STAT_INT,
	STAT_WIND,
	STAT_WATER,
	STAT_FIRE,
	STAT_END
};

enum ItemSlot
{
	IS_HELM = 0x00,
	IS_MASK = 0x01,
	IS_ARMOR = 0x02,
	IS_WEAPON1 = 0x03,
	IS_WEAPON2 = 0x04,
	IS_EARRING = 0x05,
	IS_AMULET = 0x06,
	IS_BRACELET = 0x07,
	IS_RING = 0x08,
	IS_BOOTS = 0x09,
	IS_PET = 0x0a,
	IS_CSHELM = 307,
	IS_CSMASK = 308,
	IS_CSARMOR = 309,
	IS_CSBOOTS = 310,
	IS_CLANARMOR = 311,
	IS_STONE = 312,
	IS_CHARM = 313,
	IS_BALL = 314,
	IS_TABLET = 315,
	IS_PETHELM = 317,
	IS_PETARMOR = 318,
	IS_PETBOOTS = 319,
	IS_TREEMARBLE2 = 397,//0x018D
	IS_TREEMARBLE = 398,
	IS_EARTHMARBLE = 399,
	IS_METALMARBLE = 400,
	IS_WATERMARBLE = 401,
	IS_END
};

enum ShowHtItem
{
	SHOW_HT_HELM = 0x01,
	SHOW_HT_MASK = 0x02,
	SHOW_HT_ARMOR = 0x04,
	SHOW_HT_CLANARMOR = 0x08,
	SHOW_HT_BOOTS = 0x10
};

enum WeaponTypes
{
	WT_MONK_TYPE = 52,
	WT_BOW = 53,
	WT_THROWING = 54,
	WT_SWORD_BLADE = 55,
	WT_AXE = 56,
	WT_GAUNTLET = 57,
	WT_SPEAR_ROD = 58,
	WT_DUAL_SWORD_ROD = 59
};

//TODO: Make defines for inv slots in this order!
#define EQUIP_SLOT 11
#define CASHSHOP_EQUIP_SLOT 5
#define SECONDARY_ACC_SLOT 4
#define PET_ARMOR_SLOT 3
#define FIVE_ELEMENT_SLOT 5
#define MAX_EQUIP_SLOTS EQUIP_SLOT + CASHSHOP_EQUIP_SLOT + SECONDARY_ACC_SLOT + PET_ARMOR_SLOT// + FIVE_ELEMENT_SLOT TODO: Implement five elements
#define INV_1_START 0x0B
#define INV_1_END 0x42 
#define CASHSHOP_EQUIP_START 0x0133
#define SECONDARY_ACC_START 0x0138
#define PET_ARMOR_START 0x013D
#define INV_2_START 0x0155
#define INV_2_END 0x018C
#define FIVE_ELEMENT_START 0x018D

#define MAX_COMBINED_INV_SLOTS (IS_END - 1) * 38// 38 is the combined bytes for each item. id+count+upgscount+upgs+holescount+holes

#define MAX_ITEM_UPGRADE_AMOUNT 15

enum ItemFlags
{
	SEALED = 0x01,
};

struct _ITEM_DATA
{
	uint32 itemId;
	uint16 count;
	uint8 flag;//??
	uint16 remainingRentalTime;
	uint32 expirationTime;
	uint8 upgradeCount;
	uint8 upgrades[15];
	uint8 holeCount;
	uint8 holes[15];

	uint8 GetUpgradeCount()
	{
		return upgradeCount;
	}

	uint8 GetHoleCount() { return holeCount; }

	bool IsLocked() { return flag == SEALED; }

	void UpgradeItem(uint8 upgCode) 
	{ 
		//No item, can't upg that..
		if (itemId == 0 || count == 0)
			return;

		if (upgradeCount == MAX_ITEM_UPGRADE_AMOUNT)
			return; //Max upgrade count;
		upgrades[upgradeCount++] = upgCode;
	}

	void HoleItem(uint8 hole)
	{
		//No item, can't upg that..
		if (itemId == 0 || count == 0)
			return;

		if (upgradeCount == MAX_ITEM_UPGRADE_AMOUNT - 1)
			return; //Max holes, as of now(might not even be intended for holes.)
		holes[holeCount++] = hole;
	}

	_ITEM_DATA()
	{
		Initialize();
	}

	void Initialize()
	{
		itemId = 0;
		count = 0; 
		flag = 0;
		remainingRentalTime = 0;
		expirationTime = 0;
		upgradeCount = 0;
		holeCount = 0;
		memset(&upgrades, 0x00, sizeof(upgrades));
		memset(&holes, 0x00, sizeof(holes));
	}
};

struct _LEVEL_DATA
{
	uint32 m_level;
	uint64 m_expReq;
	uint16 m_statPoint;
	uint16 m_statElement;
	uint8 m_unk2;
	uint8 m_unk3;
};

enum NameTypes
{
	NAME_TYPE_ACCOUNT,
	NAME_TYPE_CHARACTER
};

__forceinline float GetFloatTime()
{
	static bool init = false;
	static LARGE_INTEGER nTime, nFrequency;
	
	if (!init)
	{
		if (TRUE == ::QueryPerformanceCounter(&nTime))
		{
			::QueryPerformanceFrequency(&nFrequency);
		}
		else
			ASSERT(0);
		init = true;
	}

	::QueryPerformanceCounter(&nTime);

	return (float)((double)(nTime.QuadPart) / (double)nFrequency.QuadPart);
}

__forceinline bool HasSubString(std::string& fullStr, std::string& subStr)
{
	if (fullStr.find(subStr) != std::string::npos)
		return true;
	return false;
}

__forceinline void STRTOUPPER(std::string& str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = (char)toupper(str[i]);
}

__forceinline void STRTOLOWER(std::string& str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = (char)tolower(str[i]);
}

__forceinline void UINT64HEXREVERSE(uint64& val)
{
	val = htonll(val);
}

__forceinline uint8 charToHex(unsigned char val)
{
	if (val >= '0' && val <= '9')
		return val - '0';
	if (val >= 'A' && val <= 'F')
		return val - 'A' + 10;
	if (val >= 'a' && val <= 'f')
		return val - 'a' + 10;
	throw std::invalid_argument("Invalid input string");
}

__forceinline void HEXSTRTOBYTEARR(const char* hex, uint8* arr, int len)
{
	for (int i = 0; i < len / 2; i++)
	{
		*(arr + i) = charToHex(*hex) << 4 | charToHex(*(hex + 1));//Test this
		hex += 2;
	}
	/*while (*hex && hex[1])
	{
		*(arr++) = char2int(*hex) * 16 + char2int(hex[1]);
		hex += 2;
	}*/
}

__forceinline uint8* HEXSTRTOBYTEARR2(const char* hex, int& len)
{
	std::string s = std::string(hex);
	len = s.length() / 2;
	uint8* arr = new uint8[len];
	for (int i = 0; i < len; i++)
	{
		*(arr + i) = charToHex(*hex) << 4 | charToHex(*(hex + 1));//Test this
		hex += 2;
	}

	return arr;
	/*while (*hex && hex[1])
	{
	*(arr++) = char2int(*hex) * 16 + char2int(hex[1]);
	hex += 2;
	}*/
}