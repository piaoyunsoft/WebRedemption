#pragma once
#include "HookHelp.h"
#include "InlineHook.h"

namespace HookControl{

	typedef struct _HOOKCONTROL_SOCKET_CONNECT
	{
		void * CallAddress;
		int RetValue;
	}HOOKCONTROL_SOCKET_CONNECT, *PHOOKCONTROL_SOCKET_CONNECT;

	typedef int (WINAPI * __pfnconnect)(SOCKET s,const struct sockaddr *name,int namelen);
	
	extern __pfnconnect pfnconnect;

	bool OnBeforeSocketconnect(HOOKCONTROL_SOCKET_CONNECT * retStatuse, SOCKET s, const struct sockaddr * name, int namelen);
	bool OnAfterSocketconnect(HOOKCONTROL_SOCKET_CONNECT * retStatuse, SOCKET s, const struct sockaddr * name, int namelen);

	void StopSocketconnectHook();
	bool StartSocketconnectHook();
}
