#include <windows.h>
#include <tchar.h>
#include "FakeGetPrivateProfileInt.h"



namespace HookControl{

	__pfnGetPrivateProfileInt pfnGetPrivateProfileInt = NULL;

	BOOL NTAPI FakeGetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
	{
		bool bIsCall = false;
		HOOKCONTROL_GETPRIVATEPROFILEINT hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeGetPrivateProfileInt, hciInfo.CallAddress))
			return pfnGetPrivateProfileInt(lpAppName, lpKeyName, nDefault, lpFileName);

		bIsCall = OnBeforeGetPrivateProfileInt(&hciInfo, lpAppName, lpKeyName, nDefault, lpFileName);

		if (bIsCall)
			hciInfo.RetValue = pfnGetPrivateProfileInt(lpAppName, lpKeyName, nDefault, lpFileName);

		bIsCall = bIsCall && OnAfterGetPrivateProfileInt(&hciInfo, lpAppName, lpKeyName, nDefault, lpFileName);

		return hciInfo.RetValue;
	}

	bool StartGetPrivateProfileIntHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Kernel32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Kernel32.dll"));

		if (pfnGetPrivateProfileInt)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "GetPrivateProfileIntW"), FakeGetPrivateProfileInt, (void **)&pfnGetPrivateProfileInt);
	}

	void StopGetPrivateProfileIntHook()
	{
		if (pfnGetPrivateProfileInt)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), "GetPrivateProfileIntW"), pfnGetPrivateProfileInt);

		pfnGetPrivateProfileInt = NULL;
	}
}