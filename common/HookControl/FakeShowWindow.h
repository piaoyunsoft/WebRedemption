#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_SHOWWINDOW
	{
		void * CallAddress;
		BOOL RetValue;
	}HOOKCONTROL_SHOWWINDOW, *PHOOKCONTROL_SHOWWINDOW;

	typedef BOOL(WINAPI * __pfnShowWindow)(HWND, int);
	
	extern __pfnShowWindow pfnShowWindow;

	bool OnBeforeShowWindow(HOOKCONTROL_SHOWWINDOW * retStatuse, HWND hWnd, int nCmdShow);
	bool OnAfterShowWindow(HOOKCONTROL_SHOWWINDOW * retStatuse, HWND hWnd, int nCmdShow);

	void StopShowWindowHook();
	bool StartShowWindowHook();
}
