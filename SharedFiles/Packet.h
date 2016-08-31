#ifndef PACKET_H
#define PACKET_H
#include "ByteBuffer.h"

class Packet : public ByteBuffer
{
public:
	__forceinline Packet() : ByteBuffer(), m_opcode(0) { }
	__forceinline Packet(uint8 opcode) : ByteBuffer(8192), m_opcode(opcode) {}
	__forceinline Packet(uint8 opcode, size_t res) : ByteBuffer(res), m_opcode(opcode) {}
	__forceinline Packet(const Packet& packet) : ByteBuffer(packet), m_opcode(packet.m_opcode) {}
	__forceinline Packet(uint8 opcode, uint8 subOpcode) : ByteBuffer(8192), m_opcode(opcode)
	{
		append(&subOpcode, 1);
	}

	__forceinline void Initialize(uint8 opcode)
	{
		Clear();
		m_opcode = opcode;
	}

	__forceinline void Initialize(uint8 opcode, uint8 subOpcode)
	{
		Clear();
		m_opcode = opcode;
		append(&subOpcode, 1);
	}

	__forceinline uint8 GetOpcode() const { return m_opcode; }
	__forceinline void SetOpcode(uint8 opcode) { m_opcode = opcode; }

protected:
	uint8 m_opcode;
};
#endif