#pragma once
#include "stdafx.h"

class LoginSession : public OHSocket
{
public:
	LoginSession(uint16 socketID, SocketMgr* mgr);

	virtual bool HandlePacket(Packet& pkt);
	void HandleLogin(Packet& pkt);
	void HandleServerList(Packet& pkt);
	void HandleSelectServer(Packet& pkt);
	void HandleConnected(Packet& pkt);
	//void HandleChangeServer(Packet& pkt);//TODO: If we want to destroy player connection etc when he changes to the game server.

	std::string m_accountId;
	int8 m_serverTab;
	int8 m_serverId;
};

#define PKT_LOGIN_SERVER 0x00

enum LoginOpcodes
{
	LOGIN_REQUEST = 0x00,
	LOGIN_RESPONSE = 0x01,
	LOGIN_SERVERLIST_REQUEST = 0x02,
	LOGIN_SERVERLIST_RESPONSE = 0x03,
	LOGIN_SELECTSERVER_REQUEST = 0x04,
	LOGIN_SELECTSERVER_RESPONSE = 0x05,
	LOGIN_CONNECTED = 0x38, //TODO: Idk what this actually wants, just sends 38, nothing else.
	LOGIN_REQUEST_CHANGINGSERVER = 0x56, //IDK WHAT THIS DOES ACTUALLY ))

	NUM_LS_OPCODES
};

void InitPacketHandlers(void);
typedef void(LoginSession::*LSPacketHandler)(Packet&);