#include <windows.h>
#include <tchar.h>
#include "FakePostMessage.h"



namespace HookControl{

	__pfnPostMessage pfnPostMessage = NULL;

	BOOL NTAPI FakePostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		bool bIsCall = false;
		HOOKCONTROL_POSTMESSAGE hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforePostMessage, hciInfo.CallAddress))
			return pfnPostMessage(hWnd, uMsg, wParam, lParam);

		bIsCall = OnBeforePostMessage(&hciInfo, hWnd, uMsg, wParam, lParam);

		if (bIsCall)
			hciInfo.RetValue = pfnPostMessage(hWnd, uMsg, wParam, lParam);

		bIsCall = bIsCall && OnAfterPostMessage(&hciInfo, hWnd, uMsg, wParam, lParam);

		return hciInfo.RetValue;
	}

	bool StartPostMessageHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("User32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("User32.dll"));

		if (pfnPostMessage)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "PostMessageW"), FakePostMessage, (void **)&pfnPostMessage);
	}

	void StopPostMessageHook()
	{
		if (pfnPostMessage)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("User32.dll")), "PostMessageW"), pfnPostMessage);

		pfnPostMessage = NULL;
	}
}