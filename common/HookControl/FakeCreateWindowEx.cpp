#include <windows.h>
#include <tchar.h>
#include "FakeCreateWindowEx.h"



namespace HookControl{

	__pfnCreateWindowExW pfnCreateWindowExW = NULL;

	HWND NTAPI FakeCreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		bool bIsCall = false;
		WCHAR szClassName[MAX_PATH + 1] = { 0 };
		WCHAR szWindowName[MAX_PATH + 1] = { 0 };
		HOOKCONTROL_CREATEWINDOWEX hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeCreateWindowEx, hciInfo.CallAddress))
			return pfnCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

		if (ULONG(lpClassName) > 0x00010000L)
		{
			HookHelp::WSTR2TSTR(lpClassName, (TCHAR *)szClassName);
			HookHelp::WSTR2TSTR(lpWindowName, (TCHAR *)szWindowName);
		}

		bIsCall = OnBeforeCreateWindowEx(&hciInfo, dwExStyle, (ULONG(lpClassName) > 0x00010000L) ? (const TCHAR *)szClassName : (const TCHAR *)lpClassName, (TCHAR *)szWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

		if (bIsCall)
			hciInfo.RetValue = pfnCreateWindowExW(dwExStyle, (ULONG(lpClassName) > 0x00010000L) ? HookHelp::TSTR2WSTR((TCHAR *)szClassName, szClassName) : lpClassName, HookHelp::TSTR2WSTR((TCHAR *)lpWindowName, szWindowName), dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

		bIsCall = bIsCall && OnAfterCreateWindowEx(&hciInfo, dwExStyle, (ULONG(lpClassName) > 0x00010000L) ? (const TCHAR *)szClassName : (const TCHAR *)lpClassName, (TCHAR *)szWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

		return hciInfo.RetValue;
	}

	bool StartCreateWindowExHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("User32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("User32.dll"));

		if (pfnCreateWindowExW)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "CreateWindowExW"), FakeCreateWindowExW, (void **)&pfnCreateWindowExW);
	}

	void StopCreateWindowExHook()
	{
		if (pfnCreateWindowExW)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("User32.dll")), "CreateWindowExW"), pfnCreateWindowExW);

		pfnCreateWindowExW = NULL;
	}
}