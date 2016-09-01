#include <windows.h>
#include <tchar.h>
#include "FakeNtCreatePort.h"


namespace HookControl{

	__pfnNtCreatePort pfnNtCreatePort = NULL;

	NTSTATUS WINAPI FakeNtCreatePort(OUT PHANDLE PortHandle, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG MaxConnectionInfoLength, IN ULONG MaxMessageLength, IN ULONG MaxPoolUsage)
	{
		bool bIsCall = false;
		HOOKCONTROL_NTCREATEPORT hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, STATUS_SUCCESS);

		if (IsPassCall(OnBeforeNtCreatePort, hciInfo.CallAddress))
			return pfnNtCreatePort(PortHandle, ObjectAttributes, MaxConnectionInfoLength, MaxMessageLength, MaxPoolUsage);

		bIsCall = OnBeforeNtCreatePort(&hciInfo, PortHandle, ObjectAttributes, MaxConnectionInfoLength, MaxMessageLength, MaxPoolUsage);

		if (bIsCall)
			hciInfo.RetValue = pfnNtCreatePort(PortHandle, ObjectAttributes, MaxConnectionInfoLength, MaxMessageLength, MaxPoolUsage);

		bIsCall = bIsCall && OnAfterNtCreatePort(&hciInfo, PortHandle, ObjectAttributes, MaxConnectionInfoLength, MaxMessageLength, MaxPoolUsage);

		return hciInfo.RetValue;
	}

	bool StartNtCreatePortHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("ntdll.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("ntdll.dll"));

		if (pfnNtCreatePort)
			return true;

		return InlineHook(GetProcAddress(hModule, "NtCreatePort"), FakeNtCreatePort, (void **)&pfnNtCreatePort);
	}

	void StopNtCreatePortHook()
	{
		if (pfnNtCreatePort)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtCreatePort"), pfnNtCreatePort);

		pfnNtCreatePort = NULL;
	}
}