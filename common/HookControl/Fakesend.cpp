#include <windows.h>
#include <tchar.h>
#include "Fakesend.h"

namespace HookControl{

	__pfnsend pfnsend = NULL;

	int WINAPI Fakesend(SOCKET s, const char * buf, int len, int flags)
	{
		bool bIsCall = false;
		HOOKCONTROL_SOCKET_SEND hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeSocketsend, hciInfo.CallAddress))
			return pfnsend(s, buf, len, flags);

		bIsCall = OnBeforeSocketsend(&hciInfo, s, buf, len, flags);

		if (bIsCall)
			hciInfo.RetValue = pfnsend(s, buf, len, flags);

		bIsCall = bIsCall && OnAfterSocketsend(&hciInfo, s, buf, len, flags);

		return hciInfo.RetValue;
	}

	bool StartSocketsendHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Ws2_32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Ws2_32.dll"));

		if (pfnsend)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "send"), Fakesend, (void **)&pfnsend);
	}

	void StopSocketsendHook()
	{
		if (pfnsend)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Ws2_32.dll")), "send"), pfnsend);

	}
}
