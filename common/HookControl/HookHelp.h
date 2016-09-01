#pragma once 
#include <string>
#include <shtypes.h>
#include <Shlobj.h>
#include <accctrl.h>
#include <winternl.h>
#include "..\CommonControl\Commonfun.h"

#ifndef NTSTATUS
typedef /*_Return_type_success_(return >= 0)*/ LONG NTSTATUS;
#endif

//#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_SUCCESS					((NTSTATUS)0x00000000L)
#define STATUS_UNSUCCESSFUL				((NTSTATUS)0xC0000001L)
#define STATUS_ACCESS_DENIED			((NTSTATUS)0xC0000022L)
#define STATUS_NOT_IMPLEMENTED			((NTSTATUS)0xC0000002L)
#define STATUS_BUFFER_TOO_SMALL			((NTSTATUS)0xC0000023L)
#define STATUS_INVALID_INFO_CLASS		((NTSTATUS)0xC0000003L)
#define STATUS_INFO_LENGTH_MISMATCH		((NTSTATUS)0xC0000004L)
#define STATUS_OBJECT_NAME_NOT_FOUND	((NTSTATUS)0xC0000034L)
#define STATUS_OBJECT_PATH_NOT_FOUND	((NTSTATUS)0xC000003AL)
#define STATUS_NO_MORE_FILES			((NTSTATUS)0x80000006L)
#define STATUS_ACCESS_DENIED			((NTSTATUS)0xC0000022L)
#define STATUS_INVALID_PORT_ATTRIBUTES	((NTSTATUS)0xC000002EL)
#define STATUS_NOT_IMPLEMENTED			((NTSTATUS)0xC0000002L)
#define STATUS_PORT_DISCONNECTED		((NTSTATUS)0xC0000037L)
#define STATUS_PORT_CONNECTION_REFUSED	((NTSTATUS)0xC0000041L)
#define STATUS_INVALID_PORT_HANDLE		((NTSTATUS)0xC0000042L)
#define STATUS_PORT_ALREADY_SET			((NTSTATUS)0xC0000048L)
#define STATUS_INFO_LEN_MISMATCH		((NTSTATUS)0xC0000004L)
#define STATUS_DEVICE_NOT_READY			((NTSTATUS)0xC00000A3L)
#define STATUS_BUFFER_OVERFLOW			((NTSTATUS)0x80000005L)

//typedef ULONG THREADINFOCLASS;
//typedef ULONG PROCESSINFOCLASS;
typedef ULONG_PTR KAFFINITY;
typedef ULONG KPRIORITY;
#define MEMORY_BASIC_INFORMATION_SIZE 28

typedef struct _CLIENT_ID {
	DWORD_PTR UniqueProcess;
	DWORD_PTR UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _THREAD_BASIC_INFORMATION {
	NTSTATUS                ExitStatus;
	PVOID                   TebBaseAddress;
	CLIENT_ID               ClientId;
	KAFFINITY               AffinityMask;
	KPRIORITY               Priority;
	KPRIORITY               BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

// typedef struct _UNICODE_STRING {
// 	unsigned short  Length;     //UNICODE占用的内存字节数，个数*2；
// 	unsigned short  MaximumLength;
// 	wchar_t*  Buffer;
// } UNICODE_STRING, *PUNICODE_STRING;

// typedef enum _OBJECT_INFORMATION_CLASS {
// 	ObjectBasicInformation,
// 	ObjectNameInformation,
// 	ObjectTypeInformation,
// 	ObjectAllInformation,
// 	ObjectDataInformation
// } OBJECT_INFORMATION_CLASS, * POBJECT_INFORMATION_CLASS;

namespace HookHelp {
	typedef enum _OBJECT_INFORMATION_CLASS {
		     ObjectBasicInformation,
		     ObjectNameInformation,
		     ObjectTypeInformation,
		     ObjectAllInformation,
		     ObjectDataInformation
		
	} OBJECT_INFORMATION_CLASS;
	typedef enum _THREADINFOCLASS {
		ThreadBasicInformation,
		ThreadTimes,
		ThreadPriority,
		ThreadBasePriority,
		ThreadAffinityMask,
		ThreadImpersonationToken,
		ThreadDescriptorTableEntry,
		ThreadEnableAlignmentFaultFixup,
		ThreadEventPair,
		ThreadQuerySetWin32StartAddress,
		ThreadZeroTlsCell,
		ThreadPerformanceCount,
		ThreadAmILastThread,
		ThreadIdealProcessor,
		ThreadPriorityBoost,
		ThreadSetTlsArrayAddress,
		ThreadIsIoPending,
		ThreadHideFromDebugger
	} THREADINFOCLASS, *PTHREADINFOCLASS;
}


//////////////////////////////////////////////////////////////////////////
// NtCreateFile

// typedef struct _IO_STATUS_BLOCK {
// 	union
// 	{
// 		NTSTATUS Status;
// 		void * Pointer;
// 	};
// 
// 	ULONG_PTR Information;
// } IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

//
// Object Attributes structure
//

// typedef struct _OBJECT_ATTRIBUTES {
// 	ULONG Length;
// 	HANDLE RootDirectory;
// 	PUNICODE_STRING ObjectName;
// 	ULONG Attributes;
// 	PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
// 	PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
// } OBJECT_ATTRIBUTES;
// typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

typedef struct _OBJECT_NAME_INFORMATION { // Information Class 1
	UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

#define OBJ_CASE_INSENSITIVE 0x00000040L

// #define InitializeObjectAttributes( pObjectAttributes, ustrName, aAttribute, rdRootDirectory, sdSecurityDescriptor ) { \
// 	(pObjectAttributes)->Length = sizeof(OBJECT_ATTRIBUTES); \
// 	(pObjectAttributes)->RootDirectory = rdRootDirectory; \
// 	(pObjectAttributes)->Attributes = aAttribute; \
// 	(pObjectAttributes)->ObjectName = ustrName; \
// 	(pObjectAttributes)->SecurityDescriptor = sdSecurityDescriptor; \
// 	(pObjectAttributes)->SecurityQualityOfService = NULL; \
// }

//////////////////////////////////////////////////////////////////////////
// API Define

namespace HookControl{
	bool IsPassCall(void * pBeforeCallAddr,void * pCallAddress);
};

namespace HookHelp{

	inline void MbsToWcs(LPCSTR lpSource, LPWSTR lpDest);
	inline void WcsToMbs(LPCWSTR lpSource, LPSTR lpDest);

	const char * TSTR2STR(const tchar * s, char * b = NULL);
	const tchar * STR2TSTR(const char * s, tchar * b = NULL);
	const tchar * WSTR2TSTR(const wchar_t * s, tchar * b = NULL);
	const wchar_t * TSTR2WSTR(const tchar * s, wchar_t * b = NULL);
	inline const char * WSTR2STR(const wchar_t * wszStrcode, char * szStrBuffer = NULL);
	inline const wchar_t * STR2WSTR(const char * szStrascii, wchar_t * wszStrBuffer = NULL);

	inline wchar_t * SafeCopyString(wchar_t * wszTargetStr, PUNICODE_STRING ustrSourceStr)
	{
		if (NULL == ustrSourceStr || NULL == ustrSourceStr->Buffer)
			return wszTargetStr;

		wcsncpy(wszTargetStr, ustrSourceStr->Buffer, ustrSourceStr->Length);
		wszTargetStr[ustrSourceStr->Length] = L'\0';

		return wszTargetStr;
	}

	BOOL DeleteDirectory(LPCSTR pszDirectoryPath);

	bool CreateImageViewByHWND(LPCTSTR pszFileName, HWND hWebBrowserHandle, unsigned long uWidth = 0, unsigned long uHeight = 0);


	inline bool  IsPassCall(void * pCallAddress)
	{
		bool bIsPassCall = false;

		if (Common::GetModuleHandleByAddr(pCallAddress) == Common::GetModuleHandleByAddr(HookHelp::IsPassCall))
			bIsPassCall = true;

		return bIsPassCall;
	}

	bool StartWindowHook();

	bool LockFile(const tchar * pszFilePath);

	typedef VOID(WINAPI * __pfnRtlInitUnicodeString) (PUNICODE_STRING, PCWSTR);
	typedef BOOL(WINAPI * __pfnCreateProcessInternalW) (HANDLE, LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION, PHANDLE);

	typedef NTSTATUS(NTAPI *__pfnNtResumeThread)(IN HANDLE, OUT PULONG);
	typedef NTSTATUS(NTAPI * __pfnNtQueryInformationThread)(IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, OUT PVOID ThreadInformation, IN ULONG ThreadInformationLength, OUT PULONG ReturnLength OPTIONAL);
	
	typedef NTSTATUS(NTAPI * __pfnLdrLoadDll) (PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);
	typedef NTSTATUS(NTAPI * __pfnNtCreateFile) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
	typedef NTSTATUS(NTAPI * __pfnNtOpenProcess)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
	typedef NTSTATUS(NTAPI * __pfnNtSetValueKey)(HANDLE, PUNICODE_STRING, ULONG, ULONG, PVOID, ULONG);
	typedef NTSTATUS(NTAPI * __pfnNtDeviceIoControlFile) (HANDLE, HANDLE, PVOID, PVOID, PVOID, ULONG, PVOID, ULONG, PVOID, ULONG);
	typedef NTSTATUS(NTAPI * __pfnNtQueryObject) (IN HANDLE ObjectHandle, IN OBJECT_INFORMATION_CLASS ObjectInformationClass, OUT PVOID ObjectInformation, IN ULONG Length, OUT PULONG ResultLength);

	extern NTSTATUS(NTAPI *NtResumeThread)(IN HANDLE ThreadHandle, OUT PULONG SuspendCount OPTIONAL);
	extern NTSTATUS(NTAPI * NtQueryInformationThread)(IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, OUT PVOID ThreadInformation, IN ULONG ThreadInformationLength, OUT PULONG ReturnLength OPTIONAL);

	extern VOID(WINAPI * RtlInitUnicodeString) (_Out_  PUNICODE_STRING DestinationString, _In_opt_  PCWSTR SourceString);
	extern BOOL(WINAPI * CreateProcessInternalW) (HANDLE, LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION, PHANDLE);

	extern NTSTATUS(NTAPI * LdrLoadDll)(IN PWCHAR PathToFile OPTIONAL, IN ULONG Flags OPTIONAL, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle);
	extern NTSTATUS(NTAPI * NtCreateFile)(OUT PHANDLE  FileHandle, IN ACCESS_MASK  DesiredAccess, IN POBJECT_ATTRIBUTES  ObjectAttributes, OUT PIO_STATUS_BLOCK  IoStatusBlock, IN PLARGE_INTEGER  AllocationSize  OPTIONAL, IN ULONG  FileAttributes, IN ULONG  ShareAccess, IN ULONG  CreateDisposition, IN ULONG  CreateOptions, IN PVOID  EaBuffer  OPTIONAL, IN ULONG  EaLength);
	extern NTSTATUS(NTAPI * NtOpenProcess)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
	extern NTSTATUS(NTAPI * NtSetValueKey)(HANDLE, PUNICODE_STRING, ULONG, ULONG, PVOID, ULONG);
	extern NTSTATUS(NTAPI * NtDeviceIoControlFile) (HANDLE, HANDLE, PVOID, PVOID, PVOID, ULONG, PVOID, ULONG, PVOID, ULONG);
	extern NTSTATUS(NTAPI * NtQueryObject) (IN HANDLE ObjectHandle, IN OBJECT_INFORMATION_CLASS ObjectInformationClass, OUT PVOID ObjectInformation, IN ULONG Length, OUT PULONG ResultLength);


	bool NtPathToDosPath(const tchar * pszNtPath, tchar * pszDosPath);

	bool GetFullPathForHandle(IN HANDLE ObjectHandle, OUT WCHAR* strFullPath);

	BOOL GetFullPathForObjectAttributes(IN POBJECT_ATTRIBUTES ObjectAttributes, OUT WCHAR* strPath);

// 	inline bool GetFullPathForHandle(HANDLE hHandle, OUT WCHAR* strPath)
// 	{
// 		unsigned long uBufferSize = 0x1000;
// 		HANDLE hHeap = ::GetProcessHeap();
// 
// 		POBJECT_NAME_INFORMATION pName = (POBJECT_NAME_INFORMATION)::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, uBufferSize);
// 		NTSTATUS ntStatus = NtQueryObject(hHandle, ObjectNameInformation, (PVOID)pName, uBufferSize, &uBufferSize);
// 		DWORD i = 1;
// 		while (ntStatus == STATUS_INFO_LEN_MISMATCH)
// 		{
// 			pName = (POBJECT_NAME_INFORMATION)::HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, (LPVOID)pName, uBufferSize * i);
// 			ntStatus = NtQueryObject(hHandle, ObjectNameInformation, (PVOID)pName, uBufferSize, NULL);
// 			i++;
// 		}
// 
// 		wcsncpy(strPath, pName->Name.Buffer,pName->Name.Length);
// 		strPath[pName->Name.Length] = L'\0';
// 
// 		HeapFree(hHeap, HEAP_ZERO_MEMORY, pName);
// 
// 		return (ntStatus == STATUS_SUCCESS);
// 	}

	/* 获取桌面路径 */
	bool GetDesktopPath(char *pszDesktopPath);

	bool CreateLinkFile(const tchar * szStartAppPath, const tchar * szAddCmdLine, const tchar * szDestLnkPath, const tchar * szIconPath = NULL);


	//
	// 启用某个账户对某个文件(夹)的所有操作权限
	// pszPath: 文件(夹)路径
	// pszAccount: 账户名称
	// AccessPermissions：权限名称
	// 通用读（GENERIC_READ）
	// 通用写（GENERIC_WRITE）
	// 通用执行（GENERIC_EXECUTE）
	// 通用所有（GENERIC_ALL）
	// 注册表所有(KEY_ALL_ACCESS)
	// 注册表执行（KEY_EXECUTE）
	// 注册表写（KEY_WRITE）
	// 注册表读（KEY_READ）
	//
	BOOL  EnableAccountPrivilege(LPCTSTR pszPath, LPCTSTR pszAccount, DWORD AccessPermissions = GENERIC_READ | GENERIC_EXECUTE, ACCESS_MODE AccessMode = DENY_ACCESS, SE_OBJECT_TYPE dwType = SE_FILE_OBJECT);

	DWORD GetProcessIdByName(LPCTSTR pszProcessName);

	bool FileExists(__in const tchar * pszFileFullName);


	bool PathRemoveFileName(__inout tchar * pszFilePath);
	bool PathAddFileName(__inout tchar * pszFilePath, const tchar * pszFileName);
	bool PathRenameFileName(__inout tchar * pszFilePath, const tchar * pszFileName);
	bool RenameFile(_In_ LPCTSTR lpExistingFileName, _In_opt_ LPCTSTR lpNewFileName);

	typedef bool(*PFINDFILE)(LPCTSTR lpFilePath, LPCTSTR lpFileName, PVOID pParameter);
	typedef bool(*PFINDFOLDER)(LPCTSTR lpFolderPath, LPCTSTR lpFolderName, PVOID pParameter);

	//////////////////////////////////////////////////////////////////////////
	//  Explain:  查找文件 FindFiles（）
	//  lpFilePath: 欲寻找文件所在文件目录
	//  lpFileName：欲寻找文件名称
	//  pCallbackFindFile：找到文件回调方法
	//  pCallbackFindFolder：找到目录回调方法
	//  bFuzzy：是否进行模糊查询
	//  bDirectory：是否查找子目录
	//	pFileParameter: 传给文件回调方法的参数
	//	pDirectoryParameter: 传给目录回调方法的参数
	//  Return：返回查找到的数量
	ULONGLONG FindFiles(LPCTSTR lpFilePath, LPCTSTR lpFileName, PFINDFILE pCallbackFindFile = NULL, PFINDFOLDER pCallbackFindFolder = NULL, BOOL bFuzzy = FALSE, BOOL bDirectory = FALSE, PVOID pFileParameter = NULL, PVOID pDirectoryParameter = NULL);


	bool OutFile(__in const tchar * pszFileName, __in  void * pData, __in unsigned long uDataSize);


	bool InternalFindFile(LPCTSTR sFindPath, LPCTSTR sFindFileName, ULONGLONG &uCountFolder, ULONGLONG &uCountFile, PFINDFILE pCallbackFindFile, PFINDFOLDER pCallbackFindFolder, BOOL bFuzzy, BOOL bDirectory, PVOID pFileParameter, PVOID pDirectoryParameter);
	bool InternalFindFolder(LPCTSTR sPath, LPCTSTR sFindFileName, ULONGLONG &uCountFolder, ULONGLONG &uCountFile, PFINDFILE pCallbackFindFile, PFINDFOLDER pCallbackFindFolder, BOOL bFuzzy, BOOL bDirectory, PVOID pFileParameter, PVOID pDirectoryParameter);

};



