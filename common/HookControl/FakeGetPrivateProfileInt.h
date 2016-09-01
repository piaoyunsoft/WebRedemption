#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_GETPRIVATEPROFILEINT
	{
		void * CallAddress;
		UINT RetValue;
	}HOOKCONTROL_GETPRIVATEPROFILEINT, *PHOOKCONTROL_GETPRIVATEPROFILEINT;

	typedef UINT(WINAPI * __pfnGetPrivateProfileInt)(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName);
	
	extern __pfnGetPrivateProfileInt pfnGetPrivateProfileInt;

	bool OnBeforeGetPrivateProfileInt(HOOKCONTROL_GETPRIVATEPROFILEINT * retStatuse, LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName);
	bool OnAfterGetPrivateProfileInt(HOOKCONTROL_GETPRIVATEPROFILEINT * retStatuse, LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName);

	void StopGetPrivateProfileIntHook();
	bool StartGetPrivateProfileIntHook();
}
