#ifndef RING0
#define RING3
#endif

#ifdef RING3
#include <windows.h>
#else
#include <windef.h>
#endif

#include "./InlineHook.h"
#include "./LDasm.h"
#include <map>
#include "distorm3/include/distorm.h"

#ifdef _M_AMD64
#define NEW
#endif

#ifdef RING3
#define __malloc(_s)	VirtualAlloc(NULL, _s, MEM_COMMIT, PAGE_EXECUTE_READWRITE)
#define __free(_p)		VirtualFree(_p, 0, MEM_RELEASE)
#ifndef NEW
#define JMP_SIZE						5
#else
#define BEST_JMP_SIZE			5
#define IX86_JMP_SIZE			10
#define AMD64_JMP_SIZE		14
#endif

#define MAX_JMP_SIZE			AMD64_JMP_SIZE
#else
#define __malloc(_s)	ExAllocatePool(NonPagedPool, _s)
#define __free(_p)		ExFreePool(_p)
#define JMP_SIZE		7
#endif

namespace HookControl {

#ifdef RING3

#ifndef NEW

	bool InlineHook(IN	void *OrgProc,		/* 需要Hook的函数地址 */IN	void *NewProc,		/* 代替被Hook函数的地址 */OUT	void **RealProc		/* 返回原始函数的入口地址 */)
	{
		DWORD dwPatchSize;    // 得到需要patch的字节大小
							  //DWORD dwOldProtect;
		LPVOID lpHookFunc;    // 分配的Hook函数的内存
		DWORD dwBytesNeed;    // 分配的Hook函数的大小
		LPBYTE lpPatchBuffer; // jmp 指令的临时缓冲区

		if (!OrgProc || !NewProc || !RealProc)
		{
			return false;
		}
		// 得到需要patch的字节大小
		if (!GetPatchSize(OrgProc, JMP_SIZE, &dwPatchSize))
		{
			return false;
		}

		/*
		0x00000800					0x00000800		sizeof(DWORD)	// dwPatchSize
		JMP	/ FAR 0xAABBCCDD		E9 DDCCBBAA		JMP_SIZE
		...							...				dwPatchSize		// Backup instruction
		JMP	/ FAR 0xAABBCCDD		E9 DDCCBBAA		JMP_SIZE
		*/

		dwBytesNeed = sizeof(DWORD) + JMP_SIZE + dwPatchSize + JMP_SIZE;

		lpHookFunc = __malloc(dwBytesNeed);

		//备份dwPatchSize到lpHookFunc
		*(DWORD *)lpHookFunc = dwPatchSize;

		//跳过开头的4个字节
		lpHookFunc = (LPVOID)((DWORD)lpHookFunc + sizeof(DWORD));

		//开始backup函数开头的字
		memcpy((BYTE *)lpHookFunc + JMP_SIZE, OrgProc, dwPatchSize);

		//修正E9(Jmp)指令
		if (((BYTE *)lpHookFunc + JMP_SIZE)[0] == 0xE9)
			*(DWORD*)((DWORD)lpHookFunc + JMP_SIZE + 1) = *(DWORD *)((DWORD)lpHookFunc + JMP_SIZE + 1) - (((DWORD)lpHookFunc + JMP_SIZE) - (DWORD)OrgProc);

		//修正EB(Jmp)指令(短跳)
		if (((BYTE *)lpHookFunc + JMP_SIZE)[0] == 0xEB)
		{
			((BYTE *)lpHookFunc + JMP_SIZE)[0] = 0xE9;
			*(DWORD*)((DWORD)lpHookFunc + JMP_SIZE + 1) = *((char *)((DWORD)lpHookFunc + JMP_SIZE + 1)) - (((DWORD)lpHookFunc + JMP_SIZE) - ((DWORD)OrgProc)) - (JMP_SIZE - 2);
		}

		lpPatchBuffer = (LPBYTE)__malloc(dwPatchSize);

		//NOP填充
		memset(lpPatchBuffer, 0x90, dwPatchSize);

#ifdef RING3
		//jmp到Hook
		*(BYTE *)lpHookFunc = 0xE9;
		*(DWORD*)((DWORD)lpHookFunc + 1) = (DWORD)NewProc - (DWORD)lpHookFunc - JMP_SIZE;

		//跳回原始
		*(BYTE *)((DWORD)lpHookFunc + 5 + dwPatchSize) = 0xE9;
		*(DWORD*)((DWORD)lpHookFunc + 5 + dwPatchSize + 1) = ((DWORD)OrgProc + dwPatchSize) - ((DWORD)lpHookFunc + JMP_SIZE + dwPatchSize) - JMP_SIZE;


		//jmp 
		*(BYTE *)lpPatchBuffer = 0xE9;
		//注意计算长度的时候得用OrgProc
		*(DWORD*)(lpPatchBuffer + 1) = (DWORD)lpHookFunc - (DWORD)OrgProc - JMP_SIZE;

#else

		//jmp到Hook
		*(BYTE *)lpHookFunc = 0xEA;
		*(DWORD*)((DWORD)lpHookFunc + 1) = (DWORD)NewProc;
		*(WORD*)((DWORD)lpHookFunc + 5) = 0x08;

		//跳回原始
		*(BYTE *)((DWORD)lpHookFunc + JMP_SIZE + dwPatchSize) = 0xEA;
		*(DWORD*)((DWORD)lpHookFunc + JMP_SIZE + dwPatchSize + 1) = ((DWORD)OrgProc + dwPatchSize);
		*(WORD*)((DWORD)lpHookFunc + JMP_SIZE + dwPatchSize + 5) = 0x08;

		//jmp far
		*(BYTE *)lpPatchBuffer = 0xEA;

		//跳到lpHookFunc函数
		*(DWORD*)(lpPatchBuffer + 1) = (DWORD)lpHookFunc;
		*(WORD*)(lpPatchBuffer + 5) = 0x08;
#endif

		*RealProc = (void *)((DWORD)lpHookFunc + JMP_SIZE);

		WriteReadOnlyMemory((LPBYTE)OrgProc, lpPatchBuffer, dwPatchSize);

		AddHookGuard(OrgProc, lpPatchBuffer, dwPatchSize);

		__free(lpPatchBuffer);


		return true;
	}

	void UnInlineHook(void *OrgProc,  /* 需要恢复Hook的函数地址 */void *RealProc  /* 原始函数的入口地址 */)
	{
		DWORD dwPatchSize;
		//DWORD dwOldProtect;
		LPBYTE lpBuffer;

		//找到分配的空间
		lpBuffer = (LPBYTE)((DWORD)RealProc - (sizeof(DWORD) + JMP_SIZE));
		//得到dwPatchSize
		dwPatchSize = *(DWORD *)lpBuffer;

		DelHookGuard(OrgProc);

		WriteReadOnlyMemory((LPBYTE)OrgProc, (LPBYTE)RealProc, dwPatchSize);

		//释放分配的跳转函数的空间
		__free(lpBuffer);

		return;
	}

#else

	unsigned long GetJumpSize(ULONG_PTR PosA, ULONG_PTR PosB)
	{
		ULONG_PTR res = max(PosA, PosB) - min(PosA, PosB);

		if (res <= (ULONG_PTR)0x7FFF0000)
		{
			return BEST_JMP_SIZE;
		}
		else
		{
#ifdef _M_IX86  

			return IX86_JMP_SIZE;

#else ifdef _M_AMD64  

			return AMD64_JMP_SIZE;

#endif  
		}

		return 0;
	}

	bool WriteReadOnlyMemoryJump(void* pAddress, ULONG_PTR JumpTo, ULONG * pulJumpLength)
	{
		bool bIsOK = true;
		DWORD dwOldProtect = 0;

		if (!VirtualProtect(pAddress, MAX_JMP_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProtect))
			return false;

		BYTE *pDst = (BYTE *)pAddress;

		ULONG_PTR dis = max(JumpTo, (ULONG_PTR)pAddress) - min(JumpTo, (ULONG_PTR)pAddress);

		do
		{
			if (NULL == pulJumpLength || *pulJumpLength < BEST_JMP_SIZE)
				break;

			if (dis <= (ULONG_PTR)0x7FFF0000)
			{
				*pulJumpLength = BEST_JMP_SIZE;

				*(pDst++) = 0xE9;
				DWORD dwRelAddr = (DWORD)(JumpTo - (ULONG_PTR)pAddress) - 5;
				memcpy(pDst, &dwRelAddr, sizeof(DWORD));
			}

#ifdef _M_IX86  

			if (*pulJumpLength < IX86_JMP_SIZE)
				break;

			*pulJumpLength = IX86_JMP_SIZE;

			*(pDst++) = 0xFF;
			*(pDst++) = 0x25;
			*((DWORD *)pDst) = (DWORD)(((ULONG_PTR)pDst) + sizeof(DWORD));
			pDst += sizeof(DWORD);
			*((ULONG_PTR *)pDst) = JumpTo;

			break;
#else ifdef _M_AMD64  

			if (*pulJumpLength < AMD64_JMP_SIZE)
				break;

			*pulJumpLength = AMD64_JMP_SIZE;

			*(pDst++) = 0xFF;
			*(pDst++) = 0x25;
			*((DWORD*)pDst) = 0;
			pDst += sizeof(DWORD);
			*((ULONG_PTR *)pDst) = JumpTo;

			break;
#endif  

		} while (false);

		VirtualProtect(pAddress, MAX_JMP_SIZE, dwOldProtect, &dwOldProtect);

		return bIsOK;
	}

	bool InlineHook(IN	void * pHookProcAddr,		/* 需要Hook的函数地址 */IN	void * pNewProcAddr,		/* 代替被Hook函数的地址 */OUT	void ** pRealProcAddr		/* 返回原始函数的入口地址 */)
	{
#define MAX_INSTRUCTIONS 100  

		unsigned int decodedInstructionsCount = 0;
		_DecodedInst decodedInstructions[MAX_INSTRUCTIONS];

#ifdef _M_IX86  
		_DecodeType dt = Decode32Bits;
#else ifdef _M_AMD64  
		_DecodeType dt = Decode64Bits;
#endif  

		if (DECRES_INPUTERR == distorm_decode(0, (const BYTE *)pHookProcAddr, 50, dt, decodedInstructions, MAX_INSTRUCTIONS, &decodedInstructionsCount))
			return NULL;


		BYTE * pPatchBridge = (BYTE*)__malloc(sizeof(unsigned long) * 2 + MAX_INSTRUCTIONS * 3);

		DWORD dwOldProtect = 0;
		VirtualProtect(pPatchBridge, sizeof(unsigned long) * 2 + MAX_INSTRUCTIONS * 3, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		unsigned long * pulInstrSize = (unsigned long *)pPatchBridge;

		*pulInstrSize = MAX_INSTRUCTIONS;
		WriteReadOnlyMemoryJump(&pPatchBridge[sizeof(unsigned long)], (ULONG_PTR)pNewProcAddr, pulInstrSize);

		unsigned long * pulJumpsize = (unsigned long *)&pPatchBridge[*pulInstrSize + sizeof(unsigned long)];

		*pulJumpsize = *pulInstrSize;

		unsigned long ulBridgeIndex = sizeof(unsigned long) + *pulJumpsize + sizeof(unsigned long);

		*pulInstrSize = 0;
		*pRealProcAddr = &pPatchBridge[ulBridgeIndex];

		for (UINT x = 0; x < decodedInstructionsCount; x++)
		{
			if (*pulInstrSize >= *pulJumpsize)
				break;

			BYTE *pCurInstr = (BYTE *)(*pulInstrSize + (ULONG_PTR)pHookProcAddr);

			// Unfortunately, some instructions have relative address. These addresses can not be used  
			// directly in our bridge. We have to handle these kind of instructions ourselves. Some of  
			// them may have different length on x86 and x64. It's not easy to find out all of them so  
			// I'll do this when I find one. Following is an example I found when hook MessageBoxA:  
			if (*pCurInstr == 0x44/*cmp*/)
			{
#ifdef _M_AMD64  
				// Following is a example for cmp: MessageBoxA  
				// 0x771B118F: 44   39 1D   A6 0F 02 00   --   cmp dwordptr[771D213Ch], r11d  
				// 0x771B1196: ...  
				// 00 02 0F A6 is the relative address from 0x771B1196 to 0x771D213C  
				ULONG_PTR OriginalRel = *(DWORD*)(pCurInstr + 3);
				ULONG_PTR AbsoluteAddr = OriginalRel + (ULONG_PTR)pCurInstr + decodedInstructions[x].size;
				ULONG_PTR NextInstAddr = (ULONG_PTR)&pPatchBridge[ulBridgeIndex] + decodedInstructions[x].size;
				ULONG_PTR RelInBridge = AbsoluteAddr - NextInstAddr;

				// The new relative address is larger than 2GB. I have no idea about this.  
				if (max(AbsoluteAddr, NextInstAddr) - min(AbsoluteAddr, NextInstAddr) > ((ULONG_PTR)1 << 31))
				{
					return NULL;
				}

				memcpy(&pPatchBridge[ulBridgeIndex], (void*)pCurInstr, decodedInstructions[x].size);
				*(DWORD*)&pPatchBridge[ulBridgeIndex + 3] = (DWORD)RelInBridge;

				ulBridgeIndex += decodedInstructions[x].size;
#endif  
			}
			/*
			Following case is provided by the auther who wrote "Powerful x86/x64 Mini Hook-Engine".
			However, I haven't find such a case yet. So I commented it out temporarily.

			else if (*pCurInstr == 0x74) // jz
			{
			ULONG_PTR Dest = (dwInstrSize + (ULONG_PTR)Function) + (char) pCurInstr[1];

			WriteJump(&gBridgeBuffer[gBufferIndex], Dest);

			gBufferIndex += dwJumpSize;
			}
			*/
			else
			{
				memcpy(&pPatchBridge[ulBridgeIndex], (void*)pCurInstr, decodedInstructions[x].size);
				ulBridgeIndex += decodedInstructions[x].size;
			}

			*pulInstrSize += decodedInstructions[x].size;
		}

		unsigned long ulJumplen = MAX_INSTRUCTIONS;
		WriteReadOnlyMemoryJump(&pPatchBridge[ulBridgeIndex], (ULONG_PTR)(((BYTE *)pHookProcAddr) + *pulInstrSize), &ulJumplen);

		ulJumplen = MAX_INSTRUCTIONS;
		WriteReadOnlyMemoryJump(pHookProcAddr, (ULONG_PTR)&pPatchBridge[sizeof(unsigned long)], &ulJumplen);

		// 	WriteJump(&gBridgeBuffer[gBufferIndex], Function + dwInstrSize);
		// 	gBufferIndex += GetJumpSize((ULONG_PTR)&gBridgeBuffer[gBufferIndex], Function + dwInstrSize);

		return pPatchBridge;
	}
	void UnInlineHook(void *OrgProc,  /* 需要恢复Hook的函数地址 */void *RealProc  /* 原始函数的入口地址 */)
	{
		DWORD dwPatchSize;
		void * pBuffer = NULL;
		unsigned long ulJumplen = 0;

		//找到分配的空间
		ulJumplen = *((unsigned long *)(((BYTE *)RealProc) - sizeof(unsigned long)));
		//得到dwPatchSize

		pBuffer = (((BYTE *)RealProc) - (sizeof(unsigned long) + ulJumplen + sizeof(unsigned long)));

		dwPatchSize = *(DWORD *)pBuffer;

		//DelHookGuard(OrgProc);

		WriteReadOnlyMemory((LPBYTE)OrgProc, (LPBYTE)RealProc, dwPatchSize);

		//释放分配的跳转函数的空间
		__free(pBuffer);

		return;
	}
#endif

#endif

#ifdef RING3

	BOOL WriteReadOnlyMemory(LPBYTE	lpDest, LPBYTE	lpSource, ULONG	Length)
	{
		BOOL bRet;
		DWORD dwOldProtect;
		bRet = FALSE;

		if (!VirtualProtect(lpDest, Length, PAGE_EXECUTE_READWRITE, &dwOldProtect))
		{
			return bRet;
		}

		memcpy(lpDest, lpSource, Length);

		bRet = VirtualProtect(lpDest, Length, dwOldProtect, &dwOldProtect);

		return	bRet;
	}

#else

	NTSTATUS WriteReadOnlyMemory(LPBYTE	lpDest, LPBYTE	lpSource, ULONG	Length)
	{
		NTSTATUS status;
		KSPIN_LOCK spinLock;
		KIRQL oldIrql;
		PMDL pMdlMemory;
		LPBYTE lpWritableAddress;

		status = STATUS_UNSUCCESSFUL;

		pMdlMemory = IoAllocateMdl(lpDest, Length, FALSE, FALSE, NULL);

		if (NULL == pMdlMemory)
			return status;

		MmBuildMdlForNonPagedPool(pMdlMemory);
		MmProbeAndLockPages(pMdlMemory, KernelMode, IoWriteAccess);
		lpWritableAddress = MmMapLockedPages(pMdlMemory, KernelMode);
		if (NULL != lpWritableAddress)
		{
			oldIrql = 0;
			KeInitializeSpinLock(&spinLock);
			KeAcquireSpinLock(&spinLock, &oldIrql);

			memcpy(lpWritableAddress, lpSource, Length);

			KeReleaseSpinLock(&spinLock, oldIrql);
			MmUnmapLockedPages(lpWritableAddress, pMdlMemory);

			status = STATUS_SUCCESS;
		}

		MmUnlockPages(pMdlMemory);
		IoFreeMdl(pMdlMemory);

		return status;
	}

#endif

	BOOL GetPatchSize(IN	void *Proc,			/* 需要Hook的函数地址 */IN	DWORD dwNeedSize,	/* Hook函数头部占用的字节大小 */OUT LPDWORD lpPatchSize	/* 返回根据函数头分析需要修补的大小 */)
	{
		DWORD Length;
		PUCHAR pOpcode;
		DWORD PatchSize = 0;

		if (!Proc || !lpPatchSize)
		{
			return FALSE;
		}

		do
		{
			Length = SizeOfCode(Proc, &pOpcode);
			if ((Length == 1) && (*pOpcode == 0xC3))
				break;
			if ((Length == 3) && (*pOpcode == 0xC2))
				break;
			Proc = (PVOID)((DWORD)Proc + Length);

			PatchSize += Length;
			if (PatchSize >= dwNeedSize)
			{
				break;
			}

		} while (Length);

		*lpPatchSize = PatchSize;

		return TRUE;
	}

	CRITICAL_SECTION g_csHookGuardLoak;
	HANDLE g_hHookGuardThreadHandle = NULL;
	std::map<void *, byte *> g_mapPatchBuffer;
	std::map<void *, unsigned long> g_mapPatchSize;

	void DelHookGuard(void * lpDest)
	{
		EnterCriticalSection(&g_csHookGuardLoak);

		__free(g_mapPatchBuffer[lpDest]);

		g_mapPatchSize.erase(lpDest);
		g_mapPatchBuffer.erase(lpDest);

		LeaveCriticalSection(&g_csHookGuardLoak);
	}

	void AddHookGuard(void * lpDest, LPBYTE lpSource, ULONG Length)
	{
		if (NULL == g_hHookGuardThreadHandle)
		{
			DWORD dwThreadId = 0;
			InitializeCriticalSection(&g_csHookGuardLoak);
			g_hHookGuardThreadHandle = CreateThread(NULL, 0, ThreadForHookGuard, NULL, 0, &dwThreadId);
		}

		EnterCriticalSection(&g_csHookGuardLoak);

		g_mapPatchSize[lpDest] = Length;
		g_mapPatchBuffer[lpDest] = (byte *)__malloc(Length);

		memcpy(g_mapPatchBuffer[lpDest], lpSource, Length);

		LeaveCriticalSection(&g_csHookGuardLoak);
	}

	DWORD WINAPI ThreadForHookGuard(PVOID)
	{
		unsigned long uBufferLen = 0;
		std::map<void *, byte *>::iterator iterBuffer;
		while (true)
		{
			EnterCriticalSection(&g_csHookGuardLoak);

			if (g_mapPatchSize.size() > 0)
			{
				for (iterBuffer = g_mapPatchBuffer.begin(); iterBuffer != g_mapPatchBuffer.end(); ++iterBuffer)
				{
					uBufferLen = g_mapPatchSize[iterBuffer->first];

					if (0 != uBufferLen &&memcmp(iterBuffer->first, iterBuffer->second, uBufferLen))
						WriteReadOnlyMemory((LPBYTE)iterBuffer->first, iterBuffer->second, uBufferLen);
				}
			}

			LeaveCriticalSection(&g_csHookGuardLoak);
			Sleep(1000);
		}
	}

}