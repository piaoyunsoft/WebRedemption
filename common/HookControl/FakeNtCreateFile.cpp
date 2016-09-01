#include <windows.h>
#include <tchar.h>
#include "FakeNtCreateFile.h"


namespace HookControl{

	__pfnNtCreateFile pfnNtCreateFile = NULL;



	NTSTATUS WINAPI FakeNtCreateFile(OUT PHANDLE  FileHandle, IN ACCESS_MASK  DesiredAccess, IN POBJECT_ATTRIBUTES  ObjectAttributes, OUT PIO_STATUS_BLOCK  IoStatusBlock, IN PLARGE_INTEGER  AllocationSize  OPTIONAL, IN ULONG  FileAttributes, IN ULONG  ShareAccess, IN ULONG  CreateDisposition, IN ULONG  CreateOptions, IN PVOID  EaBuffer  OPTIONAL, IN ULONG  EaLength)
	{
		bool bIsCall = false;
		HOOKCONTROL_NTCREATEFILE hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, STATUS_SUCCESS);

		if (IsPassCall(OnBeforeNtCreateFile, hciInfo.CallAddress))
			return pfnNtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);

		bIsCall = OnBeforeNtCreateFile(&hciInfo, FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);

		if (bIsCall)
			hciInfo.RetValue = pfnNtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);

		bIsCall = bIsCall && OnAfterNtCreateFile(&hciInfo, FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);

		return hciInfo.RetValue;
	}

	bool StartNtCreateFileHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("ntdll.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("ntdll.dll"));

		if (pfnNtCreateFile)
			return true;

		return InlineHook(GetProcAddress(hModule, "NtCreateFile"), FakeNtCreateFile, (void **)&pfnNtCreateFile);
	}

	void StopNtCreateFileHook()
	{
		if (pfnNtCreateFile)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtCreateFile"), pfnNtCreateFile);

		pfnNtCreateFile = NULL;
	}
}