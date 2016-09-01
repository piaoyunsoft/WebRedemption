#include <windows.h>
#include <tchar.h>
#include "FakeNtReplyWaitReceivePortEx.h"


namespace HookControl{

	__pfnNtReplyWaitReceivePortEx pfnNtReplyWaitReceivePortEx = NULL;

	NTSTATUS WINAPI FakeNtReplyWaitReceivePortEx(IN HANDLE PortHandle, OUT PVOID* PortIdentifier OPTIONAL, IN PPORT_MESSAGE ReplyMessage OPTIONAL, OUT PPORT_MESSAGE Message, IN PLARGE_INTEGER Timeout)
	{
		bool bIsCall = false;
		HOOKCONTROL_NTREPLYWAITRECEIVEPORTEX hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, STATUS_SUCCESS);

		if (IsPassCall(OnBeforeNtReplyWaitReceivePortEx, hciInfo.CallAddress))
			return pfnNtReplyWaitReceivePortEx(PortHandle, PortIdentifier, ReplyMessage, Message, Timeout);

		bIsCall = OnBeforeNtReplyWaitReceivePortEx(&hciInfo, PortHandle, PortIdentifier, ReplyMessage, Message, Timeout);

		if (bIsCall)
			hciInfo.RetValue = pfnNtReplyWaitReceivePortEx(PortHandle, PortIdentifier, ReplyMessage, Message, Timeout);

		bIsCall = bIsCall && OnAfterNtReplyWaitReceivePortEx(&hciInfo, PortHandle, PortIdentifier, ReplyMessage, Message, Timeout);

		return hciInfo.RetValue;
	}

	bool StartNtReplyWaitReceivePortExHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("ntdll.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("ntdll.dll"));

		if (pfnNtReplyWaitReceivePortEx)
			return true;

		return InlineHook(GetProcAddress(hModule, "NtReplyWaitReceivePortEx"), FakeNtReplyWaitReceivePortEx, (void **)&pfnNtReplyWaitReceivePortEx);
	}

	void StopNtReplyWaitReceivePortExHook()
	{
		if (pfnNtReplyWaitReceivePortEx)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtReplyWaitReceivePortEx"), pfnNtReplyWaitReceivePortEx);

		pfnNtReplyWaitReceivePortEx = NULL;
	}
}