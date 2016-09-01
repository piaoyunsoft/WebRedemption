#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_CREATEFILE
	{
		void * CallAddress;
		HANDLE RetValue;
	}HOOKCONTROL_CREATEFILE, *PHOOKCONTROL_CREATEFILE;

	typedef HANDLE(WINAPI *__pfnCreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

	extern __pfnCreateFileW pfnCreateFileW;

	bool OnBeforeCreateFile(HOOKCONTROL_CREATEFILE * retStatuse, _In_ LPCTSTR lpFileName , _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile);
	bool OnAfterCreateFile(HOOKCONTROL_CREATEFILE * retStatuse, _In_ LPCTSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile);

	void StopCreateFileHook();
	bool StartCreateFileHook();
}
