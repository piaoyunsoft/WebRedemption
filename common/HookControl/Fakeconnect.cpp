#include <windows.h>
#include <tchar.h>
#include "Fakeconnect.h"


namespace HookControl{

	__pfnconnect pfnconnect = NULL;

	int WINAPI Fakeconnect(SOCKET s, const struct sockaddr *name, int namelen)
	{
		bool bIsCall = false;
		HOOKCONTROL_SOCKET_CONNECT hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeSocketconnect, hciInfo.CallAddress))
			return pfnconnect(s, name, namelen);

		bIsCall = OnBeforeSocketconnect(&hciInfo, s, name, namelen);

		if (bIsCall)
			hciInfo.RetValue = pfnconnect(s, name, namelen);

		bIsCall = bIsCall && OnAfterSocketconnect(&hciInfo, s, name, namelen);

		return hciInfo.RetValue;
	}

	bool StartSocketconnectHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Ws2_32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Ws2_32.dll"));

		if (pfnconnect)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "connect"), Fakeconnect, (void **)&pfnconnect);
	}

	void StopSocketconnectHook()
	{
		if (pfnconnect)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Ws2_32.dll")), "connect"), pfnconnect);

		pfnconnect = NULL;
	}
}