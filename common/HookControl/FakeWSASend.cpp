#include <tchar.h>
#include "FakeWSASend.h"

#include "HookHelp.h"
#include "InlineHook.h"

namespace HookControl{

	__pfnWSASend pfnWSASend = NULL;

	int WSAAPI FakeWSASend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in DWORD dwFlags, __inout_opt LPWSAOVERLAPPED lpOverlapped, _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
	{
		bool bIsCall = false;
		HOOKCONTROL_WSASEND hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeWSASend, hciInfo.CallAddress))
			return pfnWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);

		bIsCall = OnBeforeWSASend(&hciInfo, s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);

		if (bIsCall)
			hciInfo.RetValue = pfnWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);

		bIsCall = bIsCall && OnAfterWSASend(&hciInfo, s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);

		return hciInfo.RetValue;
	}

	bool StartWSASendHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Ws2_32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Ws2_32.dll"));

		if (pfnWSASend)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "WSASend"), FakeWSASend, (void **)&pfnWSASend);
	}

	void StopWSASendHook()
	{
		if (pfnWSASend)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Ws2_32.dll")), "WSASend"), pfnWSASend);

	}
}
