#pragma once

namespace Filter {
	bool InitSocketConnect();
	void UninitSocketConnect();
}

namespace HookControl {
	typedef bool(*__pfnSockConnect)(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData, _Out_ LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, void * pExdata);

	bool OnAfterSockConnect(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData, _Out_ LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, void * pExdata, __pfnSockConnect pfnTCPSend);
	bool OnBeforeSockConnect(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData, _Out_ LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, void * pExdata, __pfnSockConnect pfnTCPSend);
}