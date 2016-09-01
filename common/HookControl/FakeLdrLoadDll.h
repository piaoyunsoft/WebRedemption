#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_LDRLOADDLL
	{
		void * CallAddress;
		NTSTATUS RetValue;
	}HOOKCONTROL_LDRLOADDLL, *PHOOKCONTROL_LDRLOADDLL;

	typedef NTSTATUS(WINAPI *__pfnLdrLoadDll) (PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);

	extern __pfnLdrLoadDll pfnLdrLoadDll;

	bool OnBeforeLdrLoadDll(IN OUT HOOKCONTROL_LDRLOADDLL * retStatuse, IN PWCHAR PathToFile OPTIONAL, IN ULONG Flags OPTIONAL, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle);
	bool OnAfterLdrLoadDll(IN OUT HOOKCONTROL_LDRLOADDLL * retStatuse, IN PWCHAR PathToFile OPTIONAL, IN ULONG Flags OPTIONAL, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle);

	void StopLdrLoadDllHook();
	bool StartLdrLoadDllHook();
}
