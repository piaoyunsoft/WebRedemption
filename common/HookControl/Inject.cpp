#include <Windows.h>
#include <tchar.h>
#include "Inject.h"
#include "HookHelp.h"

namespace HookControl {

	void * AllocRemoteMemory(HANDLE hTargetProcess, SIZE_T sizeMemorySize)
	{
		return VirtualAllocEx(hTargetProcess, NULL, sizeMemorySize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	}

	void FreeRemoteMemory(HANDLE hTargetProcess, void * pRemoteMemoryPointer)
	{
		VirtualFreeEx(hTargetProcess, (void *)pRemoteMemoryPointer, 0, MEM_RELEASE);
	}

	SIZE_T ReadRemoteMemory(HANDLE hTargetProcess, const void * pRemoteMemoryPointer, void * pLocalMemoryPointer, SIZE_T sizeMemorySize)
	{
		if (FALSE == ReadProcessMemory(hTargetProcess, pRemoteMemoryPointer, pLocalMemoryPointer, sizeMemorySize, &sizeMemorySize))
			return 0;

		return sizeMemorySize;
	}

	SIZE_T WriteRemoteMemory(HANDLE hTargetProcess, void * pRemoteMemoryPointer, const void * pLocalMemoryPointer, SIZE_T sizeMemorySize)
	{
		if (FALSE == WriteProcessMemory(hTargetProcess, pRemoteMemoryPointer, pLocalMemoryPointer, sizeMemorySize, &sizeMemorySize))
			return 0;

		return sizeMemorySize;
	}

	HANDLE CreateRemoteRoutine(HANDLE hTargetProcess, const void * pRoutineStartPointer, const void * pRoutineContext, SIZE_T sizeContextSize)
	{
		if (pRoutineContext && 0 != sizeContextSize)
		{

			if (NULL == pRoutineContext)
				return NULL;
		}

		return CreateRemoteThread(hTargetProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRoutineStartPointer, (void *)pRoutineContext, 0, NULL);
	}

}

namespace HookControl {

	bool InjectDll(DWORD dwProcessID, LPCTSTR pcszInjectFileFullPath)
	{
		SIZE_T sizeInjectContextSize = 0;
		INJECT_SHELLCODEINFO tisInjectShellcode = { 0 };
		void * uLoadLibraryAddr = (void *)LoadLibrary;
		static const unsigned char binShellcode[] = { 0xE9,0x00,0x00,0x00,0x00 };

		tisInjectShellcode.pShellCodePointer = binShellcode;
		tisInjectShellcode.pShellCodeDataPointer = &uLoadLibraryAddr;

		tisInjectShellcode.sizeShellCodeDataPos = 1;
		tisInjectShellcode.sizeShellCodeSize = sizeof(binShellcode);

		if (pcszInjectFileFullPath)
			sizeInjectContextSize = (_tcslen(pcszInjectFileFullPath) + 1) * sizeof(TCHAR);

		return InjectShellCode(dwProcessID, &tisInjectShellcode, pcszInjectFileFullPath, sizeInjectContextSize);
	}

	bool InjectDll(DWORD dwProcessID, LPCTSTR pcszInjectFileFullPath,HINSTANCE * phRemoteModule/* = NULL*/)
	{
		SIZE_T sizeInjectContextSize = 0;
		INJECT_SHELLCODEINFO tisInjectShellcode = { 0 };

		struct 
		{
			DWORD			dwError;
			HINSTANCE	hModule;
			PVOID				pfnLoadLibrary;
			TCHAR			szInjectDllFullPath[MAX_PATH + 1];
		} tsdShellCodeData;

		static const unsigned char binShellcode[] = {
			0x8B, 0xF4,												/* mov     esi, esp */
			0xE8, 0X00,0X00, 0X00,0X00, 				/* call    $+5 */
			0X5D,															/* pop     ebp */
			0X81, 0XC5, 0X1D ,0X00, 0X00,0X00,	/* add     ebp, 1Dh */
			0X8D, 0X4D,0X0C,									/* lea     ecx, [ebp+0Ch] */
			0x51, 															/* push    ecx */
			0XFF, 0X55,0X08, 										/* call    dword ptr [ebp+8] */
			0X89, 0X45,0X04, 									/* mov     [ebp+4], eax */
			0X64, 0XA1, 0X34,0X00, 0X00,0X00,	/* mov     eax, large fs:34h */
			0X89, 0X45,0X00, 									/* mov     [ebp+0], eax */
			0X8B, 0XE6, 												/* mov     esp, esi */
			0xC3, 															/* retn */
		};

		tisInjectShellcode.pShellCodePointer = binShellcode;
		tisInjectShellcode.pShellCodeDataPointer = &tsdShellCodeData;

		tisInjectShellcode.sizeShellCodeDataPos = sizeof(binShellcode);
		tisInjectShellcode.sizeShellCodeSize = sizeof(binShellcode) + sizeof(tsdShellCodeData);

		tsdShellCodeData.pfnLoadLibrary = (void *)LoadLibrary;
		_tcscpy(tsdShellCodeData.szInjectDllFullPath, pcszInjectFileFullPath);

		if (false == InjectShellCode(dwProcessID, &tisInjectShellcode))
			return false;

		if (phRemoteModule)
			*phRemoteModule = tsdShellCodeData.hModule;

		return true;
	}

};

namespace HookControl {

	bool InjectShellCode(DWORD dwTargetProcessID, PINJECT_SHELLCODEINFO pInjectShellcode, const void * pRoutineContext/* = NULL*/, SIZE_T sizeContextSize/* = 0*/)
	{
		HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | SYNCHRONIZE, FALSE, dwTargetProcessID);

		if (NULL == hProcess)
			return false;

		return InjectShellCode(hProcess, pInjectShellcode);
	}

	bool InjectShellCode(HANDLE hTargetProcess, PINJECT_SHELLCODEINFO pInjectShellcode, const void * pRoutineContext/* = NULL*/, SIZE_T sizeContextSize/* = 0*/)
	{
		bool bIsOK = false;
		HANDLE hRemoteThread = NULL;
		SIZE_T sizeShellCodeDatasize = 0;

		void * pRemoteShellCode = NULL;
		void * pRemoteRoutineContext = NULL;

		sizeShellCodeDatasize = pInjectShellcode->sizeShellCodeSize - pInjectShellcode->sizeShellCodeDataPos;
		do 
		{
			if (NULL == (pRemoteShellCode = AllocRemoteMemory(hTargetProcess, pInjectShellcode->sizeShellCodeSize)))
				break;

			if (pInjectShellcode->sizeShellCodeSize != WriteRemoteMemory(hTargetProcess, pRemoteShellCode, pInjectShellcode->pShellCodePointer, pInjectShellcode->sizeShellCodeSize))
				break;

			if (pInjectShellcode->sizeShellCodeSize != WriteRemoteMemory(hTargetProcess, pRemoteShellCode, pInjectShellcode->pShellCodeDataPointer, sizeShellCodeDatasize))
				break;

			if (pRoutineContext && 0 != sizeContextSize)
			{
				if (NULL == (pRemoteRoutineContext = AllocRemoteMemory(hTargetProcess, sizeContextSize)))
					break;

				if (sizeContextSize != WriteRemoteMemory(hTargetProcess, pRemoteRoutineContext, pRoutineContext, sizeContextSize))
					break;
			}

			if (NULL == (hRemoteThread = CreateRemoteRoutine(hTargetProcess, pRemoteShellCode, pRemoteRoutineContext, sizeContextSize)))
				break;

			if (WAIT_OBJECT_0 != WaitForSingleObject(hRemoteThread, INFINITE))
				break;

			if (sizeShellCodeDatasize != ReadRemoteMemory(hTargetProcess, (void *)(ULONG_PTR(pRemoteShellCode) + pInjectShellcode->sizeShellCodeDataPos), pInjectShellcode->pShellCodeDataPointer, sizeShellCodeDatasize))
				break;

			bIsOK = true;
		} while (false);

		if (hRemoteThread)
		{
			::TerminateThread(hRemoteThread, -1);

			CloseHandle(hRemoteThread);
		}

		if (pRemoteShellCode)
			FreeRemoteMemory(hTargetProcess, pRemoteShellCode);

		if (pRemoteRoutineContext)
			FreeRemoteMemory(hTargetProcess, pRemoteRoutineContext);

		return bIsOK;
	}

};