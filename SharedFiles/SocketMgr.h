#pragma once
#include "Socket.h"
#include "ListenSocket.h"
//#include "Mutex.h"
#include <queue>
#include <set>

class SocketMgr
{
public:
	SocketMgr();

	__forceinline HANDLE GetCompletionPort() { return m_completionPort; };
	__forceinline void SetCompletionPort(HANDLE cp) { m_completionPort = cp; };

	void CreateCompletionPort();
	void SpawnWorkerThreads();
	void ShutdownThreads();

	virtual Socket* AssignSocket(SOCKET socket) = 0;
	virtual void OnConnect(Socket* pSock) {};
	virtual void OnDisconnect(Socket* pSock)
	{
		s_disconnectionQueueLock.Acquire();
		s_disconnectionQueue.push(pSock);
		s_disconnectionQueueLock.Release();
	}
	virtual void DisconnectCallback(Socket* pSock) {}

	virtual ~SocketMgr();

	static FastMutex SocketMgr::s_disconnectionQueueLock;
	static std::queue<Socket*> SocketMgr::s_disconnectionQueue;

protected:
	HANDLE m_completionPort;
	HANDLE* m_threads;
	static HANDLE s_cleanupThread;

	long m_threadCount;

	static void SetupWinsock();
	static void CleanupWinsock();

	__forceinline void IncRef() { if (s_refs++ == 0) SetupWinsock(); }
	__forceinline void DecRef() { if (--s_refs == 0) CleanupWinsock(); }

	static UINT32 s_refs; //Reference counter, one app can hold multiple manager istances.
};

typedef void(*OperationHandler)(Socket* s, uint32 len);

DWORD WINAPI SocketWorkerThread(LPVOID lp);

void HandleReadComplete(Socket* s, uint32 len);
void HandleWriteComplete(Socket* s, uint32 len);
void HandleShutdown(Socket* s, uint32 len);

static OperationHandler ophandlers[] =
{
	&HandleReadComplete,
	&HandleWriteComplete,
	&HandleShutdown
};