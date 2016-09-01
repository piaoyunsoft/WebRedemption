#pragma once

#include <windows.h>
#include <Dbghelp.h>
#include <tchar.h>
#include <iostream>
#include <vector>

using namespace std;

#pragma comment(lib, "Dbghelp.lib")

namespace NSDumpFile
{

#define OBJ_CASE_INSENSITIVE 0x00000040L

#ifndef InitializeObjectAttributes
#define InitializeObjectAttributes( pObjectAttributes, ustrName, aAttribute, rdRootDirectory, sdSecurityDescriptor ) { \
	(pObjectAttributes)->Length = sizeof(OBJECT_ATTRIBUTES); \
	(pObjectAttributes)->RootDirectory = rdRootDirectory; \
	(pObjectAttributes)->Attributes = aAttribute; \
	(pObjectAttributes)->ObjectName = ustrName; \
	(pObjectAttributes)->SecurityDescriptor = sdSecurityDescriptor; \
	(pObjectAttributes)->SecurityQualityOfService = NULL; \
}
#endif
	typedef LONG NTSTATUS;

	typedef LONG NTSTATUS;

	typedef struct _UNICODE_STRING {
		unsigned short  Length;     //UNICODE占用的内存字节数，个数*2；
		unsigned short  MaximumLength;
		wchar_t*  Buffer;
	} UNICODE_STRING, *PUNICODE_STRING;

	typedef struct _IO_STATUS_BLOCK {
		union
		{
			NTSTATUS Status;
			void * Pointer;
		};

		ULONG_PTR Information;
	} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

	//
	// Object Attributes structure
	//

	typedef struct _OBJECT_ATTRIBUTES {
		ULONG Length;
		HANDLE RootDirectory;
		PUNICODE_STRING ObjectName;
		ULONG Attributes;
		PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
		PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
	} OBJECT_ATTRIBUTES;
	typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

	inline void * GetProcAddress(LPCTSTR pszDllName, LPCSTR pszProcName)
	{

		HINSTANCE hModule = GetModuleHandle(pszDllName);

		if (NULL == hModule)
			hModule = LoadLibrary(pszDllName);

		return ::GetProcAddress(hModule, pszProcName);

	}

#define FILE_CREATE 0x00000002
#define IO_NO_PARAMETER_CHECKING 0x0100
#define FILE_NON_DIRECTORY_FILE 0x00000040
#define FILE_SYNCHRONOUS_IO_NONALERT 0x00000020

	typedef VOID(WINAPI * __pfnRtlInitUnicodeString) (PUNICODE_STRING, PCWSTR);
	typedef NTSTATUS(WINAPI *__pfnNtCreateFile) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);

	static VOID(WINAPI * RtlInitUnicodeString) (PUNICODE_STRING, PCWSTR) = (__pfnRtlInitUnicodeString)GetProcAddress(_T("ntdll.dll"), "RtlInitUnicodeString");
	static NTSTATUS(WINAPI * NtCreateFile) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG) = (__pfnNtCreateFile)GetProcAddress(_T("ntdll.dll"), "NtCreateFile");

	static void CreateDumpFile(EXCEPTION_POINTERS *pException)
	{
		IO_STATUS_BLOCK ioStatusBlock;
		UNICODE_STRING ustrFilefullPath;
		OBJECT_ATTRIBUTES osObjectAttributes;
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;

		HANDLE hFileHandle = NULL;
		wchar_t * pszFilefullPath = NULL;
		wchar_t szNtFilefullPath[MAX_PATH + 5] = { 0 };

		wcscpy(szNtFilefullPath, L"\\??\\");

		pszFilefullPath = &szNtFilefullPath[4];

		::GetModuleFileNameW(NULL, pszFilefullPath, MAX_PATH);


		wchar_t * pFileNameEnd = wcsrchr(pszFilefullPath, L'.');
		if (pFileNameEnd)
		{
			*(pFileNameEnd + 1) = NULL;
			swprintf_s(pszFilefullPath,MAX_PATH, L"%s%010u.dmp", pszFilefullPath, GetTickCount());
		}

		dumpInfo.ClientPointers = TRUE;
		dumpInfo.ExceptionPointers = pException;
		dumpInfo.ThreadId = GetCurrentThreadId();

		if (RtlInitUnicodeString)
			RtlInitUnicodeString(&ustrFilefullPath, szNtFilefullPath);

		InitializeObjectAttributes(&osObjectAttributes, &ustrFilefullPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

		if (RtlInitUnicodeString)
			NtCreateFile(&hFileHandle, FILE_GENERIC_WRITE, &osObjectAttributes, &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_CREATE, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

		if (NULL == hFileHandle || INVALID_HANDLE_VALUE == hFileHandle)
			hFileHandle = CreateFileW(pszFilefullPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);

#ifdef _DEBUG
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFileHandle, MiniDumpWithFullMemory, &dumpInfo, NULL, NULL);
#else
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFileHandle, MiniDumpNormal, &dumpInfo, NULL, NULL);
#endif

		CloseHandle(hFileHandle);
	}

	static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
	{
		return NULL;
	}

	static BOOL PreventSetUnhandledExceptionFilter()
	{
		HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
		if (hKernel32 == NULL)
			return FALSE;

		void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
		if(pOrgEntry == NULL)
			return FALSE;

		unsigned char newJump[ 100 ];
		DWORD dwOrgEntryAddr = (DWORD) pOrgEntry;
		dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far

		void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
		DWORD dwNewEntryAddr = (DWORD) pNewFunc;
		DWORD dwRelativeAddr = dwNewEntryAddr -  dwOrgEntryAddr;

		newJump[ 0 ] = 0xE9;  // JMP absolute
		memcpy(&newJump[ 1 ], &dwRelativeAddr, sizeof(pNewFunc));
		SIZE_T bytesWritten;
		BOOL bRet = WriteProcessMemory(GetCurrentProcess(),    pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
		return bRet;
	}

	static LONG WINAPI UnhandledExceptionFilterEx(struct _EXCEPTION_POINTERS *pException)
	{
		char szRunFile[MAX_PATH + 1] = { 0 };

		CreateDumpFile(pException);

		::GetModuleFileNameA(NULL, szRunFile, MAX_PATH);

		WinExec(szRunFile, SW_SHOW);

		return EXCEPTION_CONTINUE_SEARCH;
	}

	static void RunCrashHandler()
	{
		SetUnhandledExceptionFilter(UnhandledExceptionFilterEx);
		PreventSetUnhandledExceptionFilter();
	}
};

#define DeclareDumpFile() NSDumpFile::RunCrashHandler();
