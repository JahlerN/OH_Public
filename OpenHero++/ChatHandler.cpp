#include "stdafx.h"
#include "Packets.h"

ChatHandler::ChatHandler()
{
}

ChatHandler::~ChatHandler()
{
}

void ChatHandler::HandleChat(Packet& pkt, CUser* pSender)
{
	uint8 subCode;
	pkt >> subCode;

	switch(subCode)
	{
	case MT_LOCAL:
		HandleSendLocalMessage(pkt, pSender);
		break;
	case MT_WHISPER:
		HandleSendWhisper(pkt, pSender);
		break;
	case MT_PARTY:
		HandleSendParty(pkt, pSender);
		break;
	case MT_CLAN:
		HandleSendClan(pkt, pSender);
		break;
	case MT_SHOUT:
		HandleShout(pkt, pSender);
		break;
	case MT_SLASH_COMMAND:
		g_commandHandler->HandleChatCommand(pkt, pSender);
		break;
	case MT_PLAYER_NOTICE:
		HandleSendPlayerNotice(pkt, pSender);
		break;
	case MT_FORCE_SHOUT_REQUEST:
		HandleRequestForceShout(pkt, pSender);
		break;
	case MT_VALOROUS:
		HandleSendValorous(pkt, pSender);
		break;
	}
}

void ChatHandler::HandleSendLocalMessage(Packet& pkt, CUser* pSender)
{
	std::string msg;
	pkt.DByte();
	pkt >> msg;

	if (msg.length() <= 0)
		return;

	Packet result(PKT_GAMESERVER_MESSAGE, uint8(MT_LOCAL));
	result << pSender->GetID();
	result << pSender->m_userData->m_charId;
	result.DByte();
	result << msg;

	//TODO: Should be sent to near regions
	//g_main->SendToRegion(result, pSender->GetMap(), pSender->GetRegionX(), pSender->GetRegionZ(), NULL);
	//SERVER_INSTANCE_CHANGE
	pSender->SendToRegion(result, NULL);
}

void ChatHandler::HandleSendWhisper(Packet & pkt, CUser * pSender)
{
	std::string user, msg;
	pkt >> user;
	pkt.DByte();
	pkt >> msg;

	if (msg.length() <= 0)
		return;

	CUser* pToUser = g_main->GetUserPtr(user.c_str(), NAME_TYPE_CHARACTER);

	//TODO: Send error message
	if (pToUser == NULL)
		return;

	Packet result(PKT_GAMESERVER_MESSAGE, uint8(MT_WHISPER));
	result << pSender->GetID();
	result << user;
	result.DByte();
	result << "-> " + user + " : " + msg;

	pToUser->Send(&result);
	pSender->Send(&result);
}

void ChatHandler::HandleSendParty(Packet & pkt, CUser * pSender)
{
}

void ChatHandler::HandleSendClan(Packet & pkt, CUser * pSender)
{
}

void ChatHandler::HandleShout(Packet& pkt, CUser* pSender)
{
	std::string msg;
	pkt.DByte();
	pkt >> msg;

	if (msg.length() <= 0)
		return;

	Packet result(PKT_GAMESERVER_MESSAGE, uint8(MT_SHOUT));
	result << pSender->GetID();
	result << pSender->m_userData->m_charId;
	result.DByte();
	result << msg;

	g_main->SendToAll(result);
}

void ChatHandler::HandleSendPlayerNotice(Packet & pkt, CUser * pSender)
{
}

void ChatHandler::HandleRequestForceShout(Packet & pkt, CUser * pSender)
{
}

void ChatHandler::HandleSendValorous(Packet & pkt, CUser * pSender)
{
	std::string msg;
	pkt.DByte();
	pkt >> msg;

	if (msg.length() <= 0)
		return;

	Packet result(PKT_GAMESERVER_MESSAGE, uint8(MT_VALOROUS));
	result << pSender->GetID();
	result << pSender->m_userData->m_charId;
	result.DByte();
	result << msg;

	g_main->SendToAll(result);
}
