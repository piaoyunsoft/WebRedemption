#include <windows.h>
#include <tchar.h>
#include "FakeNtSetValueKey.h"


namespace HookControl{

	__pfnNtSetValueKey pfnNtSetValueKey = NULL;



	NTSTATUS WINAPI FakeNtSetValueKey(HANDLE KeyHandle, PUNICODE_STRING ValueName, ULONG TitleIndex, ULONG Type, PVOID Data, ULONG DataSize)
	{
		bool bIsCall = false;
		HOOKCONTROL_NTSETVALUEKEY hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, STATUS_SUCCESS);

		if (IsPassCall(OnBeforeNtSetValueKey, hciInfo.CallAddress))
			return pfnNtSetValueKey(KeyHandle, ValueName, TitleIndex, Type, Data, DataSize);

		bIsCall = OnBeforeNtSetValueKey(&hciInfo, KeyHandle, ValueName, TitleIndex, Type, Data, DataSize);

		if (bIsCall)
			hciInfo.RetValue = pfnNtSetValueKey(KeyHandle, ValueName, TitleIndex, Type, Data, DataSize);

		bIsCall = bIsCall && OnAfterNtSetValueKey(&hciInfo, KeyHandle, ValueName, TitleIndex, Type, Data, DataSize);

		return hciInfo.RetValue;
	}

	bool StartNtSetValueKeyHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("ntdll.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("ntdll.dll"));

		if (pfnNtSetValueKey)
			return true;

		return InlineHook(GetProcAddress(hModule, "NtSetValueKey"), FakeNtSetValueKey, (void **)&pfnNtSetValueKey);
	}

	void StopNtSetValueKeyHook()
	{
		if (pfnNtSetValueKey)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtSetValueKey"), pfnNtSetValueKey);

		pfnNtSetValueKey = NULL;
	}
}