#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_NTCREATEPORT
	{
		void * CallAddress;
		NTSTATUS RetValue;
	}HOOKCONTROL_NTCREATEPORT, *PHOOKCONTROL_NTCREATEPORT;

	typedef NTSTATUS(WINAPI *__pfnNtCreatePort)(PHANDLE, POBJECT_ATTRIBUTES, ULONG, ULONG, ULONG);

	extern __pfnNtCreatePort pfnNtCreatePort;

	bool OnBeforeNtCreatePort(HOOKCONTROL_NTCREATEPORT * retStatuse, OUT PHANDLE PortHandle, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG MaxConnectionInfoLength, IN ULONG MaxMessageLength, IN ULONG MaxPoolUsage);
	bool OnAfterNtCreatePort(HOOKCONTROL_NTCREATEPORT * retStatuse, OUT PHANDLE PortHandle, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG MaxConnectionInfoLength, IN ULONG MaxMessageLength, IN ULONG MaxPoolUsage);

	void StopNtCreatePortHook();
	bool StartNtCreatePortHook();
}
