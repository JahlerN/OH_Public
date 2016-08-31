#ifndef CHATHANDLER_H
#define CHATHANDLER_H

class ChatHandler
{
public:
	ChatHandler();
	~ChatHandler();

	void HandleChat(Packet& pkt, CUser* pSender);
	void HandleSendLocalMessage(Packet& pkt, CUser* pSender);
	void HandleSendWhisper(Packet& pkt, CUser* pSender);
	void HandleSendParty(Packet& pkt, CUser* pSender);
	void HandleSendClan(Packet& pkt, CUser* pSender);
	void HandleShout(Packet& pkt, CUser* pSender);
	void HandleSendPlayerNotice(Packet& pkt, CUser* pSender);
	void HandleRequestForceShout(Packet& pkt, CUser* pSender);
	void HandleSendValorous(Packet& pkt, CUser* pSender);
};
#endif