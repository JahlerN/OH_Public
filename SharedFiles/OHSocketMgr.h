#pragma once
#include <map>
#include "RWLock.h"

#include "SocketMgr.h"
#include "OHSocket.h"

typedef std::map<uint16, OHSocket*> SessionMap;

template <class T>
class OHSocketMgr : public SocketMgr
{
public:
	OHSocketMgr<T>() : m_server(NULL) {}

	virtual void InitSessions(uint16 sTotalSessions);
	virtual bool Listen(uint16 sPort, uint16 sTotalSessions);

	virtual void OnConnect(Socket* pSock);
	virtual Socket* AssignSocket(SOCKET socket);
	virtual void DisconnectCallback(Socket* pSock);

	void RunServer()
	{
		SpawnWorkerThreads();
		GetServer()->run();
	}

	void SuspendServer()
	{
		GetServer()->suspend();
	}

	void ResumeServer()
	{
		GetServer()->resume();
	}

	void SendAll(char* pBuff, int nLength)
	{
		AcquireLock();
		SessionMap& sessMap = m_activeSessions;
		for (auto itr = sessMap.Begin(); itr != sessMap.End(); ++itr)
			itr->second->Send(pBuff, nLength);
		ReleaseLock();
	}

	void SendAll(Packet* pkt)
	{
		AcquireLock();
		SessionMap& sessMap = m_activeSessions;
		for (auto itr = sessMap.Begin(); itr != sessMap.End(); ++itr)
			itr->second->Send(pkt);
		ReleaseLock();
	}

	void SendAllCompressed(char *send_buff, int len)
	{
		AcquireLock();
		SessionMap & sessMap = m_activeSessions;
		for (auto itr = sessMap.begin(); itr != sessMap.end(); ++itr)
			itr->second->SendCompressed(send_buff, len);
		ReleaseLock();
	}

	ListenSocket<T> * GetServer() { return m_server; }
	__forceinline SessionMap& GetIdleSessionMap()
	{
		AcquireLock();
		return m_idleSessions;
	}

	__forceinline SessionMap& GetActiveSessionMap()
	{
		AcquireLock();
		return m_activeSessions;
	}

	__forceinline void AcquireLock() { m_lock.AcquireReadLock(); }
	__forceinline void ReleaseLock() { m_lock.ReleaseReadLock(); }

	T* operator[] (uint16 id)
	{
		T* result = NULL;

		AcquireLock();
		auto itr = m_activeSessions.find(id);
		if (itr != m_activeSessions.end())
			result = static_cast<T*>(itr->second);
		ReleaseLock();

		return result;
	}

	virtual ~OHSocketMgr();

protected:
	SessionMap m_idleSessions, m_activeSessions;
	RWLock m_lock;

private:
	ListenSocket<T>* m_server;
};

template <class T>
void OHSocketMgr<T>::InitSessions(uint16 sTotalSessions)
{
	m_lock.AcquireWriteLock();
	for (uint16 i = 0; i < sTotalSessions; i++)
		m_idleSessions.insert(std::make_pair(i, new T(i, this)));
	m_lock.ReleaseWriteLock();
}

template <class T>
bool OHSocketMgr<T>::Listen(uint16 sPort, uint16 sTotalSessions)
{
	if (m_server != NULL)
		return false;

	CreateCompletionPort();
	m_server = new ListenSocket<T>(this, "0.0.0.0", sPort);
	if (!m_server->IsOpen())
		return false;

	InitSessions(sTotalSessions);
	return true;
}

template <class T>
Socket* OHSocketMgr<T>::AssignSocket(SOCKET socket)
{
	Socket* pSock = NULL;

	m_lock.AcquireWriteLock();
	auto itr = m_idleSessions.begin();
	if (itr != m_idleSessions.end())
	{
		m_activeSessions.insert(std::make_pair(itr->first, itr->second));
		pSock = itr->second;
		m_idleSessions.erase(itr);
		pSock->SetFd(socket);
	}
	m_lock.ReleaseWriteLock();
	return pSock;
}

template <class T>
void OHSocketMgr<T>::OnConnect(Socket* pSock)
{
	m_lock.AcquireWriteLock();
	auto itr = m_idleSessions.find(static_cast<OHSocket*>(pSock)->GetSocketID());
	if (itr != m_idleSessions.end())
	{
		m_activeSessions.insert(std::make_pair(itr->first, itr->second));
		m_idleSessions.erase(itr);
	}
	m_lock.ReleaseWriteLock();
}

template <class T>
void OHSocketMgr<T>::DisconnectCallback(Socket* pSock)
{
	m_lock.AcquireWriteLock();
	auto itr = m_activeSessions.find(static_cast<OHSocket*>(pSock)->GetSocketID());
	if (itr != m_activeSessions.end())
	{
		m_idleSessions.insert(std::make_pair(itr->first, itr->second));
		m_activeSessions.erase(itr);
	}
	m_lock.ReleaseWriteLock();
}

template <class T>
OHSocketMgr<T>::~OHSocketMgr()
{
	SessionMap killMap;
	m_lock.AcquireWriteLock();

	killMap = m_activeSessions;
	for (auto itr = killMap.begin(); itr != killMap.end(); ++itr)
		itr->second->Disconnect();

	for (auto itr = m_idleSessions.begin(); itr != m_idleSessions.end(); ++itr)
		itr->second->Disconnect();

	m_idleSessions.clear();
	m_lock.ReleaseWriteLock();

	if (m_server != NULL)
		delete m_server;
}