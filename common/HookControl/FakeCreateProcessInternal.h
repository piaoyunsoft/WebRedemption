#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_CREATEPROCESSINTERNAL
	{
		void * CallAddress;
		BOOL RetValue;
	}HOOKCONTROL_CREATEPROCESSINTERNAL, *PHOOKCONTROL_CREATEPROCESSINTERNAL;

	typedef BOOL(WINAPI *__pfnCreateProcessInternalW) (HANDLE,LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION,PHANDLE);

	extern __pfnCreateProcessInternalW pfnCreateProcessInternal;

	bool OnBeforeCreateProcessInternal(HOOKCONTROL_CREATEPROCESSINTERNAL * retStatuse, HANDLE hToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, PHANDLE hNewToken);
	bool OnAfterCreateProcessInternal(HOOKCONTROL_CREATEPROCESSINTERNAL * retStatuse, HANDLE hToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, PHANDLE hNewToken);

	void StopCreateProcessInternalHook();
	bool StartCreateProcessInternalHook();
}
