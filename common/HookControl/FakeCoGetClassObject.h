#pragma once
#include <vector>
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_COGETCLASSOBJECT
	{
		void * CallAddress;
		HRESULT RetValue;
	}HOOKCONTROL_COGETCLASSOBJECT, *PHOOKCONTROL_COGETCLASSOBJECT;

	typedef HRESULT (WINAPI *__pfnCoGetClassObject)(REFCLSID, DWORD, COSERVERINFO *, REFIID riid, LPVOID *);

	extern __pfnCoGetClassObject pfnCoGetClassObject;

	bool OnBeforeCoGetClassObject(HOOKCONTROL_COGETCLASSOBJECT * retStatus, REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID riid, LPVOID *ppv);
	bool OnAfterCoGetClassObject(HOOKCONTROL_COGETCLASSOBJECT * retStatus, REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID riid, LPVOID *ppv);

	void StopCoGetClassObjectHook();
	BOOL StartCoGetClassObjectHook();
}
