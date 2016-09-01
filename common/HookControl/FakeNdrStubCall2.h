#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_NDRSTUBCALL2
	{
		void * CallAddress;
		long RetValue;
	}HOOKCONTROL_NDRSTUBCALL2, *PHOOKCONTROL_NDRSTUBCALL2;
	
	typedef long(RPC_ENTRY *__pfnNdrStubCall2)(struct IRpcStubBuffer *, struct IRpcChannelBuffer *, PRPC_MESSAGE, unsigned long *);

	extern __pfnNdrStubCall2 pfnNdrStubCall2;

	bool OnBeforeNdrStubCall2(HOOKCONTROL_NDRSTUBCALL2 * retStatuse, _In_ struct IRpcStubBuffer *pThis, _In_ struct IRpcChannelBuffer *pChannel, _Inout_ PRPC_MESSAGE pRpcMsg, _Out_ unsigned long *pdwStubPhase);
	bool OnAfterNdrStubCall2(HOOKCONTROL_NDRSTUBCALL2 * retStatuse, _In_ struct IRpcStubBuffer *pThis, _In_  struct IRpcChannelBuffer *pChannel, _Inout_ PRPC_MESSAGE pRpcMsg, _Out_ unsigned long *pdwStubPhase);

	void StopNdrStubCall2Hook();
	bool StartNdrStubCall2Hook();
}
