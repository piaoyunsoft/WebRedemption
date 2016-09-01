#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_SOCKET_SEND
	{
		void * CallAddress;
		int RetValue;
	}HOOKCONTROL_SOCKET_SEND, *PHOOKCONTROL_SOCKET_SEND;

	typedef int (WINAPI * __pfnsend)(SOCKET s, const char *buf, int len, int flags);

	extern __pfnsend pfnsend;

	bool OnBeforeSocketsend(HOOKCONTROL_SOCKET_SEND * retStatuse, SOCKET s, const char *buf, int len, int flags);
	bool OnAfterSocketsend(HOOKCONTROL_SOCKET_SEND * retStatuse, SOCKET s, const char *buf, int len, int flags);

	void StopSocketsendHook();
	bool StartSocketsendHook();
}
