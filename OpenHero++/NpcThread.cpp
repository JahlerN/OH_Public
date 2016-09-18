#include "stdafx.h"
//#include "NpcThread.h"
#include "..\SharedFiles\globals.h"

void NpcThreadProc(CNpcThread* pThread)
{
	uint32 curTime = 0;
	uint32 prevTime = GetMSTime();

	uint32 prevSleepTime = 0;

	while (!pThread->m_exit)
	{
		curTime = GetMSTime();

		uint32 diff = 0;
		if (prevTime > curTime)
			diff = (0xFFFFFFFF - prevTime) + curTime;
		else
			diff = curTime - prevTime;

		//Update logic
		if (pThread->m_running)
		{
			foreach(itr, pThread->m_mapInstanceList)
			{
				MapInstance* t = (*itr);

				if (!t->HasUsers())
					continue;

				//for (int x = 0; x < t->GetMap()->GetXRegionMax(); x++)
					//for (int z = 0; z < t->GetMap()->GetZRegionMax(); z++)
				auto npcMap = t->GetNpcList();
				for (auto itr = npcMap.begin(); itr != npcMap.end();)
				{
					//Region* pRegion = t->GetRegion(x, z);
					//if (pRegion == NULL)
					//	continue;

					//for (auto n = pRegion->m_regionNpcMap.m_UserTypeMap.begin(); n != pRegion->m_regionNpcMap.m_UserTypeMap.end();)
					//{
						//CNpc* cn = t->GetNpcPtr((*(*n).second));
					CNpc* n = (*itr).second;
					itr++;
					if (n == NULL || n->GetType() == 25)//TODO: 25 = summon, if it's actualy summoned start updating.(Summon flag when summoned, remove when revived).
						continue;

					n->UpdateAI(diff);
					//cn++
					/*if (cn == NULL || !cn->IsMonster())
						continue;

					cn->UpdateAI(diff);*/
					//}
				}
				/*auto userList = t->GetUserList();
				foreach(user, userList)
				{
					CUser* pUser = (*user);
					if (pUser == NULL)
						continue;
					foreach_region(x, z)
					{
						Region* pRegion = t->GetRegion(pUser->GetRegionX() + x, pUser->GetRegionZ() + z);
						if (pRegion == NULL || pRegion->m_regionNpcMap.IsEmpty() || pRegion->m_marked)
							continue;

						for (auto n = pRegion->m_regionNpcMap.m_UserTypeMap.begin(); n != pRegion->m_regionNpcMap.m_UserTypeMap.end();)
						{
							CNpc* cn = t->GetNpcPtr((*(*n).second));
							n++;
							if (!cn->IsMonster())
								continue;

							UpdateNpc(cn);
						}

						pRegion->m_marked = true;
					}
				}*/

				//for (int x = 0; x < t->GetMap()->GetXRegionMax(); x++)
				//	for (int z = 0; z < t->GetMap()->GetZRegionMax(); z++)
				//	{
				//		Region* pRegion = t->GetRegion(x, z);
				//		if (pRegion->m_regionUserMap.IsEmpty() || pRegion->m_marked)
				//			continue;

				//		//TODO: A way better solution to all this shit is to just run update for all, check if there's a player in the region, no? go on. Else, update nearby regions and mark them as updated.
				//		for (auto n = pRegion->m_regionNpcMap.m_UserTypeMap.begin(); n != pRegion->m_regionNpcMap.m_UserTypeMap.end();)
				//		{
				//			CNpc* cn = t->GetNpcPtr((*(*n).second));
				//			n++;
				//			UpdateNpc(cn);
				//		}

				//		pRegion->m_marked = true;
				//		//foreach(npc, (*aReg)->m_regionNpcMap.m_UserTypeMap)
				//			//UpdateNpc(t->GetNpcPtr();
				//	}


				/*for (int x = 0; x < t->GetMap()->GetXRegionMax(); x++)
					for (int z = 0; z < t->GetMap()->GetZRegionMax(); z++)
					{
						t->GetRegion(x, z)->m_marked = false;
					}
*/

			}
			//float time = GetFloatTime();
			//foreach(itr, pThread->m_npcList)
			//{
			//	CNpc* pNpc = (*itr);

			//	if (pNpc->IsDead())
			//	{
			//		if (GetFloatTime() > pNpc->m_killTime + pNpc->GetRespawnTime())
			//			pNpc->Revive();
			//		else//I think there's nothing else we need to do if the npc is dead.
			//			continue;
			//	}

			//	switch (pNpc->m_npcState)
			//	{
			//	case NS_STANDING:
			//		break;
			//	case NS_RANDOM_MOVEMENT:
			//		break;
			//	case NS_ATTACKING_PLAYER:
			//		break;
			//	default:
			//		break;
			//	}
			//}
		}

		prevTime = curTime;

		if (diff <= 150 + prevSleepTime)
		{
			prevSleepTime = 150 + prevSleepTime - diff;

			std::this_thread::sleep_for(std::chrono::milliseconds(prevSleepTime));
		}
		else
			prevSleepTime = 0;

		//std::this_thread::sleep_for(std::chrono::milliseconds(160));
	}

	pThread->m_thread.join();
}

CNpcThreadMgr::CNpcThreadMgr() : m_threadCount(0)
{	

}


CNpcThreadMgr::~CNpcThreadMgr()
{

}
uint32 CNpcThreadMgr::CreateAndAssignNpcThreads(std::map<uint8, MapInstance*> mapList)
{
	std::vector<CNpcThread*> tempList;
	for (int i = 0; i < THREADS_PER_SERVER; i++)
	{
		CNpcThread* pNpcThread = new CNpcThread();
		pNpcThread->m_threadNum = m_threadStorage.size() + 1;
		pNpcThread->m_thread = std::thread(NpcThreadProc, pNpcThread);
		tempList.push_back(pNpcThread);
	}

	uint16 threadIndex = 0;
	for (auto itr = mapList.begin(); itr != mapList.end(); itr++)
	{
		MapInstance* pInstance = itr->second;

		CNpcThread* t = tempList.at(threadIndex);

		t->m_mapInstanceList.push_back(pInstance);

		threadIndex++;
		if (threadIndex >= tempList.size())
			threadIndex = 0;
	}

	foreach(itr, tempList)
		m_threadStorage.push_back((*itr));

	return 0;
}


//uint32 CNpcThreadMgr::CreateAndAssignNpcThreads(std::list<CNpc*>* npcList)
//{
//	
	
	//uint32 totalNpcInThread = 0;
	//std::vector<CNpcThread*> tempList;
	//for (int i = 0; i < THREADS_PER_SERVER; i++)
	//{
	//	CNpcThread* pNpcThread = new CNpcThread();
	//	pNpcThread->m_threadNum = m_threadStorage.size() + 1;
	//	pNpcThread->m_thread = std::thread(NpcThreadProc, pNpcThread);
	//	tempList.push_back(pNpcThread);
	//}

	//uint16 threadIndex = 0;
	//for (auto itr = npcList->begin(); itr != npcList->end(); itr++)
	//{
	//	CNpc* pNpc = (*itr);

	//	if (!pNpc->IsMonster())
	//		continue;

	//	CNpcThread* pNpcThread = tempList.at(threadIndex);

	//	pNpcThread->m_npcList.push_back(pNpc);

	//	threadIndex++;
	//	if (threadIndex >= tempList.size())
	//		threadIndex = 0;

	//	totalNpcInThread++;
	//}

	////Add threads to container
	//foreach(itr, tempList)
	//	m_threadStorage.push_back((*itr));

	//return totalNpcInThread;
//}

