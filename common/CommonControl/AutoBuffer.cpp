#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

#include "AutoBuffer.h"


CAutoBuffer::CAutoBuffer()
{
	m_pBufferInfo = NULL;
	Initialization(MAX_BUFFER_COUNT);

	InitializeCriticalSection(&m_csCriticalSection);
}

CAutoBuffer::~CAutoBuffer()
{
	this->Lock();
	for (int i = 0; i < m_pBufferInfo->uBufferCount; i++)
	{
		m_pBufferInfo->pBufferInfo[i].IsUSE = true;
		free(m_pBufferInfo->pBufferInfo[i].pBuffer);
		m_pBufferInfo->pBufferInfo[i].uBufferLength = 0;
	}

	free(m_pBufferInfo);
	this->UnLock();
}

void CAutoBuffer::Initialization(unsigned long uBufferCount /* = 1 */)
{
	unsigned long uBufferFullSize = sizeof(BUFFERINFORMATION)+sizeof(BUFFERINFORMATION::BUFFER) * (uBufferCount - 1);

	PBUFFERINFORMATION pBufferInfo = (PBUFFERINFORMATION)malloc(uBufferFullSize);

	memset(pBufferInfo, 0, uBufferFullSize);

	if (m_pBufferInfo)
	{
		memcpy(pBufferInfo, m_pBufferInfo, sizeof(BUFFERINFORMATION)+sizeof(BUFFERINFORMATION::BUFFER) * (m_pBufferInfo->uBufferCount - 1));
		free(m_pBufferInfo);
	}

	m_pBufferInfo = pBufferInfo;

	m_pBufferInfo->uBufferCount = uBufferCount;
}
void * CAutoBuffer::ReLockBuffer(void * pData, unsigned long uBufferSize)
{
	void * pRetData = this->LockBuffer(uBufferSize);

	if (BUFFERINFORMATION::BUFFER * pRetInfo = this->Find(pData))
	{
		uBufferSize = (uBufferSize > pRetInfo->uBufferLength) ? pRetInfo->uBufferLength : uBufferSize;
		memcpy(pRetData, pRetInfo->pBuffer, uBufferSize);
		this->UnlockBuffer(pRetInfo->pBuffer);
	}


	return pRetData;
}

CAutoBuffer::BUFFERINFORMATION::BUFFER * CAutoBuffer::Find(void * pData)
{
	BUFFERINFORMATION::BUFFER * pRetInfo = NULL;
	this->Lock();

	for (int i = 0; i < m_pBufferInfo->uBufferCount; i++)
	{
		if (pData != m_pBufferInfo->pBufferInfo[i].pBuffer)
			continue;

		pRetInfo = &m_pBufferInfo->pBufferInfo[i];
		break;
	}

	this->UnLock();

	return pRetInfo;
}


void * CAutoBuffer::LockBuffer(const unsigned long uBufferSize)
{
	int iUnlockIndex = 0;
	void * pRetBuffer = NULL;
	unsigned long uAllocLength = 0;

	uAllocLength = uBufferSize + (BYTE_ALIGNMENT - uBufferSize % BYTE_ALIGNMENT);

	this->Lock();

	for (int i = 0; i < m_pBufferInfo->uBufferCount; i++)
	{
		if (true == m_pBufferInfo->pBufferInfo[i].IsUSE)
			continue;

		iUnlockIndex = i;

		if (NULL == m_pBufferInfo->pBufferInfo[i].pBuffer)
		{
			m_pBufferInfo->pBufferInfo[i].pBuffer = malloc(uAllocLength);
			m_pBufferInfo->pBufferInfo[i].uBufferLength = uAllocLength;
			memset(m_pBufferInfo->pBufferInfo[i].pBuffer, 0, m_pBufferInfo->pBufferInfo[i].uBufferLength);
		}

		if (m_pBufferInfo->pBufferInfo[i].uBufferLength < uBufferSize)
			continue;

		m_pBufferInfo->pBufferInfo[i].IsUSE = true;
		pRetBuffer = m_pBufferInfo->pBufferInfo[i].pBuffer;
		memset(m_pBufferInfo->pBufferInfo[i].pBuffer, 0, m_pBufferInfo->pBufferInfo[i].uBufferLength);
		break;
	}

	if (NULL == pRetBuffer)
	{
		if (0 == iUnlockIndex)
		{
			Initialization(MAX_BUFFER_COUNT + m_pBufferInfo->uBufferCount);
			pRetBuffer = LockBuffer(uBufferSize);
		}
		else
		{
			m_pBufferInfo->pBufferInfo[iUnlockIndex].IsUSE = true;

			if (m_pBufferInfo->pBufferInfo[iUnlockIndex].pBuffer)
				free(m_pBufferInfo->pBufferInfo[iUnlockIndex].pBuffer);

			m_pBufferInfo->pBufferInfo[iUnlockIndex].pBuffer = malloc(uAllocLength + 1);
			m_pBufferInfo->pBufferInfo[iUnlockIndex].uBufferLength = uAllocLength;
			memset(m_pBufferInfo->pBufferInfo[iUnlockIndex].pBuffer, 0, m_pBufferInfo->pBufferInfo[iUnlockIndex].uBufferLength);

			pRetBuffer = m_pBufferInfo->pBufferInfo[iUnlockIndex].pBuffer;
		}
	}

	this->UnLock();

	return pRetBuffer;
}

void CAutoBuffer::UnlockBuffer(const void * pBuffer)
{
	this->Lock();

	for (int i = 0; i < m_pBufferInfo->uBufferCount; i++)
	{
		if (pBuffer != m_pBufferInfo->pBufferInfo[i].pBuffer)
			continue;

		if (m_pBufferInfo->pBufferInfo[i].IsUSE)
			m_pBufferInfo->uBufferIndex--;

		m_pBufferInfo->pBufferInfo[i].IsUSE = false;
		break;
	}

	this->UnLock();
}

unsigned long CAutoBuffer::GetIndex()
{
	return m_pBufferInfo->uBufferIndex;
}

unsigned long CAutoBuffer::GetCount()
{
	return m_pBufferInfo->uBufferCount;
}