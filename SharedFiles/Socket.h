#pragma once
#include "SocketDefines.h"
#include "CircularBuffer.h"
#include "Mutex.h"
#include <string>

class SocketMgr;
class Socket
{
public:
	Socket(SOCKET fd, uint32 sendBufferSize, uint32 recvBufferSize);

	bool Connect(const char* address, uint32 port);
	void Disconnect();

	void SetBlocking(bool block = false);
	void SetBuffering(bool enable = true);

	void Accept(sockaddr_in* adress);

	virtual void OnRead() { }

	virtual void OnConnect() { }

	virtual void OnDisconnect() { }

	__forceinline void BurstBegin() { m_writeMutex.Acquire(); }
	bool BurstSend(const uint8* bytes, uint32 size);
	void BurstPush();
	__forceinline void BurstEnd() { m_writeMutex.Release(); }

	bool Send(const uint8* bytes, uint32 size);

	std::string GetRemoteIP();
	__forceinline sockaddr_in& GetRemoteStruct() { return m_client; }
	__forceinline in_addr GetRemoteAddress() { return m_client.sin_addr; }
	__forceinline uint32 GetRemotePort() { return ntohs(m_client.sin_port); }
	__forceinline SOCKET GetFd() { return m_fd; }
	__forceinline SocketMgr* GetSocketMgr() { return m_socketMgr; }

	void SetupReadEvent();
	void ReadCallback(uint32 len);
	void WriteCallback();

	__forceinline bool IsDeleted() { return m_deleted; }
	__forceinline bool IsConnected() { return m_connected; }
	__forceinline CircularBuffer& GetReadBuffer() { return m_readBuffer; }
	__forceinline CircularBuffer& GetWriteBuffer() { return m_writeBuffer; }

	__forceinline void SetFd(SOCKET fd) { m_fd = fd; }
	__forceinline void SetSocketMgr(SocketMgr *mgr) { m_socketMgr = mgr; }

	void Delete();

	virtual ~Socket();

protected:
	void _OnConnect();

	SOCKET m_fd;
	CircularBuffer m_readBuffer, m_writeBuffer;
	Mutex m_writeMutex, m_readMutex;
	bool m_connected;
	bool m_deleted;
	sockaddr_in m_client;
	SocketMgr* m_socketMgr;

public:
	// Set completion port that this socket will be assigned to.
	__forceinline void SetCompletionPort(HANDLE cp) { m_completionPort = cp; }

	// Atomic wrapper functions for increasing read/write locks
	__forceinline void IncSendLock() { InterlockedIncrement(&m_writeLock); }
	__forceinline void DecSendLock() { InterlockedDecrement(&m_writeLock); }
	__forceinline bool AcquireSendLock()
	{
		if (m_writeLock)
			return false;

		IncSendLock();
		return true;
	}

	OverlappedStruct m_readEvent, m_writeEvent;
private:
	// Completion port socket is assigned to
	HANDLE m_completionPort;

	// Write lock, stops multiple write events from being posted.
	volatile long m_writeLock;

	// Assigns the socket to his completion port.
	void AssignToCompletionPort();

};