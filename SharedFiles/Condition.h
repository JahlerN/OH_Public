#pragma once

#include "Mutex.h"
#include "stdafx.h" //TODO: Shouldn't have to be here in the nd
#include <deque>
#define MAX_AWAITING_THREADS 10

struct list_entry
{
	HANDLE semaphore;
	long count;
	bool notified;
};

class Condition
{
public:
	__forceinline Condition(Mutex * mutex) : m_nLockCount(0), m_externalMutex(mutex)
	{
		::InitializeCriticalSection(&m_critsecWaitSetProtection);
	}

	~Condition()
	{
		::DeleteCriticalSection(&m_critsecWaitSetProtection);
	}

	__forceinline void BeginSynchronized()
	{
		m_externalMutex->Acquire();
		++m_nLockCount;
	}

	__forceinline void EndSynchronized()
	{
		--m_nLockCount;
		m_externalMutex->Release();
	}

	DWORD Wait(time_t timeout)
	{
		DWORD dwMillisecondsTimeout = (DWORD)timeout * 1000;
		BOOL bAlertable = FALSE;
		ASSERT(LockHeldByCallingThread());

		// Enter a new event handle into the wait set.
		HANDLE hWaitEvent = Push();
		if (NULL == hWaitEvent)
			return WAIT_FAILED;

		// Store the current lock count for re-acquisition.
		int nThisThreadsLockCount = m_nLockCount;
		m_nLockCount = 0;

		// Release the synchronization lock the appropriate number of times.
		// Win32 allows no error checking here.
		for (int i = 0; i<nThisThreadsLockCount; ++i)
		{
			//::LeaveCriticalSection(&m_critsecSynchronized);
			m_externalMutex->Release();
		}

		// NOTE: Conceptually, releasing the lock and entering the wait
		// state is done in one atomic step. Technically, that is not
		// true here, because we first leave the critical section and
		// then, in a separate line of code, call WaitForSingleObjectEx.
		// The reason why this code is correct is that our thread is placed
		// in the wait set *before* the lock is released. Therefore, if
		// we get preempted right here and another thread notifies us, then
		// that notification will *not* be missed: the wait operation below
		// will find the event signalled.

		// Wait for the event to become signalled.
		DWORD dwWaitResult = ::WaitForSingleObjectEx(
			hWaitEvent,
			dwMillisecondsTimeout,
			bAlertable
			);

		// If the wait failed, store the last error because it will get
		// overwritten when acquiring the lock.
		DWORD dwLastError = 0;
		if (WAIT_FAILED == dwWaitResult)
			dwLastError = ::GetLastError();

		// Acquire the synchronization lock the appropriate number of times.
		// Win32 allows no error checking here.
		for (int j = 0; j<nThisThreadsLockCount; ++j)
		{
			//::EnterCriticalSection(&m_critsecSynchronized);
			m_externalMutex->Acquire();
		}

		// Restore lock count.
		m_nLockCount = nThisThreadsLockCount;

		// Close event handle
		if (!CloseHandle(hWaitEvent))
			return WAIT_FAILED;

		if (WAIT_FAILED == dwWaitResult)
			::SetLastError(dwLastError);

		return dwWaitResult;
	}

	DWORD Wait()
	{
		DWORD dwMillisecondsTimeout = INFINITE;
		BOOL bAlertable = FALSE;
		ASSERT(LockHeldByCallingThread());

		// Enter a new event handle into the wait set.
		HANDLE hWaitEvent = Push();
		if (NULL == hWaitEvent)
			return WAIT_FAILED;

		// Store the current lock count for re-acquisition.
		int nThisThreadsLockCount = m_nLockCount;
		m_nLockCount = 0;

		// Release the synchronization lock the appropriate number of times.
		// Win32 allows no error checking here.
		for (int i = 0; i<nThisThreadsLockCount; ++i)
		{
			//::LeaveCriticalSection(&m_critsecSynchronized);
			m_externalMutex->Release();
		}

		// NOTE: Conceptually, releasing the lock and entering the wait
		// state is done in one atomic step. Technically, that is not
		// true here, because we first leave the critical section and
		// then, in a separate line of code, call WaitForSingleObjectEx.
		// The reason why this code is correct is that our thread is placed
		// in the wait set *before* the lock is released. Therefore, if
		// we get preempted right here and another thread notifies us, then
		// that notification will *not* be missed: the wait operation below
		// will find the event signalled.

		// Wait for the event to become signalled.
		DWORD dwWaitResult = ::WaitForSingleObjectEx(
			hWaitEvent,
			dwMillisecondsTimeout,
			bAlertable
			);

		// If the wait failed, store the last error because it will get
		// overwritten when acquiring the lock.
		DWORD dwLastError = 0;
		if (WAIT_FAILED == dwWaitResult)
			dwLastError = ::GetLastError();

		// Acquire the synchronization lock the appropriate number of times.
		// Win32 allows no error checking here.
		for (int j = 0; j<nThisThreadsLockCount; ++j)
		{
			//::EnterCriticalSection(&m_critsecSynchronized);
			m_externalMutex->Acquire();
		}

		// Restore lock count.
		m_nLockCount = nThisThreadsLockCount;

		// Close event handle
		if (!CloseHandle(hWaitEvent))
			return WAIT_FAILED;

		if (WAIT_FAILED == dwWaitResult)
			::SetLastError(dwLastError);

		return dwWaitResult;

	}

	void Signal()
	{
		// Pop the first handle, if any, off the wait set.
		HANDLE hWaitEvent = Pop();

		// If there is not thread currently waiting, that's just fine.
		if (NULL == hWaitEvent)
			return;

		// Signal the event.
		SetEvent(hWaitEvent);
	}

	void Broadcast()
	{
		// Signal all events on the deque, then clear it. Win32 allows no
		// error checking on entering and leaving the critical section.
		//
		::EnterCriticalSection(&m_critsecWaitSetProtection);
		std::deque<HANDLE>::const_iterator it_run = m_deqWaitSet.begin();
		std::deque<HANDLE>::const_iterator it_end = m_deqWaitSet.end();
		for (; it_run < it_end; ++it_run)
		{
			if (!SetEvent(*it_run))
				return;
		}
		m_deqWaitSet.clear();
		::LeaveCriticalSection(&m_critsecWaitSetProtection);
	}

private:

	HANDLE Push()
	{
		// Create the new event.
		HANDLE hWaitEvent = ::CreateEvent(
			NULL, // no security
			FALSE, // auto-reset event
			FALSE, // initially unsignalled
			NULL // string name
			);
		//
		if (NULL == hWaitEvent) {
			return NULL;
		}

		// Push the handle on the deque.
		::EnterCriticalSection(&m_critsecWaitSetProtection);
		m_deqWaitSet.push_back(hWaitEvent);
		::LeaveCriticalSection(&m_critsecWaitSetProtection);

		return hWaitEvent;
	}

	HANDLE Pop()
	{
		// Pop the first handle off the deque.
		//
		::EnterCriticalSection(&m_critsecWaitSetProtection);
		HANDLE hWaitEvent = NULL;
		if (0 != m_deqWaitSet.size())
		{
			hWaitEvent = m_deqWaitSet.front();
			m_deqWaitSet.pop_front();
		}
		::LeaveCriticalSection(&m_critsecWaitSetProtection);

		return hWaitEvent;
	}

	BOOL LockHeldByCallingThread()
	{
		//BOOL bTryLockResult = ::TryEnterCriticalSection(&m_critsecSynchronized);
		BOOL bTryLockResult = m_externalMutex->AttemptAcquire();

		// If we didn't get the lock, someone else has it.
		//
		if (!bTryLockResult)
		{
			return FALSE;
		}

		// If we got the lock, but the lock count is zero, then nobody had it.
		//
		if (0 == m_nLockCount)
		{
			m_externalMutex->Release();
			return FALSE;
		}

		// Release lock once. NOTE: we still have it after this release.
		// Win32 allows no error checking here.
		m_externalMutex->Release();

		return TRUE;
	}

	std::deque<HANDLE> m_deqWaitSet;
	CRITICAL_SECTION m_critsecWaitSetProtection;
	Mutex * m_externalMutex;
	int m_nLockCount;
};