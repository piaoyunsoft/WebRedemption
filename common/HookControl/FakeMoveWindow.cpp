#include <windows.h>
#include <tchar.h>
#include "FakeMoveWindow.h"



namespace HookControl{

	__pfnMoveWindow pfnMoveWindow = NULL;

	BOOL NTAPI FakeMoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
	{
		bool bIsCall = false;
		HOOKCONTROL_MOVEWINDOW hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeMoveWindow, hciInfo.CallAddress))
			return pfnMoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);

		bIsCall = OnBeforeMoveWindow(&hciInfo, hWnd, X, Y, nWidth, nHeight, bRepaint);

		if (bIsCall)
			hciInfo.RetValue = pfnMoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);

		bIsCall = bIsCall && OnAfterMoveWindow(&hciInfo, hWnd, X, Y, nWidth, nHeight, bRepaint);

		return hciInfo.RetValue;
	}

	bool StartMoveWindowHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("User32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("User32.dll"));

		if (pfnMoveWindow)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "MoveWindow"), FakeMoveWindow, (void **)&pfnMoveWindow);
	}

	void StopMoveWindowHook()
	{
		if (pfnMoveWindow)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("User32.dll")), "MoveWindow"), pfnMoveWindow);

		pfnMoveWindow = NULL;
	}
}