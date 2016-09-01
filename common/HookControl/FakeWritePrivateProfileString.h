#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_WRITEPRIVATEPROFILESTRING
	{
		void * CallAddress;
		BOOL RetValue;
	}HOOKCONTROL_WRITEPRIVATEPROFILESTRING, *PHOOKCONTROL_WRITEPRIVATEPROFILESTRING;

	typedef BOOL(WINAPI * __pfnWritePrivateProfileStringA)(_In_  LPCSTR lpAppName, _In_  LPCSTR lpKeyName, _In_  LPCSTR lpString, _In_  LPCSTR lpFileName);
	typedef BOOL(WINAPI * __pfnWritePrivateProfileStringW)(_In_  LPCWSTR lpAppName, _In_  LPCWSTR lpKeyName, _In_  LPCWSTR lpString, _In_  LPCWSTR lpFileName);
	
	extern __pfnWritePrivateProfileStringW pfnWritePrivateProfileStringW;

	bool OnBeforeWritePrivateProfileStringW(HOOKCONTROL_WRITEPRIVATEPROFILESTRING * retStatuse, _In_  LPCWSTR lpAppName, _In_  LPCWSTR lpKeyName, _In_  LPCWSTR lpString, _In_  LPCWSTR lpFileName);
	bool OnAfterWritePrivateProfileStringW(HOOKCONTROL_WRITEPRIVATEPROFILESTRING * retStatuse, _In_  LPCWSTR lpAppName, _In_  LPCWSTR lpKeyName, _In_  LPCWSTR lpString, _In_  LPCWSTR lpFileName);

	void StopWritePrivateProfileStringHook();
	bool StartWritePrivateProfileStringHook();
}
