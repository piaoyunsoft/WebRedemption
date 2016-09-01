#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_NTCREATEFILE
	{
		void * CallAddress;
		NTSTATUS RetValue;
	}HOOKCONTROL_NTCREATEFILE, *PHOOKCONTROL_NTCREATEFILE;

	typedef NTSTATUS(WINAPI *__pfnNtCreateFile) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);

	extern __pfnNtCreateFile pfnNtCreateFile;

	bool OnBeforeNtCreateFile(HOOKCONTROL_NTCREATEFILE * retStatuse, OUT PHANDLE  FileHandle, IN ACCESS_MASK  DesiredAccess, IN POBJECT_ATTRIBUTES  ObjectAttributes, OUT PIO_STATUS_BLOCK  IoStatusBlock, IN PLARGE_INTEGER  AllocationSize  OPTIONAL, IN ULONG  FileAttributes, IN ULONG  ShareAccess, IN ULONG  CreateDisposition, IN ULONG  CreateOptions, IN PVOID  EaBuffer  OPTIONAL, IN ULONG  EaLength);
	bool OnAfterNtCreateFile(HOOKCONTROL_NTCREATEFILE * retStatuse, OUT PHANDLE  FileHandle, IN ACCESS_MASK  DesiredAccess, IN POBJECT_ATTRIBUTES  ObjectAttributes, OUT PIO_STATUS_BLOCK  IoStatusBlock, IN PLARGE_INTEGER  AllocationSize  OPTIONAL, IN ULONG  FileAttributes, IN ULONG  ShareAccess, IN ULONG  CreateDisposition, IN ULONG  CreateOptions, IN PVOID  EaBuffer  OPTIONAL, IN ULONG  EaLength);

	void StopNtCreateFileHook();
	bool StartNtCreateFileHook();
}
