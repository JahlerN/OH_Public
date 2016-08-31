#include "stdafx.h"
//#include "OHSocket.h"
//#include "crc32.h"
#include "lzf.h"
//#include "version.h"
//#include "packets.h"
#include <math.h>

OHSocket::OHSocket(uint16 socketID, SocketMgr* mgr, SOCKET fd, uint32 sendBufferSize, uint32 recvBufferSize)
	: Socket(fd, sendBufferSize, recvBufferSize), m_socketID(socketID), m_remaining(0), m_usingCrypto(false),
	m_readTries(0), m_sequence(0), m_lastResponse(0)
{
	SetSocketMgr(mgr);
}

void OHSocket::OnConnect()
{
	TRACE("Connection recieved from %s:%d\n", GetRemoteIP().c_str(), GetRemotePort());
}

void OHSocket::OnDisconnect()
{
	TRACE("Connection closed from %s:%d\n", GetRemoteIP().c_str(), GetRemotePort());
}

void OHSocket::OnRead() 
{
	Packet pkt;

	for (;;)
	{
		if (m_remaining == 0)
		{
			if (GetReadBuffer().GetSize() < 7) //TODO: I think we can accept > 7 now, i changed the packet maker down there to accept uint8 opcodes
			{
				GetReadBuffer().Remove(GetReadBuffer().GetSize());
				return;
			}

			uint16 header = 0;
			GetReadBuffer().Read(&header, 2);
			if (header != 0x55AA)
			{
				TRACE("%s: Packet with invalid header recieved. Should be 0x55AA, but got 0x%X instead.\n", GetRemoteIP().c_str(), header);
				goto error_handler;
			}

			GetReadBuffer().Read(&m_remaining, 2);
			if (m_remaining == 0)
			{
				TRACE("%s: Pack without opcode recieved, this should never happen.\n", GetRemoteIP().c_str());
				goto error_handler;
			}
		}

		if (m_remaining > GetReadBuffer().GetAllocatedSize())
		{
			TRACE("%s: Packet exceeded maximum size: %u, max size is %u.\n", GetRemoteIP().c_str(), m_remaining, GetReadBuffer().GetAllocatedSize());
			goto error_handler;
		}

		if (m_remaining > GetReadBuffer().GetSize())
		{
			if (m_readTries > 4)
			{
				TRACE("%s: Packet fragmentation count has exceeded 4, disconnecting, i can smell ill intent.\n", GetRemoteIP().c_str());
				goto error_handler;
			}
			m_readTries++;
			return;
		}

		//TODO: If m_remaining is 1 only, make a packet manually here and set the opcode to the remaining one, skip the in stream stuff and decrypting, just run handle packet.
		uint16 len = m_remaining;
		uint8* in_stream = new uint8[m_remaining];

		m_readTries = 0;
		GetReadBuffer().Read(in_stream, m_remaining);

		uint16 tail = 0;
		GetReadBuffer().Read(&tail, 2);
		//printf("\nRecieved: %04X %04X ", 0xAA55, len);
		//for (uint32 i = 0; i < len; i++)
		//{
		//	printf("%02X ", in_stream[i]);
		//}
		//printf("%04X \n", 0x55AA);

		if (tail != 0xAA55 || !DecryptPacket(in_stream, pkt))
		{
			TRACE("%s: Invalid footer should be 0xAA55, was 0x%X or decryption failed.\n", GetRemoteIP().c_str(), tail);
			goto error_handler;
		}

		if (!HandlePacket(pkt))
		{
			TRACE("%s: Packet handler for opcode 0x%X returned false\n.", GetRemoteIP().c_str(), pkt.GetOpcode());
#ifndef _DEGUG
			goto error_handler;
#endif
		}
		m_remaining = 0;
	}
	return;

error_handler:
	GetReadBuffer().Remove(GetReadBuffer().GetSize());
	Disconnect();
}

bool OHSocket::DecryptPacket(uint8* in_stream, Packet& pkt)
{
	uint8* final_packet = NULL;
	//0x02 means that the packet is encrypted, 1,2 is the encrypted length
	if (isCryptoEnabled() && in_stream[0] == 0x02)// || ((in_stream[0] << 8) + in_stream[1]) >= 0x0211 && ((in_stream[0] << 8) + in_stream[1]) <= 0x0220)
	{
		//printf("\n");
		//for (uint16 i = 0; i < in_stream[1]; i++)
		//{
		//	printf("%02x ", in_stream[i]);
		//}
		//printf("\n");

		//uint8* temp = new uint8[in_stream[1]];//TODO: BEFORE I SEND, TRY IT WITH THE CRC IN FRONT.
		//uint32 multi = 256;
		//m_crypto.p_key = 0xD80EEBCCCCEB0CF0;//0xAC7A9FB8B89F7884; //Should probably do the thing when i made one num correct, change the next one via the 256*time etc 
		//uint8* packet = new uint8[0x13] { 0x01, 0x03, 0x00, 0x04, 0x61, 0x62, 0x63, 0x64, 0x36, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 };
		//uint32 expectedCrc = crc32(packet, 0x13);
		//printf("\n%08lx \n", expectedCrc);//allowed names seem to be 4+ 32 or less Key 4char 0x64b257707057b04c key 3char 0xAC7A9FB8B89F7884, proably actual key D70EEBCCCCEB0CF0
		//m_crypto.Init();

		//JvDecryptionFast(in_stream[1], &in_stream[3], temp);
		if (in_stream[1] < 4 || m_crypto.JvDecryptionWithCRC32(in_stream[1], &in_stream[3], in_stream) < 0)// || ++m_sequence != *(uint32*)(in_stream))
		{
			return false;
		//while (true)
		//{
		//	m_crypto.p_key = 552;
		//	m_crypto.Init();
				/*uint32 crc = crc32(temp, 18);

				printf("\n %08lx \n", crc);
				for (uint16 i = 0; i < in_stream[1]; i++)
				{
					printf("%02x ", temp[i]);
				}
				printf("\n");
				if (temp[0] == 0x01) {}
				else
				{
				m_crypto.p_key++;
			}
			if (temp[1] == 0x03) {}
			else
			{
				m_crypto.p_key += 256;
			}
			if (temp[2] == 0x00) {}
			else
			{
				m_crypto.p_key += pow(256, 2);
			}
			if (temp[3] == 0x04) {}
			else
			{
				m_crypto.p_key += pow(256, 3);
			}
			if (temp[4] == 0x61) {}
			else
			{
				m_crypto.p_key += pow(256, 4);
			}
			if (temp[5] == 0x62) {}
			else
			{
				m_crypto.p_key += pow(256, 5);
			}
			if (temp[6] == 0x63) {}
			else
			{
				m_crypto.p_key += pow(256, 6);
			}
			if (temp[7] == 0x64) {}
			else
			{
				m_crypto.p_key += 72057594037927936;
			}*/
			/*if (temp[10] == 0x00) {}
			else
			{
				m_crypto.p_key += pow(256, 2);
			}
			if (temp[11] == 0x00) {}
			else
			{
				m_crypto.p_key += pow(256, 3);
			}
			if (temp[12] == 0x00) {}
			else
			{
				m_crypto.p_key += pow(256, 4);
			}
			if (temp[13] == 0x00) {}
			else
			{
				m_crypto.p_key += pow(256, 5);
			}
			if (temp[14] == 0x00) {}
			else
			{
				m_crypto.p_key += pow(256, 6);
			}
			if (temp[15] == 0x00) {}
			else
			{
				m_crypto.p_key += 72057594037927936;
			}
			if (temp[16] == 0x00) {}
			else
			{
				m_crypto.p_key++;
			}
			if (temp[17] == 0x00)
			{
			}
			else
			{
				m_crypto.p_key += 256;
			}
			if (temp[18] == 0x00) {}
			else
			{
				m_crypto.p_key += pow(256, 2);
			}*/
			//if (temp[11] == 0x04) {}
			//else
			//{
			//	m_crypto.p_key += pow(256, 3);
			//	continue;
			//}
			//if (temp[12] == 0x61) {}
			//else
			//{
			//	m_crypto.p_key += pow(256, 4);
			//	continue;
			//}
			//if (temp[13] == 0x62) {}
			//else
			//{
			//	m_crypto.p_key += pow(256, 5);
			//	continue;
			//}
			//if (temp[14] == 0x63) {}
			//else
			//{
			//	m_crypto.p_key += pow(256, 6);
			//	continue;
			//}
			//if (temp[15] == 0x64) {}
			//else
			//{
			//	m_crypto.p_key += 72057594037927936;
			//	continue;
			//}
			//if (temp[0] == 0x01)
			//	if (temp[1] == 0x03)
			//		if (temp[2] == 0x00)
			//			if (temp[3] == 0x04)
			//				if (temp[4] == 0x61)
			//					if (temp[5] == 0x62)
			//						if (temp[6] == 0x63)
			//							if (temp[7] == 0x64)
			//								if (temp[8] == 0x35)
			//									if (temp[9] == 0x01)
			//									//continue;
			//										if (temp[18] == 0x74)
			//											if (temp[19] == 0x46)
			//												if (temp[20] == 0x9F)
			//													if (temp[21] == 0x25)
			//														continue;
			//													else
			//														m_crypto.p_key += pow(256, 6);
			//												else
			//													m_crypto.p_key += pow(256, 5);
			//											else
			//												m_crypto.p_key += pow(256, 4);
			//										else
			//											m_crypto.p_key += pow(256, 3);
			//									else m_crypto.p_key += 256;
			//								else
			//									m_crypto.p_key++;
			//							else
			//								m_crypto.p_key += 72057594037927936;
			//						else
			//							m_crypto.p_key += pow(256, 6);
			//					else
			//						m_crypto.p_key += pow(256, 5);
			//				else
			//					m_crypto.p_key += pow(256, 4);
			//			else
			//				m_crypto.p_key += pow(256, 3);
			//		else
			//			m_crypto.p_key += pow(256, 2);
			//	else
			//		m_crypto.p_key += 256;
			//else
			//	m_crypto.p_key++;
		}
		m_remaining -= 7;
		final_packet = in_stream;
		printf("\nDecrypted: ");
		for (int i = 0; i < m_remaining; i++)
		{
			printf("%02X ", final_packet[i]);
		}
		printf("\n");
	}
	else
	{
		final_packet = in_stream;
	}

	//if (m_remaining > 1)
	//{
	//	m_remaining -= 2;
	//	pkt = Packet((final_packet[0] << 8) + final_packet[1], (size_t)m_remaining); //TODO: Changed to add final_packet[1] aswell, because it's a 16bit value opcode now.
	//}
	//else
	//{
		m_remaining--;
		pkt = Packet(final_packet[0], (size_t)m_remaining);
	//}

	if (m_remaining > 0)
	{
		pkt.resize(m_remaining);
		memcpy((void*)pkt.contents(), &final_packet[1], m_remaining);
	}
	
	return true;
}

bool OHSocket::Send(char* buff, int len)
{
	Packet result(*buff, (size_t)len - 1);
	if (len > 0)
		result.append(buff + 1, len - 1);

	return Send(&result);
}

bool OHSocket::SendCompressed(char* buff, int len)
{
	Packet result(*buff);
	if (len > 1)
		result.append(buff + 1, len - 1);

	return SendCompressed(&result);
}

bool OHSocket::Send(Packet* pkt)
{
	if (!IsConnected() || pkt->size() + 1 > GetWriteBuffer().GetAllocatedSize())
		return false;

	bool r;

	uint8 opcode = pkt->GetOpcode();
	uint8* out_stream = NULL;
	uint16 len = (uint16)(pkt->size() + 1);

	/*if (isCryptoEnabled())
	{
		len += 5;

		out_stream = new uint8[len];
		*(uint16*)&out_stream[0] = 0x1efc;
		*(uint16*)&out_stream[0] = (uint16)(m_sequence);
		out_stream[4] = 0;
		uint16 val = pkt->GetOpcode();
		out_stream[5] = (val >> 8);
		out_stream[6] = val & 0xFF;
		if (pkt->size() > 0)
			memcpy(&out_stream[7], pkt->contents(), pkt->size());

		m_crypto.JvDecryptionFast(len, out_stream, out_stream);
	}
	else*/
	{
		out_stream = new uint8[len];
		out_stream[0] = pkt->GetOpcode();
		//uint16 val = pkt->GetOpcode();
		//out_stream[0] = (val >> 8);// I've also reversed this, because that's the way hero likes it(I think it's actually a subopcode, 2 uint8).
		//out_stream[1] = val & 0xFF;
		if (pkt->size() > 0)
			memcpy(&out_stream[1], pkt->contents(), pkt->size());
	}

	BurstBegin();

	if (GetWriteBuffer().GetSpace() < size_t(len + 6))
	{
		BurstEnd();
		Disconnect();
		return false;
	}

	r = BurstSend((const uint8*)"\xAA\x55", 2);
	if (r) r = BurstSend((const uint8*)&len, 2);
	if (r) r = BurstSend((const uint8*)out_stream, len);
	if (r) r = BurstSend((const uint8*)"\x55\xAA", 2);
	if (r) BurstPush();
	BurstEnd();

	delete[] out_stream;
	return r;
}

bool OHSocket::SendCompressed(Packet* pkt)
{
	uint32 crc;
	uint16 inLength = pkt->size() + 3, outLength = inLength + LZF_MARGIN;
	uint8* buffer = new uint8[inLength], *outBuffer = new uint8[outLength];

	//*buffer = pkt->GetOpcode();
	/*size_t siz = pkt->size();
	char size;
	*buffer = siz & 0xFF;
	size = siz >> 8;
	memcpy(buffer + 1, &size, 1);
	size = pkt->GetOpcode();
	memcpy(buffer + 2, &size, 1);*/
	uint16 size = pkt->size() + 1;
	buffer[0] = size & 0xFF;
	buffer[1] = size >> 8;
	buffer[2] = pkt->GetOpcode();
	if (pkt->size() > 0)
		memcpy(buffer + 3, pkt->contents(), pkt->size());
	//uint16 val = pkt->GetOpcode();//TODO: Idk if this will be correct.
	//*buffer = (val >> 8);
	//*buffer = val & 0xFF;
	//if (pkt->size() > 0)
	//	memcpy(buffer + 1, pkt->contents(), pkt->size());

	crc = (uint32)crc32(buffer, inLength);
	outLength = lzf_compress(buffer, inLength, outBuffer, outLength);

	pkt->Initialize(0x17);
	*pkt << outLength << inLength;

	//*pkt << uint32(crc);

	pkt->append(outBuffer, outLength);
	
	delete[] buffer;
	delete[] outBuffer;

	return Send(pkt);
}

void OHSocket::EnableCrypto()
{
	m_crypto.Init();
	m_usingCrypto = true;
}