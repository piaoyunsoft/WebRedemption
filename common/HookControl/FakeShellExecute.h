#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_SHELLEXECUTE
	{
		void * CallAddress;
		HINSTANCE RetValue;
	}HOOKCONTROL_SHELLEXECUTE, *PHOOKCONTROL_SHELLEXECUTE;

	typedef HINSTANCE(WINAPI *__pfnShellExecuteW) (HWND, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, INT);

	extern __pfnShellExecuteW pfnShellExecuteW;

	bool OnBeforeShellExecute(HOOKCONTROL_SHELLEXECUTE * retStatuse, HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd);
	bool OnAfterShellExecute(HOOKCONTROL_SHELLEXECUTE * retStatuse, HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd);

	void StopShellExecuteHook();
	bool StartShellExecuteHook();
}
