#include "common_main.h"
#include "export_function.h"

#include "common_helper.h"
#include "filter_socketsend.h"
#include "hook_socketsend.h"
#include "filter_socketconnect.h"
#include "hook_socketconnect.h"

#include "..\common\common_fundadores.h"

BOOL WINAPI Fundadores(const wchar_t * pszParam) {
	return Common::Fundadores_(pszParam);
}

HANDLE WINAPI SetBusinessData(sockaddr_in * paddrPACSocket, sockaddr_in * paddrEncodeSocket)
{
	BUSINESS_DATA tbdBusinessData = { 0 };

	CHAR szBuffer[MAX_IP_STRING_LEN + 1] = { 0 };

	__inet_ntop(AF_INET, paddrPACSocket->sin_addr, szBuffer, MAX_IP_STRING_LEN);

	strcpy(tbdBusinessData.szPACServerIP, szBuffer);
	tbdBusinessData.usPACServerProt = ntohs(paddrPACSocket->sin_port);

	__inet_ntop(AF_INET, paddrEncodeSocket->sin_addr, szBuffer, MAX_IP_STRING_LEN);

	strcpy(tbdBusinessData.szEncodeSockIP, szBuffer);
	tbdBusinessData.usEncodeSockProt = ntohs(paddrEncodeSocket->sin_port);

	return Common::SetBufferToShareMap("Global\\SSOORLP_ENCODE_BUSINESS_DATA", &tbdBusinessData, sizeof(BUSINESS_DATA));
}


DWORD WINAPI StartBusiness_Thread(void *)
{
	CHAR szBuffer[MAX_IP_STRING_LEN + 1] = { 0 };
	WCHAR wszBuffer[MAX_IP_STRING_LEN + 1] = { 0 };

	if (false == Common::GetBufferToShareMap("Global\\SSOORLP_ENCODE_BUSINESS_DATA", (void**)&Global::pBusinessData))
	{
#ifdef _DEBUG
		Global::pBusinessData = new BUSINESS_DATA;
		Global::pBusinessData->usEncodeSockProt = 60000;
		strcpy_s(Global::pBusinessData->szEncodeSockIP, "127.0.0.1");
#else
		Global::Log.PrintA(LOGOutputs, "StartBusiness failed: %u", ::GetLastError());
		return -1;
#endif
	}

	Global::Log.PrintA(LOGOutputs, "ENCODE:(%s,%u)", Global::pBusinessData->szEncodeSockIP, Global::pBusinessData->usEncodeSockProt);

	if (0 != Global::pBusinessData->usEncodeSockProt)
	{
		Global::addrEncodeSocket.sin_family = AF_INET;
		Global::addrEncodeSocket.sin_port = htons(Global::pBusinessData->usEncodeSockProt);
		Global::addrEncodeSocket.sin_addr.s_addr = inet_addr(Global::pBusinessData->szEncodeSockIP);

		Filter::InitSocketSend();
		Filter::InitSocketConnect();

		Hook::StartSocketSendHook();
		Hook::StartSocketConnectHook();
	}

	return 0;
}

DWORD WINAPI StartBusiness(void *)
{
	DWORD dwThreadId = 0;

	CloseHandle(CreateThread(NULL, 0, StartBusiness_Thread, NULL, 0, &dwThreadId));

	return dwThreadId;
}