#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_NTRESUMETHREAD
	{
		void * CallAddress;
		NTSTATUS RetValue;
	}HOOKCONTROL_NTRESUMETHREAD, *PHOOKCONTROL_NTRESUMETHREAD;

	typedef NTSTATUS(WINAPI *__pfnNtResumeThread)(IN HANDLE, OUT PULONG);

	extern __pfnNtResumeThread pfnNtResumeThread;

	bool OnBeforeNtResumeThread(HOOKCONTROL_NTRESUMETHREAD * retStatuse, IN HANDLE ThreadHandle,OUT PULONG SuspendCount OPTIONAL);
	bool OnAfterNtResumeThread(HOOKCONTROL_NTRESUMETHREAD * retStatuse, IN HANDLE ThreadHandle, OUT PULONG SuspendCount OPTIONAL);

	void StopNtResumeThreadHook();
	bool StartNtResumeThreadHook();
}
