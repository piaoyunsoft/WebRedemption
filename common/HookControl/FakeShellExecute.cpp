#include <windows.h>
#include <tchar.h>
#include "FakeShellExecute.h"


namespace HookControl{

	__pfnShellExecuteW pfnShellExecuteW = NULL;



	HINSTANCE WINAPI FakeShellExecuteW(_In_opt_  HWND hwnd, _In_opt_  LPCTSTR lpOperation, _In_      LPCTSTR lpFile, _In_opt_  LPCTSTR lpParameters, _In_opt_  LPCTSTR lpDirectory, _In_      INT nShowCmd)
	{
		bool bIsCall = false;
		HOOKCONTROL_SHELLEXECUTE hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, STATUS_SUCCESS);

		if (IsPassCall(OnBeforeShellExecute, hciInfo.CallAddress))
			return pfnShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);

		bIsCall = OnBeforeShellExecute(&hciInfo, hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);

		if (bIsCall)
			hciInfo.RetValue = pfnShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);

		bIsCall = bIsCall && OnAfterShellExecute(&hciInfo, hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);

		return hciInfo.RetValue;
	}

	bool StartShellExecuteHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Shell32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Shell32.dll"));

		if (pfnShellExecuteW)
			return true;

		return InlineHook(GetProcAddress(hModule, "ShellExecuteW"), FakeShellExecuteW, (void **)&pfnShellExecuteW);
	}

	void StopShellExecuteHook()
	{
		if (pfnShellExecuteW)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Shell32.dll")), "ShellExecuteW"), pfnShellExecuteW);

		pfnShellExecuteW = NULL;
	}
}