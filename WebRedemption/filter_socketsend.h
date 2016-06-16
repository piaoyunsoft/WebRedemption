#pragma once

namespace Filter {
	bool InitSocketSend();
	void UninitSocketSend();
}

namespace HookControl {
	typedef bool(*__pfnSockSend)(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata);

	bool IsPassCall(const TCHAR * pszCallType, void * pCallAddress);

	bool OnAfterSockSend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata, __pfnSockSend pfnSockSend);
	bool OnBeforeSockSend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata, __pfnSockSend pfnSockSend);
}
