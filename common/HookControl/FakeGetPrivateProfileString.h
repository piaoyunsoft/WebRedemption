#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_GETPRIVATEPROFILESTRINGW
	{
		void * CallAddress;
		DWORD RetValue;
	}HOOKCONTROL_GETPRIVATEPROFILESTRINGW, *PHOOKCONTROL_GETPRIVATEPROFILESTRINGW;

	typedef DWORD(WINAPI * __pfnGetPrivateProfileStringW)(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR nDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName);
	
	extern __pfnGetPrivateProfileStringW pfnGetPrivateProfileStringW;

	bool OnBeforeGetPrivateProfileStringW(HOOKCONTROL_GETPRIVATEPROFILESTRINGW * retStatuse, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName);
	bool OnAfterGetPrivateProfileStringW(HOOKCONTROL_GETPRIVATEPROFILESTRINGW * retStatuse, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName);

	void StopGetPrivateProfileStringWHook();
	bool StartGetPrivateProfileStringWHook();
}
