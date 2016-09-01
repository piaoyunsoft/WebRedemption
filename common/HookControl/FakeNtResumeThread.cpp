#include <windows.h>
#include <tchar.h>
#include "FakeNtResumeThread.h"



namespace HookControl{

	__pfnNtResumeThread pfnNtResumeThread = NULL;

	NTSTATUS WINAPI FakeNtResumeThread(IN HANDLE ThreadHandle, OUT PULONG SuspendCount OPTIONAL)
	{
		bool bIsCall = false;
		HOOKCONTROL_NTRESUMETHREAD hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, NULL);

		if (IsPassCall(OnBeforeNtResumeThread, hciInfo.CallAddress))
			return pfnNtResumeThread(ThreadHandle, SuspendCount);

		bIsCall = OnBeforeNtResumeThread(&hciInfo, ThreadHandle, SuspendCount);

		if (bIsCall)
			hciInfo.RetValue = pfnNtResumeThread(ThreadHandle, SuspendCount);

		bIsCall = bIsCall &&OnAfterNtResumeThread(&hciInfo, ThreadHandle, SuspendCount);

		return hciInfo.RetValue;
	}

	bool StartNtResumeThreadHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("Ntdll.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("Kernel32.dll"));

		if (pfnNtResumeThread)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "NtResumeThread"), FakeNtResumeThread, (void **)&pfnNtResumeThread);
	}

	void StopNtResumeThreadHook()
	{
		if (pfnNtResumeThread)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("Ntdll.dll")), "NtResumeThread"), pfnNtResumeThread);

		pfnNtResumeThread = NULL;
	}
}