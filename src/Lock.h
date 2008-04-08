#ifndef LOCK_H
#define LOCK_H

#include <windows.h>

class MyLock
{
	CRITICAL_SECTION m_cs;
public:
	inline MyLock() {::InitializeCriticalSection(&m_cs);}
	inline void Lock() {::EnterCriticalSection(&m_cs);}
	inline void Unlock() {::LeaveCriticalSection(&m_cs);}
	inline bool IsLocked() {return m_cs.LockCount != 0;}
};

extern MyLock g_lock;

class AutoLock
{
public:
	inline AutoLock() {g_lock.Lock();}
	inline ~AutoLock() {g_lock.Unlock();}
};

#endif // LOCK_H
