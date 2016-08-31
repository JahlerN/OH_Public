#pragma once
#include "..\SharedFiles\globals.h"

#define _LISTEN_PORT 15000
#define MAX_USERS 250

//struct _SERVER_INFO
//{
//	char strServerIP[32];
//	char strLanIP[32];
//	char strServerName[32];
//	uint8 userCount;
//	uint8 serverID;
//	uint8 playerCap;//CAP is 255 due to uint8, however, offical servers cap at 250. Use define MAX_USERS
//	uint8 serverStatus;
//
//	_SERVER_INFO() {
//		memset(strServerIP, 0x00, sizeof(strServerIP));
//		memset(strServerName, 0x00, sizeof(strServerName));
//
//		userCount = serverID = playerCap = serverStatus = 0;
//	}
//};