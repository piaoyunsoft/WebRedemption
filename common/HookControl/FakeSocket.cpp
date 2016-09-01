#include <windows.h>
#include <tchar.h>
#include "FakeSocket.h"

namespace HookControl{

	__pfnrecv pfnrecv = NULL;

	int WINAPI Fakerecv(SOCKET s, const char * buf, int len, int flags)
	{
		bool bIsCall = false; 
		HOOKCONTROL_SOCKET_RECV hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeSocketrecv, hciInfo.CallAddress))
			return pfnrecv(s, buf, len, flags);

		bIsCall = OnBeforeSocketrecv(&hciInfo, s, buf, len, flags);

		if (bIsCall)
			hciInfo.RetValue = pfnrecv(s, buf, len, flags);

		bIsCall = bIsCall && OnAfterSocketrecv(&hciInfo, s, buf, len, flags);

		return hciInfo.RetValue;
	}

	bool StartSocketrecvHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Ws2_32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Ws2_32.dll"));

		if (pfnrecv)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "recv"), Fakerecv, (void **)&pfnrecv);
	}

	void StopSocketrecvHook()
	{
		if (pfnrecv)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Ws2_32.dll")), "recv"), pfnrecv);

		pfnrecv = NULL;
	}
}
