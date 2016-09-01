#pragma once 

#include <winbase.h>

namespace Lock{

	class ILock{
	public:
		virtual void lock() = 0;
		virtual void unlock() = 0;
	};

	class CAuto {
	public:
		//构造的时候调用lock。  
		inline CAuto(ILock * iLock) : m_pLock(iLock) { m_pLock->lock(); }
		inline CAuto(ILock & iLock) : m_pLock(&iLock)  { m_pLock->lock(); }
		//析构的时候调用unlock。  
		inline ~CAuto() { m_pLock->unlock(); }
	private:
		ILock * m_pLock;
	};

	class CCritical : public ILock{
	public:
		CCritical(){ InitializeCriticalSection(&m_CriticalSection); }
		~CCritical(){ DeleteCriticalSection(&m_CriticalSection); }

	public:
		virtual void lock() { EnterCriticalSection(&m_CriticalSection); }
		virtual void unlock() { LeaveCriticalSection(&m_CriticalSection); }

	private:
		CRITICAL_SECTION m_CriticalSection;
	};

	class CMutex : public ILock{
	public:
		CMutex(const TCHAR * pszMutexName){ m_hMutex = ::CreateMutex(NULL, FALSE, pszMutexName); }
		~CMutex(){ ::CloseHandle(m_hMutex); }

	public:
		virtual void lock() { DWORD d = WaitForSingleObject(m_hMutex, INFINITE); }
		virtual void unlock() { ::ReleaseMutex(m_hMutex); }

	private:
		HANDLE m_hMutex;
	};
}
