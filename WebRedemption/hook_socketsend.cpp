#include "common_main.h"
#include "hook_socketsend.h"

#include <Ws2spi.h>
#include <HookControl/IATHook.h>
#include <HookControl/InlineHook.h>

#include "common_helper.h"
#include "filter_socketsend.h"

namespace {
	struct PARAMETERS_CALL_FAKEWSPSEND {
		int nRetValue;
		LPWSPSEND pfnWSPSend;

		DWORD dwFlags;
		LPWSATHREADID lpThreadId;
	};

	bool Call_FakeWSPSend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata)
	{
		bool bIsSuccess = true;
		PARAMETERS_CALL_FAKEWSPSEND * pCallParameters = (PARAMETERS_CALL_FAKEWSPSEND *)pExdata;

		pCallParameters->nRetValue = pCallParameters->pfnWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, pCallParameters->dwFlags, lpOverlapped, lpCompletionRoutine, pCallParameters->lpThreadId, pnErrorcode);

		if (SOCKET_ERROR == pCallParameters->nRetValue) {
			bIsSuccess = (WSA_IO_PENDING == *pnErrorcode);
		}

		return bIsSuccess;
	}

	//WSPSend
	LPWSPSEND pfnWSPSend = NULL;
	int WINAPI FakeWSPSend(__in SOCKET s, __in LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out LPDWORD lpNumberOfBytesSent, __in DWORD dwFlags, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, __in LPWSATHREADID lpThreadId, __out LPINT lpErrno)
	{
		bool bIsCall = false;

		int nRetValue = 0;
		void * pCallAddress = NULL;
		PARAMETERS_CALL_FAKEWSPSEND tpiCallParameters = { 0 };

		GetRetAddress(pCallAddress);

		tpiCallParameters.dwFlags = dwFlags;
		tpiCallParameters.lpThreadId = lpThreadId;
		tpiCallParameters.pfnWSPSend = pfnWSPSend;

		if (HookControl::IsPassCall(_T("FakeWSPSend"), pCallAddress))
			return  tpiCallParameters.pfnWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

		bIsCall = HookControl::OnBeforeSockSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, lpErrno, lpOverlapped, lpCompletionRoutine, &tpiCallParameters, Call_FakeWSPSend);

		if (bIsCall)
			return tpiCallParameters.pfnWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

		bIsCall = HookControl::OnAfterSockSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, lpErrno, lpOverlapped, lpCompletionRoutine, &tpiCallParameters, Call_FakeWSPSend);

		if (false == bIsCall && (0 != *lpErrno && WSA_IO_PENDING != *lpErrno)) { // 如果未调用，并且过滤函数返回错误代码，设置 WSA 错误代码
			WSASetLastError(*lpErrno);
			tpiCallParameters.nRetValue = SOCKET_ERROR;
		}

		return tpiCallParameters.nRetValue;
	}
}

namespace {
	struct PARAMETERS_CALL_FAKEWSASEND {
		int nRetValue;
		FUN::__pfnWSASend pfnWSASend;

		DWORD dwFlags;
	};

	bool Call_FakeWSASend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata)
	{
		bool bIsSuccess = true;
		PARAMETERS_CALL_FAKEWSASEND * pCallParameters = (PARAMETERS_CALL_FAKEWSASEND *)pExdata;

		pCallParameters->nRetValue = pCallParameters->pfnWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, pCallParameters->dwFlags, lpOverlapped, lpCompletionRoutine);

		if (SOCKET_ERROR == pCallParameters->nRetValue) {
			bIsSuccess = false;
			*pnErrorcode = WSAGetLastError();
		}

		return bIsSuccess;
	}


	FUN::__pfnWSASend pfnWSASend = NULL;
	int WSAAPI FakeWSASend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in DWORD dwFlags, __inout_opt LPWSAOVERLAPPED lpOverlapped, _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
	{
		bool bIsCall = false;

		int nErrorcode = 0;
		void * pCallAddress = NULL;
		PARAMETERS_CALL_FAKEWSASEND tpiCallParameters = { 0 };

		GetRetAddress(pCallAddress);

		tpiCallParameters.dwFlags = dwFlags;
		tpiCallParameters.pfnWSASend = pfnWSASend;

		if (HookControl::IsPassCall(_T("FakeWSASend"), pCallAddress))
			return tpiCallParameters.pfnWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);

		bIsCall = HookControl::OnBeforeSockSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, &nErrorcode, lpOverlapped, lpCompletionRoutine, &tpiCallParameters, Call_FakeWSASend);

		if (bIsCall)
			return tpiCallParameters.pfnWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);

		bIsCall = HookControl::OnAfterSockSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, &nErrorcode, lpOverlapped, lpCompletionRoutine, &tpiCallParameters, Call_FakeWSASend);

		if (false == bIsCall && (0 != nErrorcode && WSA_IO_PENDING != nErrorcode)) // 如果未调用，并且过滤函数返回错误代码，设置 WSA 错误代码
		{
			WSASetLastError(nErrorcode);
			tpiCallParameters.nRetValue = SOCKET_ERROR;
		}

		return tpiCallParameters.nRetValue;
	}

}

namespace {
	struct PARAMETERS_CALL_FAKESEND {
		int nRetValue;
		FUN::__pfnsend pfnsend;

		int nFlags;
	};

	bool Call_Fakesend(__in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount, __out_opt LPDWORD lpNumberOfBytesSent, __in int * pnErrorcode, __in LPWSAOVERLAPPED lpOverlapped, __in LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, void * pExdata)
	{
		bool bIsSuccess = true;
		PARAMETERS_CALL_FAKESEND * pCallParameters = (PARAMETERS_CALL_FAKESEND *)pExdata;
		
		pCallParameters->nRetValue = pCallParameters->pfnsend(s, lpBuffers->buf, lpBuffers->len, pCallParameters->nFlags);

		if (SOCKET_ERROR != pCallParameters->nRetValue ** lpNumberOfBytesSent) {
			*lpNumberOfBytesSent = pCallParameters->nRetValue;
		}

		if (SOCKET_ERROR == pCallParameters->nRetValue) {
			bIsSuccess = false;
			*pnErrorcode = WSAGetLastError();
		}

		return bIsSuccess;
	}

	FUN::__pfnsend pfnsend = NULL;
	int WSAAPI Fakesend(__in SOCKET s, __in_bcount(len) const char FAR * buf, __in int len, __in int flags) {
		bool bIsCall = true;
		WSABUF wsaBuffers = { 0 };
		DWORD dwNumberOfBytesSent = 0;

		int nErrorcode = 0;
		void * pCallAddress = NULL;
		PARAMETERS_CALL_FAKESEND tpiCallParameters = { 0 };

		GetRetAddress(pCallAddress);

		wsaBuffers.len = len;
		wsaBuffers.buf = (char FAR *)buf;

		tpiCallParameters.nFlags = flags;
		tpiCallParameters.pfnsend = pfnsend;

		if (HookControl::IsPassCall(_T("Fakesend"), pCallAddress))
			return tpiCallParameters.pfnsend(s, buf, len, flags);

		bIsCall = HookControl::OnBeforeSockSend(s, &wsaBuffers, 1, &dwNumberOfBytesSent, &nErrorcode, NULL, NULL, &tpiCallParameters, Call_Fakesend);

		tpiCallParameters.nRetValue = dwNumberOfBytesSent;

		if (bIsCall)
			return tpiCallParameters.pfnsend(s, wsaBuffers.buf, wsaBuffers.len, tpiCallParameters.nFlags);

		bIsCall = HookControl::OnAfterSockSend(s, &wsaBuffers, 1, &dwNumberOfBytesSent, &nErrorcode, NULL, NULL, &tpiCallParameters, Call_Fakesend);

		if (false == bIsCall && (0 != nErrorcode && WSA_IO_PENDING != nErrorcode)) // 如果未调用，并且过滤函数返回错误代码，设置 WSA 错误代码
		{
			WSASetLastError(nErrorcode);
			tpiCallParameters.nRetValue = SOCKET_ERROR;
		}

		return tpiCallParameters.nRetValue;
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
			if (NULL == pfnWSASend) {
				pfnWSASend = (FUN::__pfnWSASend)GetProcAddress(hBaseInstance, "WSASend");

				HINSTANCE hHookInstance = NULL;
				for (int i = 0; i < sizeof(pszSocketHookLists) / sizeof(pszSocketHookLists[0]); i++) {
					if (false == LockModule(pszSocketHookLists[i], &hHookInstance)) {
						continue;
					}

					bIsOK = HookControl::IATHook(hHookInstance, _T("WS2_32.dll"), pfnWSASend, FakeWSASend);
					Global::Log.Print(LOGOutputs, _T("HookControl::IATHook(% 15s,[WS2_32.dll,WSASend], FakeWSASend) is %s(%u)."), pszSocketHookLists[i], tszHitModuleFileName, bIsOK);
				}

				bIsOK = HookControl::InlineHook(pfnWSASend, FakeWSASend, (void**)&pfnWSASend);
				Global::Log.Print(LOGOutputs, _T("HookControl::InlineHook([WS2_32.dll,WSASend], FakeWSASend) is %s(%u)."), tszHitModuleFileName, bIsOK);
			}

			if (NULL == pfnsend) {
				pfnsend = (FUN::__pfnsend)GetProcAddress(hBaseInstance, "send");

				HINSTANCE hHookInstance = NULL;
				for (int i = 0; i < sizeof(pszSocketHookLists) / sizeof(pszSocketHookLists[0]); i++) {
					if (false == LockModule(pszSocketHookLists[i], &hHookInstance)) {
						continue;
					}

					bIsOK = HookControl::IATHook(hHookInstance, _T("WS2_32.dll"), pfnsend, Fakesend);
					Global::Log.Print(LOGOutputs, _T("HookControl::IATHook(% 15s,[WS2_32.dll,send], Fakesend) is %s(%u)."), pszSocketHookLists[i], tszHitModuleFileName, bIsOK);
				}

				bIsOK = HookControl::InlineHook(pfnsend, Fakesend, (void**)&pfnsend);
				Global::Log.Print(LOGOutputs, _T("HookControl::InlineHook([WS2_32.dll,send], Fakesend) is %s(%u)."), tszHitModuleFileName, bIsOK);
			}
		}

		return TRUE;
	}
}

bool Hook::StartSocketSendHook()
{
	DWORD dwThreadID = 0;

	HANDLE hThread = CreateThread(NULL, 0, Thread_HookControl, NULL, 0, &dwThreadID);

	if (hThread)
		CloseHandle(hThread);

	return NULL != hThread;
}
