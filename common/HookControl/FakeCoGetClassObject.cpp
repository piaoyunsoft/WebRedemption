#include <windows.h>
#include <tchar.h>
#include "FakeCoGetClassObject.h"


namespace HookControl{

	__pfnCoGetClassObject pfnCoGetClassObject = NULL;

	HRESULT WINAPI FakeCoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID riid, LPVOID *ppv)
	{
		bool bIsCall = false;
		HOOKCONTROL_COGETCLASSOBJECT hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, S_OK);

		if (IsPassCall(OnBeforeCoGetClassObject, hciInfo.CallAddress))
			return pfnCoGetClassObject(rclsid, dwClsContext, pServerInfo, riid, ppv);

		bIsCall = OnBeforeCoGetClassObject(&hciInfo,rclsid, dwClsContext, pServerInfo, riid, ppv);

		if (bIsCall)
			hciInfo.RetValue = pfnCoGetClassObject(rclsid, dwClsContext, pServerInfo, riid, ppv);

		bIsCall = bIsCall && OnAfterCoGetClassObject(&hciInfo, rclsid, dwClsContext, pServerInfo, riid, ppv);

		return hciInfo.RetValue;
	}

	BOOL StartCoGetClassObjectHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Ole32.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Ole32.dll"));

		if (pfnCoGetClassObject)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "CoGetClassObject"), FakeCoGetClassObject, (void **)&pfnCoGetClassObject);
	}

	void StopCoGetClassObjectHook()
	{
		if (pfnCoGetClassObject)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Ole32.dll")), "CoGetClassObject"), pfnCoGetClassObject);

		pfnCoGetClassObject = NULL;
	}
}