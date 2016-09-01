#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_CREATEWINDOWEX
	{
		void * CallAddress;
		HWND RetValue;
	}HOOKCONTROL_CREATEWINDOWEX, *PHOOKCONTROL_CREATEWINDOWEX;

	typedef HWND(WINAPI * __pfnCreateWindowExW)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);

	extern __pfnCreateWindowExW pfnCreateWindowExW;

	bool OnBeforeCreateWindowEx(HOOKCONTROL_CREATEWINDOWEX * retStatuse, DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	bool OnAfterCreateWindowEx(HOOKCONTROL_CREATEWINDOWEX * retStatuse, DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

	void StopCreateWindowExHook();
	bool StartCreateWindowExHook();
}
