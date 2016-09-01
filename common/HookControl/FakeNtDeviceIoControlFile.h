#pragma once
#include <vector>
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_NTDEVICEIOCONTROLFILE
	{
		void * CallAddress;
		NTSTATUS RetValue;
	}HOOKCONTROL_NTDEVICEIOCONTROLFILE, *PHOOKCONTROL_NTDEVICEIOCONTROLFILE;

	typedef NTSTATUS(WINAPI *__pfnNtDeviceIoControlFile) (HANDLE, HANDLE, PVOID, PVOID, PVOID, ULONG, PVOID, ULONG, PVOID, ULONG);

	extern __pfnNtDeviceIoControlFile pfnNtDeviceIoControlFile;

	bool OnBeforeNtDeviceIoControlFile(HOOKCONTROL_NTDEVICEIOCONTROLFILE * retStatus, HANDLE FileHandle, HANDLE Event OPTIONAL, PVOID ApcRoutine OPTIONAL, PVOID ApcContext OPTIONAL, PVOID IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer OPTIONAL, ULONG InputBufferLength, PVOID OutputBuffer OPTIONAL, ULONG OutputBufferLength);
	bool OnAfterNtDeviceIoControlFile(HOOKCONTROL_NTDEVICEIOCONTROLFILE * retStatus, HANDLE FileHandle, HANDLE Event OPTIONAL, PVOID ApcRoutine OPTIONAL, PVOID ApcContext OPTIONAL, PVOID IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer OPTIONAL, ULONG InputBufferLength, PVOID OutputBuffer OPTIONAL, ULONG OutputBufferLength);

	void StopNtDeviceIoControlFileHook();
	BOOL StartNtDeviceIoControlFileHook();
}
