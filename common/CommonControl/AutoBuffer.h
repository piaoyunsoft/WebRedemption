#pragma once
#include <windows.h>

#define BYTE_ALIGNMENT				4096
#define MAX_BUFFER_COUNT		20

class CAutoBuffer{

public:
	CAutoBuffer();
	~CAutoBuffer();

	void    UnlockBuffer(const void * pBuffer);
	void * LockBuffer(const unsigned long uBufferSize);
	void * ReLockBuffer(void * pData,  unsigned long uBufferSize);

	unsigned long GetIndex();
	unsigned long GetCount();

protected:
	void Initialization(unsigned long uBufferCount);
	void Lock()
	{
		EnterCriticalSection(&m_csCriticalSection);
	}

	void UnLock()
	{
		LeaveCriticalSection(&m_csCriticalSection);
	}

protected:
		typedef struct _BUFFERINFORMATION
		{
			unsigned long uBufferIndex;
			unsigned long uBufferCount;
			struct BUFFER
			{
				bool IsUSE;

				void * pBuffer;
				unsigned long uBufferLength;
			} pBufferInfo[1];

		}BUFFERINFORMATION, *PBUFFERINFORMATION;

protected:
	BUFFERINFORMATION::BUFFER * Find(void * pData);

protected:
	CRITICAL_SECTION m_csCriticalSection;
	PBUFFERINFORMATION m_pBufferInfo;
};