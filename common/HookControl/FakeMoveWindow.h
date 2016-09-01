#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_MOVEWINDOW
	{
		void * CallAddress;
		BOOL RetValue;
	}HOOKCONTROL_MOVEWINDOW, *PHOOKCONTROL_MOVEWINDOW;

	typedef BOOL(WINAPI * __pfnMoveWindow)(HWND hWnd,int X,int Y,int nWidth,int nHeight,BOOL bRepaint);
	
	extern __pfnMoveWindow pfnMoveWindow;

	bool OnBeforeMoveWindow(HOOKCONTROL_MOVEWINDOW * retStatuse, HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
	bool OnAfterMoveWindow(HOOKCONTROL_MOVEWINDOW * retStatuse, HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);

	void StopMoveWindowHook();
	bool StartMoveWindowHook();
}
