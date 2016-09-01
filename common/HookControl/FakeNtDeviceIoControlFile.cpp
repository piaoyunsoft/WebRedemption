#include <windows.h>
#include <tchar.h>
#include "HookHelp.h"
#include "FakeNtDeviceIoControlFile.h"


namespace HookControl{

	__pfnNtDeviceIoControlFile pfnNtDeviceIoControlFile = NULL;

	NTSTATUS WINAPI FakeNtDeviceIoControlFile(HANDLE FileHandle, HANDLE Event OPTIONAL, PVOID ApcRoutine OPTIONAL, PVOID ApcContext OPTIONAL, PVOID IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer OPTIONAL, ULONG InputBufferLength, PVOID OutputBuffer OPTIONAL, ULONG OutputBufferLength)
	{
		bool bIsCall = false;
		HOOKCONTROL_NTDEVICEIOCONTROLFILE hciInfo = { 0 };

		HciSetRetAddr(hciInfo);
		HciSetRetValue(hciInfo, STATUS_SUCCESS);

		if (IsPassCall(OnBeforeNtDeviceIoControlFile, hciInfo.CallAddress))
			return pfnNtDeviceIoControlFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);

		bIsCall = OnBeforeNtDeviceIoControlFile(&hciInfo, FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);

		if (bIsCall)
			hciInfo.RetValue = pfnNtDeviceIoControlFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);

		bIsCall = bIsCall && OnAfterNtDeviceIoControlFile(&hciInfo, FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);

		return hciInfo.RetValue;
	}

	BOOL StartNtDeviceIoControlFileHook()
	{
		HINSTANCE hModule = GetModuleHandle(_T("ntdll.dll"));

		if (NULL == hModule)
			hModule = LoadLibrary(_T("ntdll.dll"));

		if (pfnNtDeviceIoControlFile)
			return TRUE;

		return InlineHook(GetProcAddress(hModule, "NtDeviceIoControlFile"), FakeNtDeviceIoControlFile, (void **)&pfnNtDeviceIoControlFile);
	}

	void StopNtDeviceIoControlFileHook()
	{
		if (pfnNtDeviceIoControlFile)
			UnInlineHook(GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtDeviceIoControlFile"), pfnNtDeviceIoControlFile);

		pfnNtDeviceIoControlFile = NULL;
	}
}