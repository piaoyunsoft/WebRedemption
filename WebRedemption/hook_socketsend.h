#pragma once

namespace Hook {
	void StopSocketSendHook();
	bool StartSocketSendHook();
}

namespace FUN {
	typedef int (WSAAPI* __pfnsend)(__in SOCKET s, __in_bcount(len) const char FAR * buf, __in int len, __in int flags);
	typedef int (WSAAPI * __pfnWSASend) (__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in DWORD dwFlags, __inout_opt LPWSAOVERLAPPED lpOverlapped, _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
}
