#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_NTSETVALUEKEY
	{
		void * CallAddress;
		NTSTATUS RetValue;
	}HOOKCONTROL_NTSETVALUEKEY, *PHOOKCONTROL_NTSETVALUEKEY;

	typedef NTSTATUS(WINAPI *__pfnNtSetValueKey)(HANDLE, PUNICODE_STRING, ULONG, ULONG, PVOID, ULONG);

	extern __pfnNtSetValueKey pfnNtSetValueKey;

	bool OnBeforeNtSetValueKey(HOOKCONTROL_NTSETVALUEKEY * retStatuse, HANDLE KeyHandle, PUNICODE_STRING ValueName, ULONG TitleIndex, ULONG Type, PVOID Data, ULONG DataSize);
	bool OnAfterNtSetValueKey(HOOKCONTROL_NTSETVALUEKEY * retStatuse, HANDLE KeyHandle, PUNICODE_STRING ValueName, ULONG TitleIndex, ULONG Type, PVOID Data, ULONG DataSize);

	void StopNtSetValueKeyHook();
	bool StartNtSetValueKeyHook();
}
