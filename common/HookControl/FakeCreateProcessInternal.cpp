#include <windows.h>
#include <tchar.h>
#include "FakeCreateProcessInternal.h"


namespace HookControl{

	__pfnCreateProcessInternalW pfnCreateProcessInternal = NULL;



	BOOL WINAPI FakeCreateProcessInternalW(HANDLE hToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, PHANDLE hNewToken)
	{
		bool bIsCall = false;
		HOOKCONTROL_CREATEPROCESSINTERNAL hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, STATUS_SUCCESS);

		if (IsPassCall(OnBeforeCreateProcessInternal, hciInfo.CallAddress))
			return pfnCreateProcessInternal(hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, hNewToken);

		bIsCall = OnBeforeCreateProcessInternal(&hciInfo, hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, hNewToken);

		if (bIsCall)
			hciInfo.RetValue = pfnCreateProcessInternal(hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, hNewToken);

		bIsCall = bIsCall && OnAfterCreateProcessInternal(&hciInfo, hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, hNewToken);

		return hciInfo.RetValue;
	}

	bool StartCreateProcessInternalHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Kernel32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Kernel32.dll"));

		if (pfnCreateProcessInternal)
			return true;

		return InlineHook(GetProcAddress(hModule, "CreateProcessInternalW"), FakeCreateProcessInternalW, (void **)&pfnCreateProcessInternal);
	}

	void StopCreateProcessInternalHook()
	{
		if (pfnCreateProcessInternal)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Kernel32.dll")), "CreateProcessInternalW"), pfnCreateProcessInternal);

		pfnCreateProcessInternal = NULL;
	}
}