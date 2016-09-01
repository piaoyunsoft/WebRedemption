#include <windows.h>
#include <tchar.h>
#include "FakeCreateDirectory.h"



namespace HookControl{

	__pfnCreateDirectoryA pfnCreateDirectoryA = NULL;
	__pfnCreateDirectoryW pfnCreateDirectoryW = NULL;

	BOOL WINAPI FakeCreateDirectoryA(_In_      LPCSTR lpPathName, _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes)
	{
		bool bIsCall = false;
		CHAR szFileName[MAX_PATH + 1] = { 0 };
		TCHAR tszFileName[MAX_PATH + 1] = { 0 };
		HOOKCONTROL_CREATEDIRECTORY hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, FALSE);

		if (IsPassCall(OnBeforeCreateDirectory, hciInfo.CallAddress))
			return pfnCreateDirectoryA(lpPathName, lpSecurityAttributes);

		HookHelp::STR2TSTR(lpPathName, tszFileName);

		bIsCall = OnBeforeCreateDirectory(&hciInfo, tszFileName, lpSecurityAttributes);

		if (bIsCall)
			hciInfo.RetValue = pfnCreateDirectoryA(HookHelp::TSTR2STR(tszFileName,szFileName), lpSecurityAttributes);

		bIsCall = bIsCall && OnAfterCreateDirectory(&hciInfo, tszFileName, lpSecurityAttributes);

		return hciInfo.RetValue;
	}

	BOOL WINAPI FakeCreateDirectoryW(_In_      LPCWSTR lpPathName, _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes)
	{
		bool bIsCall = false;
		WCHAR szFileName[MAX_PATH + 1] = { 0 };
		TCHAR tszFileName[MAX_PATH + 1] = { 0 };
		HOOKCONTROL_CREATEDIRECTORY hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, FALSE);

		if (IsPassCall(OnBeforeCreateDirectory, hciInfo.CallAddress))
			return pfnCreateDirectoryW(lpPathName, lpSecurityAttributes);

		HookHelp::WSTR2TSTR(lpPathName, tszFileName);

		bIsCall = OnBeforeCreateDirectory(&hciInfo, tszFileName, lpSecurityAttributes);

		if (bIsCall)
			hciInfo.RetValue = pfnCreateDirectoryW(HookHelp::TSTR2WSTR(tszFileName, szFileName), lpSecurityAttributes);

		bIsCall = bIsCall &&OnAfterCreateDirectory(&hciInfo, tszFileName, lpSecurityAttributes);

		return hciInfo.RetValue;
	}

	bool StartCreateDirectoryHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Kernel32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Kernel32.dll"));

		if (NULL == pfnCreateDirectoryA)
			InlineHook(GetProcAddress(hModule, "CreateDirectoryA"), FakeCreateDirectoryA, (void **)&pfnCreateDirectoryA);

		if (NULL == pfnCreateDirectoryW)
			InlineHook(GetProcAddress(hModule, "CreateDirectoryW"), FakeCreateDirectoryW, (void **)&pfnCreateDirectoryW);

		return (pfnCreateDirectoryA && pfnCreateDirectoryW);
	}

	void StopCreateDirectoryHook()
	{
		if (pfnCreateDirectoryA)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), "CreateDirectoryW"), pfnCreateDirectoryA);

		if (pfnCreateDirectoryW)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), "CreateDirectoryW"), pfnCreateDirectoryW);

		pfnCreateDirectoryA = NULL;
		pfnCreateDirectoryW = NULL;
	}
}