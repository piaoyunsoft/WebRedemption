#include <windows.h>
#include <tchar.h>
#include "FakeWritePrivateProfileString.h"



namespace HookControl{

	__pfnWritePrivateProfileStringW pfnWritePrivateProfileStringW = NULL;

	BOOL NTAPI FakeWritePrivateProfileStringW(_In_  LPCWSTR lpAppName, _In_  LPCWSTR lpKeyName, _In_  LPCWSTR lpString, _In_  LPCWSTR lpFileName)
	{
		bool bIsCall = false;
		HOOKCONTROL_WRITEPRIVATEPROFILESTRING hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeWritePrivateProfileStringW, hciInfo.CallAddress))
			return pfnWritePrivateProfileStringW(lpAppName, lpKeyName, lpString, lpFileName);

		bIsCall = OnBeforeWritePrivateProfileStringW(&hciInfo, lpAppName, lpKeyName, lpString, lpFileName);

		if (bIsCall)
			hciInfo.RetValue = pfnWritePrivateProfileStringW(lpAppName, lpKeyName, lpString, lpFileName);

		bIsCall = bIsCall && OnAfterWritePrivateProfileStringW(&hciInfo, lpAppName, lpKeyName, lpString, lpFileName);

		return hciInfo.RetValue;
	}

	bool StartWritePrivateProfileStringHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Kernel32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Kernel32.dll"));

		if (pfnWritePrivateProfileStringW)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "WritePrivateProfileStringW"), FakeWritePrivateProfileStringW, (void **)&pfnWritePrivateProfileStringW);
	}

	void StopWritePrivateProfileStringHook()
	{
		if (pfnWritePrivateProfileStringW)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), "WritePrivateProfileStringW"), pfnWritePrivateProfileStringW);

		pfnWritePrivateProfileStringW = NULL;
	}
}