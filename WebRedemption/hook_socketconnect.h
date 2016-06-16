#pragma once

namespace Hook {
	void StopSocketConnectHook();
	bool StartSocketConnectHook();
}

namespace FUN {
	typedef int (WSAAPI* __pfnconnect)(_In_ SOCKET s, const struct sockaddr FAR *name, _In_ int namelen);
	typedef int (WSAAPI* __pfnWSAConnect)(_In_ SOCKET s, const struct sockaddr FAR * name, _In_ int namelen, _In_opt_ LPWSABUF lpCallerData, _Out_opt_ LPWSABUF lpCalleeData, _In_opt_ LPQOS lpSQOS, _In_opt_ LPQOS lpGQOS);
}