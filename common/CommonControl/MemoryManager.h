#pragma once

#define DEF_BUFFER_INDEX										(16)
#define DEF_BUFFER_SEPPING										(4096) //不能小于2096

#define MEMMANAGE_POSSIZE										(m_pDataInfoPoint->uMaxHandlerIndex * sizeof(MEMMANAGE_HANDLER) + sizeof(MEMMANAGE_INFORMATION))


#define memmanage_checkbuffersize(size)							((m_pDataInfoPoint->uDataContentSize - m_pDataInfoPoint->uDataCurrentContnetPos) < size)
#define memmanage_getbuffersizebycontentsize(contentsize)		(contentsize + MEMMANAGE_POSSIZE)
#define memmanage_getcontentsizebybuffersize(contentsize)		(contentsize - MEMMANAGE_POSSIZE)

#define memmanage_gethandlerbybuffer(buffer)						(PMEMMANAGE_HANDLER)((ULONG_PTR)buffer +  sizeof(MEMMANAGE_INFORMATION))
#define memmanage_getbufferbyhandler(handler)						(handler -  sizeof(MEMMANAGE_INFORMATION))

#define memmanage_getcontentbybuffer(buffer)					(bit *)((ULONG_PTR)buffer + MEMMANAGE_POSSIZE)
#define memmanage_getbufferbycontent(content)					(void *)((ULONG_PTR)content - MEMMANAGE_POSSIZE)
#define memmanage_getcurrentpoint()								(void *)((ULONG_PTR)m_pDataContentPoint + m_pDataInfoPoint->uDataCurrentContnetPos)
#define memmanage_gethandlersizebyindex(index)					m_pDataHandlerPoint[(ULONG_PTR)index].umemsize
#define memmanage_gethandlerpointbyindex(index)					(void *)&m_pDataContentPoint[m_pDataHandlerPoint[(ULONG_PTR)index].umempos]

#ifndef memmanage_memalloc
#define memmanage_memalloc(size)								new bit[size + MEMMANAGE_POSSIZE]
#endif

#ifndef memmanage_memfree
#define memmanage_memfree(buffer)								delete [] memmanage_getbufferbycontent(buffer)
#endif

#ifndef memmanage_memcpy
#define memmanage_memcpy(target,source,size)					memcpy(target,source,size)
#endif

#ifndef memmanage_memset
#define memmanage_memset(target,size)							memset(target,0,size)
#endif

#define memmanage_copycontenttopoint(target)					memmanage_memcpy(target,memmanage_getbufferbycontent(m_pDataContentPoint),memmanage_getbuffersizebycontentsize(m_pDataInfoPoint->uDataContentSize))

typedef unsigned char bit;
typedef unsigned long mmhandler;

typedef struct _MEMMANAGE_HANDLER
{
	unsigned long umempos;
	unsigned long umemsize;
}MEMMANAGE_HANDLER, *PMEMMANAGE_HANDLER;

typedef struct _MEMMANAGE_INFORMATION
{
	unsigned long uDataContentSize;
	unsigned long uMaxHandlerIndex;
	unsigned long uDataCurrentContnetPos;
	unsigned long uDataCurrentHandlerIndex;
}MEMMANAGE_INFORMATION, * PMEMMANAGE_INFORMATION;

class CMemoryManager
{
public:
	CMemoryManager(unsigned long uSize = 0);
	~CMemoryManager();

public:
	void * Get(mmhandler pHandle, unsigned long * uSize = NULL);

	mmhandler Set(unsigned long uSize);
	mmhandler Set(const void * pData, unsigned long uSize = 0);

	void * GetAll(unsigned long * uSize = NULL);
	void    SetAll(void * pData,unsigned long uSize);

	unsigned long GetHandleSize(unsigned long * uMaxHandleSize);

protected:
	void * Alloc(unsigned long uSize);
	void   ReBuffer(unsigned long uSize = DEF_BUFFER_SEPPING);

protected:
	bit * m_pDataContentPoint;
	PMEMMANAGE_HANDLER m_pDataHandlerPoint;

	MEMMANAGE_INFORMATION m_mmDataInfo;
	PMEMMANAGE_INFORMATION m_pDataInfoPoint;
};

