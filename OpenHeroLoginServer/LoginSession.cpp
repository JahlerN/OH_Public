#include "stdafx.h"
//#include <boost\regex.hpp>

LSPacketHandler PacketHandlers[NUM_LS_OPCODES];
void InitPacketHandlers(void)
{
	memset(&PacketHandlers, 0, sizeof(LSPacketHandler) * NUM_LS_OPCODES);
	PacketHandlers[LOGIN_REQUEST]				= &LoginSession::HandleLogin;
	PacketHandlers[LOGIN_SERVERLIST_REQUEST]	= &LoginSession::HandleServerList;
	PacketHandlers[LOGIN_SELECTSERVER_REQUEST]	= &LoginSession::HandleSelectServer;
	PacketHandlers[LOGIN_CONNECTED]				= &LoginSession::HandleConnected;
}

LoginSession::LoginSession(uint16 socketID, SocketMgr* mgr) : OHSocket(socketID, mgr, -1, 2048, 2048) 
{
	m_serverId = -1;
	m_serverTab = -1;
}

bool LoginSession::HandlePacket(Packet& pkt)
{

	if (pkt.GetOpcode() == LOGIN_CONNECTED)
		return true;
	else if (pkt.GetOpcode() != PKT_LOGIN_SERVER)
		return false;

	uint8 opcode;// = pkt.GetOpcode();
	pkt >> opcode;
	if (opcode > NUM_LS_OPCODES || PacketHandlers[opcode] == NULL)
		return false;

	(this->*PacketHandlers[opcode])(pkt);
	return true;
}

void LoginSession::HandleLogin(Packet& pkt)
{
	enum LoginErrorCodes
	{
		LOGIN_ERR_OK = 0x01,
		LOGIN_ERR_INCORRECTLOGININFO = 0x02,
		LOGIN_ERR_BANNED = 0x03,
		LOGIN_ERR_FAILED = 0x04,
		LOGIN_ERR_NEWACC_OK = 0x05,
		LOGIN_ERR_NEWACC_NOTOK = 0x06
	};

	//For newer versions they changed the structure abit, there's now one empty byte in the front
	Packet result(PKT_LOGIN_SERVER, uint8(LOGIN_ERR_OK));
	uint16 resultCode;
	string acc, pw;

	pkt >> acc >> pw;
	if (acc.size() == 0 || pw.size() == 0 || pw.size() > 64) //Max name len 34 is only if there is _n for new acc.
		resultCode = LOGIN_ERR_FAILED;
	else
	{
	//	boost::regex regex("\w+");
		//if (!boost::regex_match(acc.begin(), acc.end(), regex))
		//	resultCode = LOGIN_ERR_FAILED;
		if (acc.size() <= 34 && acc.substr(acc.length() - 2) == "_n")
			resultCode = g_main->m_DBProcess.AccountCreate(acc, pw);
		else
			resultCode = g_main->m_DBProcess.AccountLogin(acc, pw);
	}

	if (resultCode == LOGIN_ERR_OK)
	{
		result << int8(LOGIN_ERR_OK);
		result << acc;
		result << pw;
		result << uint8(1);//End byte? idk
		m_accountId = acc;
	}
	else if (resultCode == LOGIN_ERR_INCORRECTLOGININFO)
	{
		result << int8(0);//Not ok
		string errCode = "Incorrect login information.";
		result << string(errCode);
		result << uint8(0);//End byte? idk
	}
	else if (resultCode == LOGIN_ERR_NEWACC_OK)
	{
		result << int8(0);//Not ok.
		string errCode = "Account created, you might have to restart the client to login.";
		result << string(errCode);
		result << uint8(0);
	}
	else if (resultCode == LOGIN_ERR_NEWACC_NOTOK)
	{
		result << int8(0);//Not ok
		string errCode = "Account name already exists.";
		result << string(errCode);
		result << uint8(0);
	}
	Send(&result);
}

void LoginSession::HandleServerList(Packet& pkt)
{
	if (m_accountId.empty())
	{
		Disconnect();
		return;
	}

	Packet result(PKT_LOGIN_SERVER, uint8(LOGIN_SERVERLIST_RESPONSE));

	ServerInfoList* pServerInfoList = g_main->GetServerList();

	result << uint8(pServerInfoList->size());//Num tabs
	result << uint8(0);//Tab index? 
	for (int i = 0; i < 2; i++)//Max tabs is 2, crate definition
		if (pServerInfoList->size() > i)
			result << uint8(pServerInfoList->at(i)->m_tabId) << uint8(0);//Tab index, unk
		else
			result << uint8(0) << uint8(0);
	for (int i = 0; i < pServerInfoList->size(); i++)
		result << pServerInfoList->at(i)->m_tabName;
	for (auto itr = pServerInfoList->begin(); itr != pServerInfoList->end(); itr++)
	{
		result << uint8((*itr)->m_serverInfoArr.size()) << uint8(0);
		std::ostringstream ss;
		foreach(server, (*itr)->m_serverInfoArr)
		{
			_SERVER_INFO* pServer = (*server);
			result << pServer->m_serverId
				<< uint8(0)//Unk
				<< pServer->m_serverName
				<< pServer->m_curPlayers
				<< pServer->m_maxPlayers
				<< uint32(0x12000000)
				<< uint8(pServer->m_serverStatus)
				<< uint8(0);//Unk
		}
	}
	Send(&result);
}

void LoginSession::HandleSelectServer(Packet& pkt)
{
	Packet result(PKT_LOGIN_SERVER, uint8(LOGIN_SELECTSERVER_RESPONSE));

	result << uint8(1);//If this is 0 it shows the IP adress in a popup window instead.
	uint16 tabId, serverId;
	pkt >> tabId >> serverId;
	bool foundServer = false;
	if (tabId < g_main->GetServerList()->size());
	{
		_SERVER_TAB* pTab = g_main->GetServerList()->at(tabId);

		if (serverId < pTab->m_serverInfoArr.size())
		{
			_SERVER_INFO* pInfo = pTab->m_serverInfoArr.at(serverId);
			if (pInfo->m_curPlayers < pInfo->m_maxPlayers &&
				pInfo->m_serverStatus == SERVER_STATUS_OPEN)
			{
				result << pTab->m_serverIp;
				result << pTab->m_port;
				m_serverTab = tabId;
				m_serverId = serverId;
			}
		}
	}

	if (m_serverTab < 0 || m_serverId < 0)
	{
		Disconnect();
		return;
	}
	result << uint16(0x0000);

	g_main->m_DBProcess.UpdateAccountSession(this);

	Send(&result);
}

void LoginSession::HandleConnected(Packet& pkt)
{
	//Ignore for now
}