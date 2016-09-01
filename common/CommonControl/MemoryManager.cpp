#include <Windows.h>
#include <memory.h>
#include <tchar.h>

#include "MemoryManager.h"


CMemoryManager::CMemoryManager(unsigned long uSize /* = 0 */)
{
	m_mmDataInfo.uDataCurrentContnetPos		= 0;
	m_mmDataInfo.uDataContentSize			= 0;
	m_mmDataInfo.uMaxHandlerIndex			= (0 == uSize) ? DEF_BUFFER_INDEX : uSize + 1;
	m_mmDataInfo.uDataCurrentHandlerIndex	= 0;

	m_pDataContentPoint			= NULL;
	m_pDataHandlerPoint			= NULL;
	m_pDataInfoPoint = &m_mmDataInfo;
}
CMemoryManager::~CMemoryManager()
{
	if (m_pDataContentPoint)
		memmanage_memfree(m_pDataContentPoint);
}

mmhandler CMemoryManager::Set(unsigned long uSize)
{
	if (m_pDataInfoPoint->uMaxHandlerIndex == m_pDataInfoPoint->uDataCurrentHandlerIndex)
		return (mmhandler)NULL;

	memmanage_memset(this->Alloc(uSize),uSize);

	return (mmhandler)m_pDataInfoPoint->uMaxHandlerIndex - (m_pDataInfoPoint->uDataCurrentHandlerIndex - 1);
}

mmhandler CMemoryManager::Set(const void * pData, unsigned long uSize /* = 0 */)
{
	if (!pData)
		return (mmhandler)NULL;

	if (0 == uSize)
		uSize = (_tcslen((TCHAR *)pData) + 1) * sizeof(TCHAR);

	if (m_pDataInfoPoint->uMaxHandlerIndex == m_pDataInfoPoint->uDataCurrentHandlerIndex)
		return (mmhandler)NULL;

	memmanage_memcpy(this->Alloc(uSize), pData, uSize);

	return (mmhandler)m_pDataInfoPoint->uMaxHandlerIndex - (m_pDataInfoPoint->uDataCurrentHandlerIndex - 1);
}

void * CMemoryManager::Alloc(unsigned long uSize)
{
	if (memmanage_checkbuffersize(uSize))
		ReBuffer((uSize < DEF_BUFFER_SEPPING) ? DEF_BUFFER_SEPPING : uSize);

	void * pCurrentPoint = memmanage_getcurrentpoint();

	m_pDataHandlerPoint[m_pDataInfoPoint->uDataCurrentHandlerIndex].umempos  = m_pDataInfoPoint->uDataCurrentContnetPos;
	m_pDataHandlerPoint[m_pDataInfoPoint->uDataCurrentHandlerIndex].umemsize = uSize;
	m_pDataInfoPoint->uDataCurrentHandlerIndex++;
	m_pDataInfoPoint->uDataCurrentContnetPos += uSize;

	return pCurrentPoint;
}

void CMemoryManager::ReBuffer(unsigned long uSize /* = MAX_BUFFER_SEPPING */)
{
	uSize += MEMMANAGE_POSSIZE;
	unsigned long uMuitiple = uSize / DEF_BUFFER_SEPPING;

	if (uSize % DEF_BUFFER_SEPPING)
		uMuitiple++;

	uSize = m_pDataInfoPoint->uDataContentSize + uMuitiple * DEF_BUFFER_SEPPING - MEMMANAGE_POSSIZE;
	void * pNewBuffer = memmanage_memalloc(uSize);

	if (NULL != m_pDataContentPoint)
	{
		memmanage_copycontenttopoint(pNewBuffer);
		memmanage_memfree(m_pDataContentPoint);
	}
	else
	{
		memmanage_memcpy(pNewBuffer,&m_mmDataInfo,sizeof(MEMMANAGE_INFORMATION));
	}

	m_pDataInfoPoint = (PMEMMANAGE_INFORMATION)pNewBuffer;
	m_pDataHandlerPoint = memmanage_gethandlerbybuffer(pNewBuffer);
	m_pDataContentPoint = memmanage_getcontentbybuffer(pNewBuffer);

	m_pDataInfoPoint->uDataContentSize  = uSize;
}

void * CMemoryManager::Get(mmhandler mmHandle, unsigned long * uSize /* = NULL */)
{
	if (!mmHandle)
		return NULL;

	mmHandle = m_pDataInfoPoint->uMaxHandlerIndex - mmHandle;

	void * pReturnPoint = NULL;

	if (m_pDataContentPoint && (mmhandler)-1 != mmHandle)
	{
		pReturnPoint = memmanage_gethandlerpointbyindex(mmHandle);

		if (uSize)
			*uSize = memmanage_gethandlersizebyindex(mmHandle);
	}

	return pReturnPoint;
}

void CMemoryManager::SetAll(void * pData, unsigned long uSize)
{
	if (NULL == pData)
		return ;

	if (NULL != m_pDataContentPoint)
		memmanage_memfree(m_pDataContentPoint);

	void * pBuffer = memmanage_memalloc(uSize);
	memmanage_memcpy(pBuffer,pData,uSize);

	m_pDataInfoPoint = (PMEMMANAGE_INFORMATION)pBuffer;
	m_pDataHandlerPoint = memmanage_gethandlerbybuffer(pBuffer);
	m_pDataContentPoint = memmanage_getcontentbybuffer(pBuffer);

	m_pDataInfoPoint->uDataContentSize = memmanage_getcontentsizebybuffersize(uSize);
}

void * CMemoryManager::GetAll(unsigned long * uSize /* = NULL */)
{
	if (NULL == m_pDataContentPoint)
		return NULL;

	if (uSize)
		*uSize = memmanage_getbuffersizebycontentsize(m_pDataInfoPoint->uDataContentSize);

	return memmanage_getbufferbycontent(m_pDataContentPoint);
}

unsigned long CMemoryManager::GetHandleSize(unsigned long * uMaxHandleSize)
{
	if (uMaxHandleSize)
		*uMaxHandleSize = m_pDataInfoPoint->uMaxHandlerIndex;

	return m_pDataInfoPoint->uDataCurrentHandlerIndex;
}