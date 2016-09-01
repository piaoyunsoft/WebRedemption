#include <windows.h>
#include <tchar.h>
#include "FakeNdrStubCall2.h"


namespace HookControl{

	__pfnNdrStubCall2 pfnNdrStubCall2 = NULL;

	long WINAPI FakeNdrStubCall2(_In_ struct IRpcStubBuffer *pThis, _In_ struct IRpcChannelBuffer *pChannel, _Inout_ PRPC_MESSAGE pRpcMsg, _Out_ unsigned long *pdwStubPhase)
	{
		bool bIsCall = false;
		HOOKCONTROL_NDRSTUBCALL2 hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, STATUS_SUCCESS);

		if (IsPassCall(OnBeforeNdrStubCall2, hciInfo.CallAddress))
			return pfnNdrStubCall2(pThis, pChannel, pRpcMsg, pdwStubPhase);

		bIsCall = OnBeforeNdrStubCall2(&hciInfo, pThis, pChannel, pRpcMsg, pdwStubPhase);

		if (bIsCall)
			hciInfo.RetValue = pfnNdrStubCall2(pThis, pChannel, pRpcMsg, pdwStubPhase);

		bIsCall = bIsCall && OnAfterNdrStubCall2(&hciInfo, pThis, pChannel, pRpcMsg, pdwStubPhase);

		return hciInfo.RetValue;
	}

	bool StartNdrStubCall2Hook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("RpcRT4.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("RpcRT4.dll"));

		if (pfnNdrStubCall2)
			return true;

		return InlineHook(GetProcAddress(hModule, "NdrStubCall2"), FakeNdrStubCall2, (void **)&pfnNdrStubCall2);
	}

	void StopNdrStubCall2Hook()
	{
		if (pfnNdrStubCall2)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("RpcRT4.dll")), "NdrStubCall2"), pfnNdrStubCall2);

		pfnNdrStubCall2 = NULL;
	}
}