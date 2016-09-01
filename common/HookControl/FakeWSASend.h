#pragma once
#include <winsock2.h>

namespace HookControl{

	typedef struct _HOOKCONTROL_WSASEND
	{
		void * CallAddress;
		int RetValue;
	}HOOKCONTROL_WSASEND, *PHOOKCONTROL_WSASEND;

	typedef int (WSAAPI * __pfnWSASend) (__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in DWORD dwFlags, __inout_opt LPWSAOVERLAPPED lpOverlapped, _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

	extern __pfnWSASend pfnWSASend;

	bool OnBeforeWSASend(HOOKCONTROL_WSASEND * retStatuse, __in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in DWORD dwFlags, __inout_opt LPWSAOVERLAPPED lpOverlapped, _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	bool OnAfterWSASend(HOOKCONTROL_WSASEND * retStatuse, __in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in DWORD dwFlags, __inout_opt LPWSAOVERLAPPED lpOverlapped, _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

	void StopWSASendHook();
	bool StartWSASendHook();
}
