#include <windows.h>
#include <tchar.h>
#include "FakeShowWindow.h"



namespace HookControl{

	__pfnShowWindow pfnShowWindow = NULL;

	BOOL NTAPI FakeShowWindow(HWND hWnd, int nCmdShow)
	{
		bool bIsCall = false;
		HOOKCONTROL_SHOWWINDOW hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeShowWindow, hciInfo.CallAddress))
			return pfnShowWindow(hWnd, nCmdShow);

		bIsCall = OnBeforeShowWindow(&hciInfo, hWnd,nCmdShow);

		if (bIsCall)
			hciInfo.RetValue = pfnShowWindow( hWnd, nCmdShow);

		bIsCall = bIsCall && OnAfterShowWindow(&hciInfo, hWnd, nCmdShow);

		return hciInfo.RetValue;
	}

	bool StartShowWindowHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("User32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("User32.dll"));

		if (pfnShowWindow)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "ShowWindow"), FakeShowWindow, (void **)&pfnShowWindow);
	}

	void StopShowWindowHook()
	{
		if (pfnShowWindow)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("User32.dll")), "ShowWindow"), pfnShowWindow);

		pfnShowWindow = NULL;
	}
}