#include <windows.h>
#include <tchar.h>
#include "FakeGetPrivateProfileString.h"



namespace HookControl{

	__pfnGetPrivateProfileStringW pfnGetPrivateProfileStringW = NULL;

	BOOL NTAPI FakeGetPrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
	{
		bool bIsCall = false;
		HOOKCONTROL_GETPRIVATEPROFILESTRINGW hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeGetPrivateProfileStringW, hciInfo.CallAddress))
			return pfnGetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);

		bIsCall = OnBeforeGetPrivateProfileStringW(&hciInfo, lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);

		if (bIsCall)
			hciInfo.RetValue = pfnGetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);

		bIsCall = bIsCall && OnAfterGetPrivateProfileStringW(&hciInfo, lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);

		return hciInfo.RetValue;
	}

	bool StartGetPrivateProfileStringWHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Kernel32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Kernel32.dll"));

		if (pfnGetPrivateProfileStringW)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "GetPrivateProfileStringW"), FakeGetPrivateProfileStringW, (void **)&pfnGetPrivateProfileStringW);
	}

	void StopGetPrivateProfileStringWHook()
	{
		if (pfnGetPrivateProfileStringW)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), "GetPrivateProfileStringW"), pfnGetPrivateProfileStringW);

		pfnGetPrivateProfileStringW = NULL;
	}
}