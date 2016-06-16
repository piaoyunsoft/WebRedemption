#include "common_main.h"
#include "filter_socketsend.h"
#include "common_helper.h"

#define MAX_BUFFER_LEN				0x1000
#define MAX_ENCODE_LEN				((USHORT)0xFFFF)
#define MAX_CONCURRENT				10

namespace Global {
	bool Init = false;
	char * WSASendBuffer[MAX_CONCURRENT] = { 0 };
	HANDLE hWSASendMutex[MAX_CONCURRENT] = { 0 };
}

bool Filter::InitSocketSend() {
	if (false == Global::Init)
	{
		bool bInit = false;

		for (int i = 0; i < MAX_CONCURRENT; i++)
		{
			bInit = false;
			Global::WSASendBuffer[i] = (char *)malloc(MAX_BUFFER_LEN);

			if (NULL == Global::WSASendBuffer[i])
				break;

			Global::hWSASendMutex[i] = CreateMutex(NULL, FALSE, NULL);
			bInit = true;
		}

		Global::Init = bInit;
	}

	return Global::Init;
}

void Filter::UninitSocketSend() {
	if (false == Global::Init)
		return;

	Global::Init = false;

	if (WAIT_FAILED == WaitForMultipleObjects(MAX_CONCURRENT, Global::hWSASendMutex, TRUE, INFINITE)) {
		return; //出错
	}

	for (int i = 0; i < MAX_CONCURRENT; i++)
	{
		if (Global::WSASendBuffer[i])
			free(Global::WSASendBuffer[i]);

		Global::WSASendBuffer[i] = NULL;
		CloseHandle(Global::hWSASendMutex[i]);
	}
}

bool HookControl::IsPassCall(const TCHAR * pszCallType, void * pCallAddress)
{
#ifdef _DEBUG
	TCHAR szCallModuleName[MAX_PATH + 1] = { 0 };
	::GetModuleFileName(Common::GetModuleHandleByAddr(pCallAddress), szCallModuleName, MAX_PATH);
	Global::Log.Print(LOGOutputs, _T("HookControl::%s call module is: %s"), pszCallType, szCallModuleName);
#endif

	return false;
}

bool HookControl::OnBeforeSockSend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata, HookControl::__pfnSockSend pfnTCPSend)
{
	bool bIsCall = true;

	if (false == Global::Init)
		return true;

	if (1 != dwBufferCount)
		return true;

	if (lpBuffers->len < 4 || lpBuffers->len > MAX_BUFFER_LEN)
		return true;

	if (' TEG' != *((ULONG *)lpBuffers->buf)) // GET 判断
		return true;

	//////////////////////////////////////////////////////////////////////////
	// 
	sockaddr_in addrSocket = { 0 };

	int nSize = sizeof(sockaddr_in);

	if (0 != getpeername(s, (sockaddr*)&addrSocket, &nSize))
		return true;

	//////////////////////////////////////////////////////////////////////////
	// 

	CHAR szBuffer[MAX_IP_STRING_LEN + 1] = { 0 };
	__inet_ntop(addrSocket.sin_family, addrSocket.sin_addr, szBuffer, MAX_IP_STRING_LEN);

	Global::Log.PrintA(LOGOutputs, "HookControl::StartHTTPEncode(%s:%d) [len = %u]\r\n%s\r\n", szBuffer, ntohs(addrSocket.sin_port), lpBuffers->len, lpBuffers->buf);

	if (addrSocket.sin_port != Global::addrEncodeSocket.sin_port)
		return true;

	if (addrSocket.sin_addr.S_un.S_addr != Global::addrEncodeSocket.sin_addr.S_un.S_addr)
		return true;

	//////////////////////////////////////////////////////////////////////////
	// 

	int nIndex = 0;
	WSABUF wsaBuffers = { 0 };

	nIndex = WaitForMultipleObjects(MAX_CONCURRENT, Global::hWSASendMutex, FALSE, INFINITE);

	if (WAIT_FAILED == nIndex || WAIT_TIMEOUT == nIndex)
		return true;

	nIndex -= WAIT_OBJECT_0;

	wsaBuffers.len = lpBuffers->len;
	wsaBuffers.buf = (char *)Global::WSASendBuffer[nIndex];

	memcpy(wsaBuffers.buf, lpBuffers->buf, lpBuffers->len);

	//////////////////////////////////////////////////////////////////////////
	// HTTP Encode

	if (wsaBuffers.len > MAX_ENCODE_LEN)
		return true;

	USHORT usEncodeLen = (USHORT)wsaBuffers.len;
	PBYTE pEncodeHeader = (PBYTE)wsaBuffers.buf;

	pEncodeHeader[0] = 0xCD;

	*((USHORT *)&pEncodeHeader[1]) = htons((USHORT)usEncodeLen - 4); //4 = 头部大小

	pEncodeHeader[3] = pEncodeHeader[0] ^ (pEncodeHeader[1] + pEncodeHeader[2]);

	for (int i = 4; i < usEncodeLen; i++)
		pEncodeHeader[i] ^= pEncodeHeader[3] | 0x80;

	if (true == pfnTCPSend(s, &wsaBuffers, dwBufferCount, lpNumberOfBytesSent, pnErrorcode, lpOverlapped, lpCompletionRoutine, pExdata))
		bIsCall = false;

	ReleaseMutex(Global::hWSASendMutex[nIndex]);

	return bIsCall;
}

bool HookControl::OnAfterSockSend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata, HookControl::__pfnSockSend pfnTCPSend)
{
	return true;
}
