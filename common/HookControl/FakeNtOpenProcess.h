#pragma once
#include "HookHelp.h"
#include "InlineHook.h"

namespace HookControl{

	typedef struct _HOOKCONTROL_NTOPENPROCESS
	{
		void * CallAddress;
		NTSTATUS RetValue;
	}HOOKCONTROL_NTOPENPROCESS, *PHOOKCONTROL_NTOPENPROCESS;

	typedef NTSTATUS(NTAPI * __pfnNtOpenProcess)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES,PCLIENT_ID);

	extern __pfnNtOpenProcess pfnNtOpenProcess;

	bool OnBeforeNtOpenProcess(HOOKCONTROL_NTOPENPROCESS * retStatuse, OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId OPTIONAL);
	bool OnAfterNtOpenProcess(HOOKCONTROL_NTOPENPROCESS * retStatuse, OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId OPTIONAL);

	void StopNtOpenProcessHook();
	bool StartNtOpenProcessHook();
}
