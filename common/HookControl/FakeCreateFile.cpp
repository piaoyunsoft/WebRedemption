#include <windows.h>
#include <tchar.h>
#include "FakeCreateFile.h"



namespace HookControl{

	__pfnCreateFileW pfnCreateFileW = NULL;

	HANDLE WINAPI FakeCreateFileW(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile)
	{
		bool bIsCall = false;
		WCHAR szFileName[MAX_PATH + 1] = { 0 };
		TCHAR tszFileName[MAX_PATH + 1] = { 0 };
		HOOKCONTROL_CREATEFILE hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeCreateFile, hciInfo.CallAddress))
			return pfnCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

		HookHelp::WSTR2TSTR(lpFileName, tszFileName);

		bIsCall = OnBeforeCreateFile(&hciInfo, tszFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

		if (bIsCall)
			hciInfo.RetValue = pfnCreateFileW(HookHelp::TSTR2WSTR(tszFileName, szFileName), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

		bIsCall = bIsCall &&OnAfterCreateFile(&hciInfo, tszFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

		return hciInfo.RetValue;
	}

	bool StartCreateFileHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Kernel32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Kernel32.dll"));

		if (pfnCreateFileW)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "CreateFileW"), FakeCreateFileW, (void **)&pfnCreateFileW);
	}

	void StopCreateFileHook()
	{
		if (pfnCreateFileW)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), "CreateFileW"), pfnCreateFileW);

		pfnCreateFileW = NULL;
	}
}