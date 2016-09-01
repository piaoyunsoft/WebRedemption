#include "common_main.h"
#include "hook_socketconnect.h"
#include <Ws2spi.h>

#include "HookControl\InlineHook.h"
#include "filter_socketsend.h"
#include "filter_socketconnect.h"
#include "common_helper.h"
#include "HookControl\IATHook.h"
#include <mswsock.h>

namespace {
	struct PARAMETERS_CALL_FAKEWSPCONNECT {
		int nRetValue;
		LPWSPCONNECT pfnWSPConnect;

		LPINT lpErrno;
	};

	bool Call_FakeWSPConnect(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen, _In_ LPWSABUF lpCallerData, _Out_ LPWSABUF lpCalleeData, _In_ LPQOS lpSQOS, _In_ LPQOS lpGQOS, void * pExdata)
	{
		bool bIsSuccess = true;
		PARAMETERS_CALL_FAKEWSPCONNECT * pCallParameters = (PARAMETERS_CALL_FAKEWSPCONNECT *)pExdata;

		pCallParameters->nRetValue = pCallParameters->pfnWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, pCallParameters->lpErrno);

		if (SOCKET_ERROR == pCallParameters->nRetValue) {
			bIsSuccess = false;
		}

		return bIsSuccess;
	}

	LPWSPCONNECT pfnWSPConnect = NULL;
	int WSPAPI FakeWSPConnect(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno)
	{
		bool bIsCall = false;

		void * pCallAddress = NULL;
		PARAMETERS_CALL_FAKEWSPCONNECT tpiCallParameters = { 0 };

		GetRetAddress(pCallAddress);

		tpiCallParameters.lpErrno = lpErrno;
		tpiCallParameters.pfnWSPConnect = pfnWSPConnect;

		if (HookControl::IsPassCall(_T("FakeWSPConnect"), pCallAddress))
			return tpiCallParameters.pfnWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);

		bIsCall = HookControl::OnBeforeSockConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, &tpiCallParameters, Call_FakeWSPConnect);

		if (bIsCall)
			return tpiCallParameters.pfnWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);

		bIsCall = HookControl::OnAfterSockConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, &tpiCallParameters, Call_FakeWSPConnect);

		return tpiCallParameters.nRetValue;
	}
}

namespace {
	struct PARAMETERS_CALL_FAKEWSACONNECT {
		int nRetValue;
		FUN::__pfnWSAConnect pfnWSAConnect;
	};

	bool Call_FakeWSAConnect(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen, _In_ LPWSABUF lpCallerData, _Out_ LPWSABUF lpCalleeData, _In_ LPQOS lpSQOS, _In_ LPQOS lpGQOS, void * pExdata)
	{
		bool bIsSuccess = true;
		PARAMETERS_CALL_FAKEWSACONNECT * pCallParameters = (PARAMETERS_CALL_FAKEWSACONNECT *)pExdata;

		pCallParameters->nRetValue = pCallParameters->pfnWSAConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);

		if (SOCKET_ERROR == pCallParameters->nRetValue) {
			bIsSuccess = false;
		}

		return bIsSuccess;
	}

	FUN::__pfnWSAConnect pfnWSAConnect = NULL;
	int WSAAPI FakeWSAConnect(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS)
	{
		bool bIsCall = false;

		void * pCallAddress = NULL;
		PARAMETERS_CALL_FAKEWSACONNECT tpiCallParameters = { 0 };
		// 返回调用者的地址
		GetRetAddress(pCallAddress);

		tpiCallParameters.pfnWSAConnect = pfnWSAConnect;

		// 作用:
		// 	1、防止调用自己的死循环
		// 	2、防止双重hook的负效应
		if (HookControl::IsPassCall(_T("FakeWSAConnect"), pCallAddress))
			return tpiCallParameters.pfnWSAConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);

		bIsCall = HookControl::OnBeforeSockConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, &tpiCallParameters, Call_FakeWSAConnect);

		if (bIsCall)
			return tpiCallParameters.pfnWSAConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);

		//bIsCall = HookControl::OnAfterSockConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, &tpiCallParameters, Call_FakeWSAConnect);

		return tpiCallParameters.nRetValue;
	}
}

namespace {
	struct PARAMETERS_CALL_FAKECONNECTEX {
		int nRetValue;
		LPFN_CONNECTEX pfnConnectEx;

		PVOID lpSendBuffer;
		DWORD dwSendDataLength;
		LPDWORD lpdwBytesSent;
		LPOVERLAPPED lpOverlapped;
	};

	bool Call_FakeConnectEx(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen, _In_ LPWSABUF lpCallerData, _Out_ LPWSABUF lpCalleeData, _In_ LPQOS lpSQOS, _In_ LPQOS lpGQOS, void * pExdata)
	{
		bool bIsSuccess = true;
		PARAMETERS_CALL_FAKECONNECTEX * pCallParameters = (PARAMETERS_CALL_FAKECONNECTEX *)pExdata;

		pCallParameters->nRetValue = pCallParameters->pfnConnectEx(s, name, namelen, pCallParameters->lpSendBuffer, pCallParameters->dwSendDataLength, pCallParameters->lpdwBytesSent, pCallParameters->lpOverlapped);

		if (SOCKET_ERROR == pCallParameters->nRetValue) {
			bIsSuccess = false;
		}

		return bIsSuccess;
	}

	LPFN_CONNECTEX pfnConnectEx = NULL;
	BOOL PASCAL FAR FakeConnectEx(SOCKET s, const struct sockaddr FAR *name, int namelen, PVOID lpSendBuffer, DWORD dwSendDataLength, LPDWORD lpdwBytesSent, LPOVERLAPPED lpOverlapped) {
		bool bIsCall = false;

		void * pCallAddress = NULL;
		PARAMETERS_CALL_FAKECONNECTEX tpiCallParameters = { 0 };

		GetRetAddress(pCallAddress);

		tpiCallParameters.pfnConnectEx = pfnConnectEx;

		tpiCallParameters.lpSendBuffer = lpSendBuffer;
		tpiCallParameters.dwSendDataLength = dwSendDataLength;
		tpiCallParameters.lpdwBytesSent = lpdwBytesSent;
		tpiCallParameters.lpOverlapped = lpOverlapped;

		if (HookControl::IsPassCall(_T("FakeConnectEx"), pCallAddress))
			return tpiCallParameters.pfnConnectEx(s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);

		bIsCall = HookControl::OnBeforeSockConnect(s, name, namelen, NULL, NULL, NULL, NULL, &tpiCallParameters, Call_FakeConnectEx);

		if (bIsCall)
			return tpiCallParameters.pfnConnectEx(s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);

		//bIsCall = HookControl::OnAfterSockConnect(s, name, namelen, NULL, NULL, NULL, NULL, &tpiCallParameters, Call_FakeConnectEx);

		return tpiCallParameters.nRetValue;
	}
}

namespace {
	struct PARAMETERS_CALL_FAKECONNECT {
		int nReturn;
		FUN::__pfnconnect pfnconnect;
	};

	bool Call_Fakeconnect(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen, _In_ LPWSABUF lpCallerData, _Out_ LPWSABUF lpCalleeData, _In_ LPQOS lpSQOS, _In_ LPQOS lpGQOS, void * pExdata)
	{
		bool bIsSuccess = true;
		PARAMETERS_CALL_FAKECONNECT * pCallParameters = (PARAMETERS_CALL_FAKECONNECT *)pExdata;

		pCallParameters->nReturn = pCallParameters->pfnconnect(s, name, namelen);

		if (SOCKET_ERROR == pCallParameters->nReturn) {
			bIsSuccess = false;
		}

		return bIsSuccess;
	}

	FUN::__pfnconnect pfnconnect = NULL;
	int WSAAPI Fakeconnect(SOCKET s, const struct sockaddr *name, int namelen)
	{
		bool bIsCall = false;

		int nRetValue = 0;
		void * pCallAddress = NULL;
		PARAMETERS_CALL_FAKECONNECT tpiCallParameters = { 0 };

		GetRetAddress(pCallAddress);

		tpiCallParameters.pfnconnect = pfnconnect;

		if (HookControl::IsPassCall(_T("Fakeconnect"), pCallAddress))
			return tpiCallParameters.pfnconnect(s, name, namelen);

		bIsCall = HookControl::OnBeforeSockConnect(s, name, namelen, NULL, NULL, NULL, NULL, &tpiCallParameters, Call_Fakeconnect);

		if (bIsCall)
			return tpiCallParameters.pfnconnect(s, name, namelen);

		//bIsCall = bIsCall && HookControl::OnAfterSockConnect(s, name, namelen, NULL, NULL, NULL, NULL, &tpiCallParameters, Call_Fakeconnect);

		return  tpiCallParameters.nReturn;
	}
}

namespace {

	DWORD WINAPI Thread_HookControl(void *)
	{
		HINSTANCE hHitHinstance = NULL;
		TCHAR tszHitModuleFileName[MAX_PATH + 1] = { 0 };

		//////////////////////////////////////////////////////////////////////////
		// IAT Hook

		for (int count = 0; count < 6666; count++)
		{
			hHitHinstance = Lists::ModuleNameHitTest();
			if (NULL != hHitHinstance && 0 != ::GetModuleFileName(hHitHinstance, tszHitModuleFileName, MAX_PATH)) {
				break;
			}

			if (NULL != Lists::ProcessNameHitTest() && 0 != ::GetModuleFileName(NULL, tszHitModuleFileName, MAX_PATH)) {
				break;
			}

			Sleep(count % 10);
		}

		bool bIsOK = false;
		HINSTANCE hBaseInstance = NULL;

		if (0 != _tcslen(tszHitModuleFileName) && LockModule(_T("WS2_32.dll"), &hBaseInstance))
		{
			if (NULL == pfnWSAConnect) {
				pfnWSAConnect = (FUN::__pfnWSAConnect)GetProcAddress(hBaseInstance, "WSAConnect");

				HINSTANCE hHookInstance = NULL;
				for (int i = 0; i < sizeof(pszSocketHookLists) / sizeof(pszSocketHookLists[0]); i++) {
					if (false == LockModule(pszSocketHookLists[i], &hHookInstance)) {
						continue;
					}

					bIsOK = HookControl::IATHook(hHookInstance, _T("WS2_32.dll"), pfnWSAConnect, FakeWSAConnect);
					Global::Log.Print(LOGOutputs, _T("HookControl::IATHook(% 15s,[WS2_32.dll,WSAConnect], FakeWSAConnect) is %s(%u)."), pszSocketHookLists[i], tszHitModuleFileName, bIsOK);
				}

				bIsOK = HookControl::InlineHook(pfnWSAConnect, FakeWSAConnect, (void**)&pfnWSAConnect);
				Global::Log.Print(LOGOutputs, _T("HookControl::InlineHook([WS2_32.dll,WSAConnect], FakeWSAConnect) is %s(%u)."), tszHitModuleFileName, bIsOK);
			}

			if (NULL == pfnConnectEx) {
				WSADATA wsaData;
				WSAStartup(MAKEWORD(2, 2), &wsaData);

				GUID wsaIDConnectEx = WSAID_CONNECTEX;
				if (GetWSAExFunction(wsaIDConnectEx, (void **)&pfnConnectEx) && pfnConnectEx) {

					bIsOK = HookControl::InlineHook(pfnConnectEx, FakeConnectEx, (void **)&pfnConnectEx);
					Global::Log.Print(LOGOutputs, _T("HookControl::InlineHook([mswsock.dll,ConnectEx], Fakeconnect) is %s(%u)."), tszHitModuleFileName, bIsOK);
				}
			}

			if (NULL == pfnconnect) {
				pfnconnect = (FUN::__pfnconnect)GetProcAddress(hBaseInstance, "connect");

				HINSTANCE hHookInstance = NULL;
				for (int i = 0; i < sizeof(pszSocketHookLists) / sizeof(pszSocketHookLists[0]); i++) {
					if (false == LockModule(pszSocketHookLists[i], &hHookInstance)) {
						continue;
					}

					bIsOK = HookControl::IATHook(hHookInstance, _T("WS2_32.dll"), pfnconnect, Fakeconnect);
					Global::Log.Print(LOGOutputs, _T("HookControl::IATHook(% 15s,[WS2_32.dll,connect], Fakeconnect) is %s(%u)."), pszSocketHookLists[i], tszHitModuleFileName, bIsOK);
				}

				bIsOK = HookControl::InlineHook(pfnconnect, Fakeconnect, (void**)&pfnconnect);
				Global::Log.Print(LOGOutputs, _T("HookControl::InlineHook([WS2_32.dll,connect], Fakeconnect) is %s(%u)."), tszHitModuleFileName, bIsOK);
			}
		}

		return TRUE;
	}
}

bool Hook::StartSocketConnectHook()
{
	DWORD dwThreadID = 0;

	HANDLE hThread = CreateThread(NULL, 0, Thread_HookControl, NULL, 0, &dwThreadID);

	if (hThread)
		CloseHandle(hThread);

	return NULL != hThread;
}
