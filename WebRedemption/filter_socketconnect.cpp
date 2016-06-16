#include "common_main.h"
#include "filter_socketconnect.h"
#include "common_helper.h"

bool Filter::InitSocketConnect() {
	return true;
}

void Filter::UninitSocketConnect() {
	return;
}

bool HookControl::OnBeforeSockConnect(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen, _In_ LPWSABUF lpCallerData, _Out_ LPWSABUF lpCalleeData, _In_ LPQOS lpSQOS, _In_ LPQOS lpGQOS, void * pExdata, __pfnSockConnect pfnSockConnect) {
	if (80 != ntohs(((sockaddr_in*)name)->sin_port)) {
		return true;
	}

	CHAR szBuffer[MAX_IP_STRING_LEN + 1] = { 0 };
	__inet_ntop(((sockaddr_in*)name)->sin_family, ((sockaddr_in*)name)->sin_addr, szBuffer, MAX_IP_STRING_LEN);

	Global::Log.PrintA(LOGOutputs, "HookControl::StartSockReconnect(%s:%d) to %s:%d\r\n", szBuffer, ntohs(((sockaddr_in*)name)->sin_port), Global::pBusinessData->szEncodeSockIP, Global::pBusinessData->usEncodeSockProt);

	pfnSockConnect(s, (sockaddr*)&Global::addrEncodeSocket, sizeof(sockaddr_in), lpCallerData, lpCalleeData, lpSQOS, lpGQOS, pExdata);

	return false;
}

bool HookControl::OnAfterSockConnect(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen, _In_ LPWSABUF lpCallerData, _Out_ LPWSABUF lpCalleeData, _In_ LPQOS lpSQOS, _In_ LPQOS lpGQOS, void * pExdata, __pfnSockConnect pfnSockConnect) {
	return true;
}

