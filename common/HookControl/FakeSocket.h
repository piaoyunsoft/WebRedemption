#pragma once
#include "HookHelp.h"
#include "InlineHook.h"

namespace HookControl{

	typedef struct _HOOKCONTROL_SOCKET_RECV
	{
		void * CallAddress;
		int RetValue;
	}HOOKCONTROL_SOCKET_RECV, *PHOOKCONTROL_SOCKET_RECV;

	typedef int (WINAPI * __pfnrecv)(SOCKET s, const char *buf, int len, int flags);

	extern __pfnrecv pfnrecv;

	bool OnBeforeSocketrecv(HOOKCONTROL_SOCKET_RECV * retStatuse, SOCKET s, const char * buf, int len, int flags);
	bool OnAfterSocketrecv(HOOKCONTROL_SOCKET_RECV * retStatuse, SOCKET s, const char * buf, int len, int flags); 

	void StopSocketrecvHook();
	bool StartSocketrecvHook();
}