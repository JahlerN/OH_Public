#ifndef NPCTHREAD_H
#define NPCTHREAD_H
#include <thread>
#include "MapInstance.h"

//#define NPCS_PER_THREAD 2000//250
#define THREADS_PER_SERVER 2//8

class CNpc;

class CNpcThread
{
public:
	uint32 m_threadNum;
	std::thread m_thread;
	std::list<CNpc*> m_npcList;
	std::list<MapInstance*> m_mapInstanceList;
	bool m_running = false, m_exit = false;//TODO: Move these to the mgr?
};

void NpcThreadProc(CNpcThread* pThread);

class CNpcThreadMgr
{
public:
	CNpcThreadMgr();
	~CNpcThreadMgr();

	//uint32 CreateAndAssignNpcThreads(std::list<CNpc*>* npcList);
	uint32 CreateAndAssignNpcThreads(std::map<uint8, MapInstance*> mapList);

	std::vector<CNpcThread*> m_threadStorage;
	uint32 m_threadCount;
	uint32 m_npcCounter;

	uint32 GetTotalNpcCount() { return m_npcCounter; }

	void PauseAi()
	{
		foreach(itr, m_threadStorage)
		{
			(*itr)->m_running = false;
		}
	}

	void ResumeAi()
	{
		foreach(itr, m_threadStorage)
		{
			(*itr)->m_running = true;
		}
	}

	void ShutdownThreads() 
	{ 
		foreach(itr, m_threadStorage)
		{
			(*itr)->m_running = true;
			(*itr)->m_exit = true;
		}
	}

private:
	
};
#endif