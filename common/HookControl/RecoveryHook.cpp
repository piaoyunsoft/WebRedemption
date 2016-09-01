#include <windows.h>
#include "RecoveryHook.h"
#include "HookHelp.h"
#include "MemoryModule.h"
#include <tchar.h>
#include "InlineHook.h"

void * FileToMemory(const tchar * pszFileName)
{
	char * pBuffer = NULL;
	void * pRetValue = NULL;
	FILE * hFileHandle = NULL;
	//fputs("File error", stderr);

	do 
	{
		/* 若要一个byte不漏地读入整个文件，只能采用二进制方式打开 */
		hFileHandle = _tfopen(pszFileName, _T("rb"));

		if (hFileHandle == NULL)
			break;

		fseek(hFileHandle, 0, SEEK_END);
		unsigned long uBufferSize = ftell(hFileHandle);
		fseek(hFileHandle,0,SEEK_SET);

		pBuffer = (char*)malloc(sizeof(char)*uBufferSize);

		if (pBuffer == NULL)
			break;

		if (0 == fread(pBuffer, uBufferSize, 1, hFileHandle))
			break;

		pRetValue = pBuffer;
	} while (false);

	if (hFileHandle)
		fclose(hFileHandle);

	if (pBuffer && NULL == pRetValue)
		free(pBuffer);

	return pRetValue;
}

bool RecoveryHook(void * pProcAddr)
{
	return RecoveryInlineHook(pProcAddr);
}

bool RecoveryAPI(const tchar * pszDllName, const char * pszFunctionName)
{
	bool bIsOK = false;
	HMEMORYMODULE hMemModule = NULL;
	TCHAR szInstancePath[MAX_PATH + 1] = { 0 };

	if (::GetModuleFileName(GetModuleHandle(pszDllName), szInstancePath, MAX_PATH))
	{
		void * pFileConnext = FileToMemory(szInstancePath);
		MemoryLoadLibraryToAddr(pFileConnext, (unsigned char *)GetModuleHandle(szInstancePath), &hMemModule);

		if (NULL != pFileConnext)
		{
			void * pOldProc = GetProcAddress(GetModuleHandle(szInstancePath), pszFunctionName);
			void * pBakProc = MemoryGetProcAddress(hMemModule, pszFunctionName);

			if (pOldProc && pBakProc)
				bIsOK = TRUE == HookControl::WriteReadOnlyMemory((LPBYTE)pOldProc, (LPBYTE)pBakProc, SIZE_RECOVERY);
		}

		MemoryFreeLibrary(hMemModule);

 		if (pFileConnext)
			free(pFileConnext);
	}



	return bIsOK;
}


bool RecoveryMem(const tchar * pszDllName,unsigned long uBaseAddrPos,unsigned long uMemLength)
{
	bool bIsOK = false;
	HMEMORYMODULE hMemModule = NULL;
	TCHAR szInstancePath[MAX_PATH + 1] = { 0 };

	if (::GetModuleFileName(GetModuleHandle(pszDllName), szInstancePath, MAX_PATH))
	{
		void * pFileConnext = FileToMemory(szInstancePath);
		unsigned char *pModuleAddr = (unsigned char *)GetModuleHandle(szInstancePath);

		unsigned char * pMemModuleAddr = MemoryLoadLibraryToAddr(pFileConnext, (unsigned char *)pModuleAddr, &hMemModule);

		if (NULL != pFileConnext)
		{
			void * pOldProc = pModuleAddr + uBaseAddrPos;
			void * pBakProc = pMemModuleAddr + uBaseAddrPos;

			if (pOldProc && pBakProc)
				bIsOK = TRUE == HookControl::WriteReadOnlyMemory((LPBYTE)pOldProc, (LPBYTE)pBakProc, uMemLength);
		}

		MemoryFreeLibrary(hMemModule);

		if (pFileConnext)
			free(pFileConnext);
	}



	return bIsOK;
}

bool RecoveryInlineHook(void * pProcAddr)
{
	bool bIsOK = false;
	TCHAR szInstancePath[MAX_PATH + 1] = { 0 };
	HINSTANCE hInstance = Common::GetModuleHandleByAddr(pProcAddr);

	if (NULL != hInstance && ::GetModuleFileName(hInstance, szInstancePath, MAX_PATH))
		bIsOK = RecoveryInlineHook(szInstancePath, pProcAddr);

	return bIsOK;
}

bool RecoveryInlineHook(const tchar * pszDllName, void * pProcAddr)
{
	bool bIsOK = false;
	HINSTANCE hInstance = GetModuleHandle(pszDllName);

	if (NULL == hInstance)
		return bIsOK;

	return RecoveryMem(pszDllName, (unsigned long)pProcAddr - (unsigned long)hInstance, SIZE_RECOVERY);
}