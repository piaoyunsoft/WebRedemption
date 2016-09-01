#include <windows.h>
#include <tchar.h>
#include "FakeNtOpenProcess.h"



namespace HookControl{

	__pfnNtOpenProcess pfnNtOpenProcess = NULL;

	NTSTATUS WINAPI FakeNtOpenProcess(OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId OPTIONAL)
	{
		bool bIsCall = false;
		HOOKCONTROL_NTOPENPROCESS hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeNtOpenProcess, hciInfo.CallAddress))
			return pfnNtOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);

		bIsCall = OnBeforeNtOpenProcess(&hciInfo, ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);

		if (bIsCall)
			hciInfo.RetValue = pfnNtOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);

		bIsCall = bIsCall && OnAfterNtOpenProcess(&hciInfo, ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);

		return hciInfo.RetValue;
	}

	bool StartNtOpenProcessHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Ntdll.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Ntdll.dll"));

		if (pfnNtOpenProcess)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "NtOpenProcess"), FakeNtOpenProcess, (void **)&pfnNtOpenProcess);
	}

	void StopNtOpenProcessHook()
	{
		if (pfnNtOpenProcess)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Ntdll.dll")), "NtOpenProcess"), pfnNtOpenProcess);

		pfnNtOpenProcess = NULL;
	}
}