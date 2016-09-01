#include <windows.h>
#include <tchar.h>
#include "FakeLdrLoadDll.h"

namespace HookControl{

	__pfnLdrLoadDll pfnLdrLoadDll = NULL;

	NTSTATUS WINAPI FakeLdrLoadDll(IN PWCHAR PathToFile OPTIONAL, IN ULONG Flags OPTIONAL, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle)
	{
		bool bIsCall = false;
		HOOKCONTROL_LDRLOADDLL hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, STATUS_SUCCESS);

		if (IsPassCall(OnBeforeLdrLoadDll, hciInfo.CallAddress))
			return pfnLdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle);

		bIsCall = OnBeforeLdrLoadDll(&hciInfo, PathToFile, Flags, ModuleFileName, ModuleHandle);

		if (bIsCall)
			hciInfo.RetValue = pfnLdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle);

		bIsCall = bIsCall && OnAfterLdrLoadDll(&hciInfo, PathToFile, Flags, ModuleFileName, ModuleHandle);

		return hciInfo.RetValue;
	}

	bool StartLdrLoadDllHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("ntdll.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("ntdll.dll"));

		if (pfnLdrLoadDll)
			return true;

		return InlineHook(GetProcAddress(hModule, "LdrLoadDll"), FakeLdrLoadDll, (void **)&pfnLdrLoadDll);
	}

	void StopLdrLoadDllHook()
	{
		if (pfnLdrLoadDll)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "LdrLoadDll"), pfnLdrLoadDll);

		pfnLdrLoadDll = NULL;
	}
}