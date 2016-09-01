#include "common_main.h"
#include "filter_socketsend.h"
#include "common_helper.h"
#include <vector>

#define MAX_HEADER_SIZE								4
#define MAX_ENCODE_LEN								((USHORT)0xFFFF)

#define MAX_BUFFER_LEN								(MAX_HEADER_SIZE + MAX_ENCODE_LEN)
#define MAX_CONCURRENT								10
#define MAX_TRANSPARENT_SOCKET				0x1000

namespace Global {
	bool Init = false;
	char * WSASendBuffer[MAX_CONCURRENT] = { 0 };
	HANDLE hWSASendMutex[MAX_CONCURRENT] = { 0 };
	std::vector<SOCKET> vTransparentSockets;
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
	if (Common::GetModuleHandleByAddr(pCallAddress) == Common::GetModuleHandleByAddr(IsPassCall)) {
		return true;
	}


	TCHAR szCallModuleName[MAX_PATH + 1] = { 0 };
	::GetModuleFileName(Common::GetModuleHandleByAddr(pCallAddress), szCallModuleName, MAX_PATH);
#ifdef _DEBUG
	Global::Log.Print(LOGOutputs, _T("HookControl::%s call module is: %s"), pszCallType, szCallModuleName);
#endif

	PathStripPath(szCallModuleName);
	if (NULL != StrStrI(_T("impshae.dll"), szCallModuleName))  // 360net.dll/
	{
#ifdef _DEBUG		
		Global::Log.Print(LOGOutputs, _T("HookControl::%s module=%s, return true"), pszCallType, szCallModuleName);
#endif
		return true;
	}

	return false;
}

#define HTTP_SOCKETHEADER_GET			' TEG'
#define HTTP_SOCKETHEADER_POST			'TSOP'

#define HTTP_SOCKETHEADER_CONN			'NNOC'

#define HTTP_SOCKETHEADER_PUT			' TUP'
#define HTTP_SOCKETHEADER_HEAD			'DAEH'
#define HTTP_SOCKETHEADER_TRACE		'CART'
#define HTTP_SOCKETHEADER_DELECT		'ELED'

inline size_t GetWSABufTotalSize(__in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in int nBufferCount) {
	size_t sizeTaotalBuffSize = 0;
	for (int i = 0; i < nBufferCount; i++) {
		sizeTaotalBuffSize += lpBuffers[i].len;
	}

	return sizeTaotalBuffSize;
}

inline bool IsTransparentSocket(__in SOCKET s) {
	std::vector<SOCKET>::reverse_iterator it;
	for (it = Global::vTransparentSockets.rbegin(); it != Global::vTransparentSockets.rend(); it++) {
		if (*it == s) {
			return true;
		}
	}

	return false;
}

inline byte SendEncodeHeader(__in SOCKET s, PBYTE pEncodeHeader , USHORT usTotalEncodeSize, void * pExdata, HookControl::__pfnSockSend pfnTCPSend) {
	pEncodeHeader[0] = 0xFF;

	*((USHORT *)&pEncodeHeader[1]) = htons((USHORT)usTotalEncodeSize); //4 = 头部大小

	pEncodeHeader[3] = pEncodeHeader[0] ^ (pEncodeHeader[1] + pEncodeHeader[2]);

	WSABUF wsaBuffers = { 0 };
	wsaBuffers.len = MAX_HEADER_SIZE;
	wsaBuffers.buf = (CHAR *)pEncodeHeader;
	
	DWORD dwNumberOfBytesSent;
	int nErrorcode;
	pfnTCPSend(s, &wsaBuffers, 1, &dwNumberOfBytesSent, &nErrorcode, NULL, NULL, pExdata); // send(s, (char *)pEncodeHeader, MAX_HEADER_SIZE, 0);
#ifdef _DEBUG
	Global::Log.PrintA(LOGOutputs, "SendEncodeHeader(%d) BytesSent=%d, error=%d(LastError=%d) \r\n", s, dwNumberOfBytesSent, nErrorcode, GetLastError());
#endif

	return pEncodeHeader[3];
}

bool HookControl::OnBeforeSockSend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata, HookControl::__pfnSockSend pfnTCPSend)
{
	bool bIsCall = true;
	size_t sizeTotalBuffSize = GetWSABufTotalSize(lpBuffers, dwBufferCount);

	if (IsTransparentSocket(s)) {
		return true;
	}

	if (false == Global::Init) {
		Global::vTransparentSockets.push_back(s);
	}

	if (lpBuffers->len < 4 || sizeTotalBuffSize > MAX_ENCODE_LEN) {
		Global::vTransparentSockets.push_back(s);
	}

	//////////////////////////////////////////////////////////////////////////
	// 
	sockaddr_in addrSocket = { 0 };

	int nSize = sizeof(sockaddr_in);

	if (0 != getpeername(s, (sockaddr*)&addrSocket, &nSize)) {
		Global::vTransparentSockets.push_back(s);
	}

	CHAR szBuffer[MAX_IP_STRING_LEN + 1] = { 0 };
	__inet_ntop(addrSocket.sin_family, addrSocket.sin_addr, szBuffer, MAX_IP_STRING_LEN);

	if (addrSocket.sin_port != Global::addrEncodeSocket.sin_port) {
		Global::vTransparentSockets.push_back(s);
	}

	if (addrSocket.sin_addr.S_un.S_addr != Global::addrEncodeSocket.sin_addr.S_un.S_addr) {
		Global::vTransparentSockets.push_back(s);
	}

	if (IsTransparentSocket(s)) {
		return true;
	}

	int nIndex = 0;
	nIndex = WaitForMultipleObjects(MAX_CONCURRENT, Global::hWSASendMutex, FALSE, INFINITE);

	if (WAIT_FAILED == nIndex || WAIT_TIMEOUT == nIndex) {
		Global::vTransparentSockets.push_back(s);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 

	WSABUF wsaBuffers = { 0 };

	if (nIndex < WAIT_ABANDONED_0)
		nIndex -= WAIT_OBJECT_0;
	else
		nIndex -= WAIT_ABANDONED_0;

	wsaBuffers.len = sizeTotalBuffSize;
	wsaBuffers.buf = &Global::WSASendBuffer[nIndex][MAX_HEADER_SIZE];

	int nCurrSetPos = 0;
	for (DWORD i = 0; i < dwBufferCount;i++) {
		memcpy(&wsaBuffers.buf[nCurrSetPos], lpBuffers[i].buf, lpBuffers[i].len);
		nCurrSetPos += lpBuffers[i].len;
	}

	byte byteEncodeCode = SendEncodeHeader(s, (PBYTE)Global::WSASendBuffer[nIndex], sizeTotalBuffSize, pExdata, pfnTCPSend);

	//////////////////////////////////////////////////////////////////////////
	// HTTP Encode

	Global::Log.PrintA(LOGOutputs, "HookControl::StartHTTPEncode(%s:%d) [len = %u]\r\n%s\r\n", szBuffer, ntohs(addrSocket.sin_port), wsaBuffers.len, wsaBuffers.buf);

	for (size_t i = 0; i < sizeTotalBuffSize; i++) {
		wsaBuffers.buf[i] ^= byteEncodeCode | 0x80;
	}

	if (true == pfnTCPSend(s, &wsaBuffers, 1, lpNumberOfBytesSent, pnErrorcode, lpOverlapped, lpCompletionRoutine, pExdata)) {
		bIsCall = false;
	}

	ReleaseMutex(Global::hWSASendMutex[nIndex]);

	return bIsCall;
}

bool HookControl::OnAfterSockSend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata, HookControl::__pfnSockSend pfnTCPSend)
{
	return true;
}
