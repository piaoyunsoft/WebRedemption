#pragma  once
#include <malloc.h>
#include <windows.h>

#define MAX_CONTAINER													0x100
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type *)( (PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))
#endif

#define TEMPLATE_CONTAINER template<typename  KeyT ,typename  TypedefT>

TEMPLATE_CONTAINER
class CContainer
{
public:
	~CContainer<KeyT, TypedefT>();
	CContainer<KeyT, TypedefT>(unsigned long uMaxCount = MAX_CONTAINER);

public:
	void DelAll();
	void DelIndex(int nIndex);
	void Del(KeyT kContainer);
	int Add(KeyT kContainer, TypedefT vContainer, bool bIsReset = false);
	int Add(KeyT kContainer, TypedefT * vContainer, bool bIsReset = false);

public:
	TypedefT * GetIndex(unsigned long iIndex, TypedefT * pvContainer = NULL);

public:
	bool Find(KeyT kContainer, TypedefT * pvContainer = NULL);
	bool Find(KeyT kContainer, TypedefT ** pvContainer = NULL);

protected:
	typedef struct CONTAINERINFO
	{
		unsigned long uContainerIndex;
		unsigned long uContainerCount;
		struct MAPINFO{
			KeyT kContainerKey;
			TypedefT ContainerInfo;
		}pContainerInfos[1];
	} *HCONTAINER;

protected:
	HCONTAINER m_hContainerHandle;
	CRITICAL_SECTION m_csContainerLock;
};

TEMPLATE_CONTAINER CContainer<KeyT, TypedefT>::CContainer(unsigned long uMaxCount/* = MAX_REDIRECT*/)
{
	unsigned long uBufferLength = (sizeof(CONTAINERINFO) - sizeof(CONTAINERINFO::MAPINFO)) + sizeof(CONTAINERINFO::MAPINFO)* uMaxCount;

	m_hContainerHandle = (HCONTAINER)malloc(uBufferLength);

	if (m_hContainerHandle)
	{
		memset(m_hContainerHandle, 0, uBufferLength);

		m_hContainerHandle->uContainerIndex = 0;
		m_hContainerHandle->uContainerCount = uMaxCount;
	}

	InitializeCriticalSection(&m_csContainerLock);
}

TEMPLATE_CONTAINER CContainer<KeyT, TypedefT>::~CContainer()
{
	if (m_hContainerHandle)
		free(m_hContainerHandle);
}

TEMPLATE_CONTAINER int CContainer<KeyT, TypedefT>::Add(KeyT kContainer, TypedefT * pvContainer, bool bIsReset/* = false*/)
{
	TypedefT * pvExistContainer = NULL;
	CONTAINERINFO::MAPINFO * pvSaveContainer = NULL;
	int nCurrentIndex = m_hContainerHandle->uContainerIndex;

	if (this->Find(kContainer, &pvExistContainer) && false == bIsReset)
		return -1;

	if(NULL == pvExistContainer)
		pvExistContainer = &m_hContainerHandle->pContainerInfos[m_hContainerHandle->uContainerIndex].ContainerInfo;

	pvSaveContainer = CONTAINING_RECORD(pvExistContainer, CONTAINERINFO::MAPINFO, ContainerInfo);

	EnterCriticalSection(&m_csContainerLock);

	pvSaveContainer->kContainerKey = kContainer;

	memcpy(&pvSaveContainer->ContainerInfo, pvContainer, sizeof(pvSaveContainer->ContainerInfo));

	m_hContainerHandle->uContainerIndex++;

	if (m_hContainerHandle->uContainerCount <= m_hContainerHandle->uContainerIndex)
		m_hContainerHandle->uContainerIndex = 0;

	LeaveCriticalSection(&m_csContainerLock);

	return nCurrentIndex;
}

TEMPLATE_CONTAINER int CContainer<KeyT, TypedefT>::Add(KeyT kContainer, TypedefT vContainer, bool bIsReset/* = false*/)
{
	return this->Add(kContainer, &vContainer, bIsReset);
}

TEMPLATE_CONTAINER void CContainer<KeyT, TypedefT>::DelAll()
{
	m_hContainerHandle->uContainerIndex = 0;
}

TEMPLATE_CONTAINER void CContainer<KeyT, TypedefT>::Del(KeyT kContainer)
{
	EnterCriticalSection(&m_csContainerLock);

	for (unsigned long i = 0; i < m_hContainerHandle->uContainerIndex; i++)
	{
		if (kContainer == m_hContainerHandle->pContainerInfos[i].kContainerKey)
		{
			memset(&m_hContainerHandle->pContainerInfos[i].ContainerInfo, 0, sizeof(m_hContainerHandle->pContainerInfos[i].ContainerInfo));
			memset(&m_hContainerHandle->pContainerInfos[i].kContainerKey, 0, sizeof(m_hContainerHandle->pContainerInfos[i].kContainerKey));

			if (m_hContainerHandle->uContainerIndex > 0)
			{
				m_hContainerHandle->uContainerIndex--;


				memcpy(&m_hContainerHandle->pContainerInfos[i].ContainerInfo, &m_hContainerHandle->pContainerInfos[m_hContainerHandle->uContainerIndex].ContainerInfo, sizeof(m_hContainerHandle->pContainerInfos[i].ContainerInfo));
				memcpy(&m_hContainerHandle->pContainerInfos[i].kContainerKey, &m_hContainerHandle->pContainerInfos[m_hContainerHandle->uContainerIndex].kContainerKey, sizeof(m_hContainerHandle->pContainerInfos[i].kContainerKey));
			}
			break;
		}
	}

	LeaveCriticalSection(&m_csContainerLock);
}

TEMPLATE_CONTAINER TypedefT * CContainer<KeyT, TypedefT>::GetIndex(unsigned long uIndex, __out TypedefT * pvContainer/* = NULL*/)
{
	TypedefT * pvRetContainer = NULL;

	if (uIndex >= m_hContainerHandle->uContainerIndex)
		return NULL;

	EnterCriticalSection(&m_csContainerLock);

	if (pvContainer)
		memcpy(pvContainer, &m_hContainerHandle->pContainerInfos[uIndex].ContainerInfo, sizeof(m_hContainerHandle->pContainerInfos[uIndex].ContainerInfo));

	pvRetContainer = &m_hContainerHandle->pContainerInfos[uIndex].ContainerInfo;

	LeaveCriticalSection(&m_csContainerLock);

	return pvRetContainer;
}

TEMPLATE_CONTAINER void CContainer<KeyT, TypedefT>::DelIndex(int nIndex)
{
	EnterCriticalSection(&m_csContainerLock);

	memset(&m_hContainerHandle->pContainerInfos[nIndex].ContainerInfo, 0, sizeof(m_hContainerHandle->pContainerInfos[nIndex].ContainerInfo));
	memset(&m_hContainerHandle->pContainerInfos[nIndex].kContainerKey, 0, sizeof(m_hContainerHandle->pContainerInfos[nIndex].kContainerKey));

	if (m_hContainerHandle->uContainerIndex > 0)
	{
		m_hContainerHandle->uContainerIndex--;


		memcpy(&m_hContainerHandle->pContainerInfos[nIndex].ContainerInfo, &m_hContainerHandle->pContainerInfos[m_hContainerHandle->uContainerIndex].ContainerInfo, sizeof(m_hContainerHandle->pContainerInfos[nIndex].ContainerInfo));
		memcpy(&m_hContainerHandle->pContainerInfos[nIndex].kContainerKey, &m_hContainerHandle->pContainerInfos[m_hContainerHandle->uContainerIndex].kContainerKey, sizeof(m_hContainerHandle->pContainerInfos[nIndex].kContainerKey));
	}

	LeaveCriticalSection(&m_csContainerLock);
}

TEMPLATE_CONTAINER bool CContainer<KeyT, TypedefT>::Find(KeyT kContainer, __out TypedefT * * pvContainer)
{
	bool bIsFind = false;
	unsigned long i = 0;

	if (NULL == pvContainer)
		return false;

	EnterCriticalSection(&m_csContainerLock);

	for (i = 0; i < m_hContainerHandle->uContainerIndex; i++)
	{
		if (kContainer == m_hContainerHandle->pContainerInfos[i].kContainerKey)
		{
			bIsFind = true;
			*pvContainer = &m_hContainerHandle->pContainerInfos[i].ContainerInfo;

			break;
		}
	}

	LeaveCriticalSection(&m_csContainerLock);

	return bIsFind;
}

TEMPLATE_CONTAINER bool CContainer<KeyT, TypedefT>::Find(KeyT kContainer, __out TypedefT * pvContainer)
{
	TypedefT * pvFindContainer = NULL;

	if (NULL == pvContainer)
		return false;

	if (false == this->Find(kContainer, &pvFindContainer))
		return false;

	EnterCriticalSection(&m_csContainerLock);

	memcpy(pvContainer, pvFindContainer, sizeof(TypedefT));

	LeaveCriticalSection(&m_csContainerLock);

	return true;
}
