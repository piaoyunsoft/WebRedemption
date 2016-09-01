#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_CREATEDIRECTORY
	{
		void * CallAddress;
		BOOL RetValue;
	}HOOKCONTROL_CREATEDIRECTORY, *PHOOKCONTROL_CREATEDIRECTORY;


	typedef BOOL(WINAPI * __pfnCreateDirectoryA) (LPCSTR, LPSECURITY_ATTRIBUTES);
	typedef BOOL(WINAPI * __pfnCreateDirectoryW) (LPCWSTR, LPSECURITY_ATTRIBUTES);

	extern __pfnCreateDirectoryA pfnCreateDirectoryA;
	extern __pfnCreateDirectoryW pfnCreateDirectoryW;

	bool OnBeforeCreateDirectory(HOOKCONTROL_CREATEDIRECTORY * retStatuse, LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
	bool OnAfterCreateDirectory(HOOKCONTROL_CREATEDIRECTORY * retStatuse, LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

	void StopCreateDirectoryHook();
	bool StartCreateDirectoryHook();
}
