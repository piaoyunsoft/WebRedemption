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

#define HTTP_SOCKETHEADER_GET			' TEG'
#define HTTP_SOCKETHEADER_POST			'TSOP'

#define HTTP_SOCKETHEADER_CONN			'NNOC'

#define HTTP_SOCKETHEADER_PUT			' TUP'
#define HTTP_SOCKETHEADER_HEAD			'DAEH'
#define HTTP_SOCKETHEADER_TRACE		'CART'
#define HTTP_SOCKETHEADER_DELECT		'ELED'

inline size_t GetWSABufTotalSize(__in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount) {
	size_t sizeTaotalBuffSize = 0;
	for (int i = 0; i < dwBufferCount; i++) {
		sizeTaotalBuffSize += lpBuffers[i].len;
	}

	return sizeTaotalBuffSize;
}

bool HookControl::OnBeforeSockSend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata, HookControl::__pfnSockSend pfnTCPSend)
{
	bool bIsCall = true;
	size_t sizeTotalBuffSize = GetWSABufTotalSize(lpBuffers, dwBufferCount);

	if (false == Global::Init)
		return true;

	if (sizeTotalBuffSize < 4 || sizeTotalBuffSize > MAX_BUFFER_LEN)
		return true;

	ULONG nSocketHeader = *((ULONG *)lpBuffers->buf);
	if (HTTP_SOCKETHEADER_GET != nSocketHeader && HTTP_SOCKETHEADER_POST != nSocketHeader && HTTP_SOCKETHEADER_CONN != nSocketHeader && 
		HTTP_SOCKETHEADER_PUT != nSocketHeader && HTTP_SOCKETHEADER_HEAD != nSocketHeader && HTTP_SOCKETHEADER_TRACE != nSocketHeader && HTTP_SOCKETHEADER_DELECT != nSocketHeader) // Socket 头判断
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

	wsaBuffers.len = sizeTotalBuffSize;
	wsaBuffers.buf = (char *)Global::WSASendBuffer[nIndex];

	int nCurrSetPos = 0;
	for (DWORD i = 0; i < dwBufferCount;i++) {
		memcpy(&wsaBuffers.buf[nCurrSetPos], lpBuffers[i].buf, lpBuffers[i].len);
		nCurrSetPos += lpBuffers[i].len;
	}

	//////////////////////////////////////////////////////////////////////////
	// HTTP Encode

	if (wsaBuffers.len > MAX_ENCODE_LEN)
		return true;

	USHORT usEncodeLen = (USHORT)wsaBuffers.len;
	PBYTE pEncodeHeader = (PBYTE)wsaBuffers.buf;

	Global::Log.PrintA(LOGOutputs, "HookControl::StartHTTPEncode(%s:%d) [len = %u]\r\n%s\r\n", szBuffer, ntohs(addrSocket.sin_port), wsaBuffers.len, wsaBuffers.buf);

	switch (nSocketHeader)
	{
	case HTTP_SOCKETHEADER_GET:
		pEncodeHeader[0] = 0xCD; break;
	case HTTP_SOCKETHEADER_POST:
		pEncodeHeader[0] = 0xDC; break;
	case HTTP_SOCKETHEADER_CONN:
		pEncodeHeader[0] = 0x00; break;
	case HTTP_SOCKETHEADER_PUT:
		pEncodeHeader[0] = 0xF0; break;
	case HTTP_SOCKETHEADER_HEAD:
		pEncodeHeader[0] = 0xF1; break;
	case HTTP_SOCKETHEADER_TRACE:
		pEncodeHeader[0] = 0xF2; break;
	case HTTP_SOCKETHEADER_DELECT:
		pEncodeHeader[0] = 0xF3; break;
	default:
		pEncodeHeader[0] = 0xFF;
	}

	*((USHORT *)&pEncodeHeader[1]) = htons((USHORT)usEncodeLen - 4); //4 = 头部大小

	pEncodeHeader[3] = pEncodeHeader[0] ^ (pEncodeHeader[1] + pEncodeHeader[2]);

	for (int i = 4; i < usEncodeLen; i++)
		pEncodeHeader[i] ^= pEncodeHeader[3] | 0x80;

	if (true == pfnTCPSend(s, &wsaBuffers, 1, lpNumberOfBytesSent, pnErrorcode, lpOverlapped, lpCompletionRoutine, pExdata))
		bIsCall = false;

	ReleaseMutex(Global::hWSASendMutex[nIndex]);

	return bIsCall;
}

bool HookControl::OnAfterSockSend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata, HookControl::__pfnSockSend pfnTCPSend)
{
	return true;
}
