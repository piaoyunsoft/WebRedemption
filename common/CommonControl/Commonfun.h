#pragma once
#include <windows.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <iphlpapi.h>
#include <tchar.h>
#include "Commondef.h"

#pragma comment(lib,"shlwapi.lib")

namespace Common{

	static VOID SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo)
	{
		if (NULL == lpSystemInfo) return;
		typedef VOID(WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
		LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)::GetProcAddress(::GetModuleHandle(_T("kernel32")), "GetNativeSystemInfo");;
		if (NULL != fnGetNativeSystemInfo)
		{
			fnGetNativeSystemInfo(lpSystemInfo);
		}
		else
		{
			GetSystemInfo(lpSystemInfo);
		}
	}

	static int GetSystemBits()
	{
		SYSTEM_INFO si;
		SafeGetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		{
			return 64;
		}
		return 32;
	}
  
	static int GetVersionInfo(char* systeminfo)
	{

		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx(&osvi)) {
			return 0;
		}

		//判断版本  
		if (osvi.dwMajorVersion == 5) {

			switch (osvi.dwMinorVersion) {
			case 0:
				//wcscpy_s(systeminfo,_T("Windows 2000"));  
				strcpy(systeminfo, "Windows 2000");
				break;
			case 1:
				strcpy(systeminfo, "Windows XP");
				break;
			case 2:
				strcpy(systeminfo, "Windows Server 2003");
				break;
			default:
				strcpy(systeminfo, "Unknown");
				break;
			}

		}
		else if (osvi.dwMajorVersion == 6) {

			switch (osvi.dwMinorVersion) {
			case 0:
				strcpy(systeminfo, "Windows Vista");
				break;
			case 1:
				strcpy(systeminfo, "Windows 7");
				break;
			case 2:
				strcpy(systeminfo, "Windows 8");
				break;
			default:
				strcpy(systeminfo, "Unknown");
				break;
			}

		}
		else {
			strcpy(systeminfo, "Unknown");
		}

		return osvi.dwMajorVersion;
	}

	static void * GetThreadStartAddr(DWORD dwThreadID = 0)
	{
		typedef enum _THREADINFOCLASS {
			ThreadBasicInformation,
			ThreadTimes,
			ThreadPriority,
			ThreadBasePriority,
			ThreadAffinityMask,
			ThreadImpersonationToken,
			ThreadDescriptorTableEntry,
			ThreadEnableAlignmentFaultFixup,
			ThreadEventPair_Reusable,
			ThreadQuerySetWin32StartAddress,
			ThreadZeroTlsCell,
			ThreadPerformanceCount,
			ThreadAmILastThread,
			ThreadIdealProcessor,
			ThreadPriorityBoost,
			ThreadSetTlsArrayAddress,
			ThreadIsIoPending,
			ThreadHideFromDebugger,
			ThreadBreakOnTermination,
			MaxThreadInfoClass
		} THREADINFOCLASS;

		typedef NTSTATUS (WINAPI *NtQueryInformationThreadProc)(_In_       HANDLE ThreadHandle,_In_       THREADINFOCLASS ThreadInformationClass,_Inout_    PVOID ThreadInformation,_In_       ULONG ThreadInformationLength,_Out_opt_  PULONG ReturnLength);

		NtQueryInformationThreadProc   NtQueryInformationThread = NULL;
		NtQueryInformationThread = (NtQueryInformationThreadProc)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationThread");

		HANDLE  hThread = NULL;
		PVOID   pvStart = NULL;

		if (0 == dwThreadID)
			dwThreadID = ::GetCurrentThreadId();

		hThread = OpenThread(THREAD_QUERY_INFORMATION | THREAD_TERMINATE, FALSE, dwThreadID);

		NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &pvStart, sizeof(pvStart), NULL);

		CloseHandle(hThread);

		return pvStart;
	}

	static void * GetResourceContent(HINSTANCE hInstance, size_t sizeResourceID, const char * pszReosurceType,size_t * __out psizeResourceSize = NULL) {
		HRSRC hRsrc = ::FindResourceA(hInstance, MAKEINTRESOURCEA(sizeResourceID), pszReosurceType);
		if (NULL == hRsrc) {
			return NULL;
		}

		if (NULL == psizeResourceSize) {
			psizeResourceSize = &sizeResourceID;
		}

		*psizeResourceSize = ::SizeofResource(hInstance, hRsrc);

		HGLOBAL hGlobal = ::LoadResource(hInstance, hRsrc);
		if (NULL == hGlobal) {
			return NULL;
		}

		void * pResource = malloc(*psizeResourceSize);
		memcpy(pResource, LockResource(hGlobal), *psizeResourceSize);

		FreeResource(hGlobal);

		return pResource;
	}

	static bool IsCreateEvent(LPCTSTR pszMutexName,bool bIsCreate = true)
	{
		bool bRet = false;
		HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, pszMutexName);

		do 
		{
			if (NULL == hEvent)
				break;

			bRet = true;
		} while (false);
		
		if (bIsCreate)
			::CreateEvent(NULL, FALSE, FALSE, pszMutexName);

		return bRet;
	}

	static HANDLE CreateMutex(LPCTSTR pszMutexName)
	{
		return  ::CreateMutex(NULL, FALSE, pszMutexName);
	}

	static bool IsCreateMutex(LPCTSTR pszMutexName, bool bIsCreate = true)
	{
		bool bIsOK = false;
		HANDLE hMutex = NULL;

		if (NULL != (hMutex = OpenMutex(SYNCHRONIZE, FALSE, pszMutexName)))
		{
			CloseHandle(hMutex);
			return true;
		}

		do 
		{
			if (false == bIsCreate)
				break;

			if (NULL == CreateMutex(pszMutexName))
				bIsOK = true;

		} while (false);

		return bIsOK;
	}

	static bool CreateAtom(LPCTSTR pszAtomName)
	{
		return 0 != GlobalAddAtom(pszAtomName);
	}

	static bool DeleteAtom(LPCTSTR pszAtomName)
	{
		return 0 == GlobalDeleteAtom(::GlobalFindAtom(pszAtomName));
	}

	static bool IsCreateAtom(LPCTSTR pszAtomName, bool bIsCreate = false)
	{
		if (::GlobalFindAtom(pszAtomName) != 0)
			return true;

		if (bIsCreate)
			CreateAtom(pszAtomName);

		return false;
	}

	struct internal_share_map_info
	{
		unsigned long Len;
		byte Data[1];
	};

	static HANDLE SetBufferToShareMap(const tchar * pszShareMapName, void * pBuffer, int nBufferlen)
	{
		HANDLE hMapping = NULL;
		HANDLE hOutHandle = NULL;
		internal_share_map_info * ptmiSharemapInfo = NULL;

		do
		{
			hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(internal_share_map_info)+nBufferlen, pszShareMapName);

			if (hMapping == NULL)
				break;

			ptmiSharemapInfo = (internal_share_map_info *)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(internal_share_map_info)+nBufferlen);

			if (ptmiSharemapInfo == NULL)
				break;

			hOutHandle = hMapping;
			hMapping = NULL;

			ptmiSharemapInfo->Len = nBufferlen;

			memcpy((PVOID)ptmiSharemapInfo->Data, pBuffer, nBufferlen);
		} while (false);

		if (ptmiSharemapInfo)
			UnmapViewOfFile(ptmiSharemapInfo);

		if (hMapping)
			CloseHandle(hMapping);

		return hOutHandle;
	}

	static bool GetBufferToShareMap(const tchar * pszShareMapName,void ** pBuffer, int * nBufferlen = NULL)
	{
		bool bIsSuccess = false;
		HANDLE hMapping = NULL;
		internal_share_map_info * ptmiSharemapInfo = NULL;

		do
		{
			hMapping = OpenFileMapping(FILE_MAP_READ, FALSE, pszShareMapName);

			if (hMapping == NULL)
				break;

			ptmiSharemapInfo = (internal_share_map_info *)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);

			if (ptmiSharemapInfo == NULL)
				break;

			bIsSuccess = true;
			*pBuffer = malloc(ptmiSharemapInfo->Len);

			memcpy(*pBuffer, ptmiSharemapInfo->Data, ptmiSharemapInfo->Len);
		} while (false);

		if (ptmiSharemapInfo)
			UnmapViewOfFile(ptmiSharemapInfo);

		if (hMapping)
			CloseHandle(hMapping);

		return bIsSuccess;
	}

	static bool CreateProcess(const char * pszFileFullPath, const char * pszCommandline = NULL, const char * pszCurrentDirectory = NULL, int nWait = 0)
	{
		bool bIsSuccess = false;
		char * pszCommand = NULL;
		STARTUPINFOA siStratupInfo = { 0 };
		PROCESS_INFORMATION piProcessInformation = { 0 };

		siStratupInfo.cb = sizeof(STARTUPINFO);

		if (pszCurrentDirectory && pszCurrentDirectory[0] == '\0')
			pszCurrentDirectory = NULL;

		if (pszCommandline)
		{
			pszCommand = (char *)malloc(0x800);
			sprintf_s(pszCommand, 0x800, "\"%s\" %s", pszFileFullPath, pszCommandline);
		}

		if (bIsSuccess = (TRUE == ::CreateProcessA(NULL, pszCommand, NULL, NULL, FALSE, 0, NULL, pszCurrentDirectory, &siStratupInfo, &piProcessInformation)))
		{
			WaitForSingleObject(piProcessInformation.hProcess, nWait);

			CloseHandle(piProcessInformation.hProcess);
		}

		free(pszCommand);

		return bIsSuccess;
	}

	static unsigned long IP2LONG(const char *ip)
	{
		char szAddress[MAX_IP4_LEN] = { 0 };

		sscanf(ip, "%d.%d.%d.%d", szAddress, szAddress + 1, szAddress + 2, szAddress + 3);

		return *((unsigned long*)(&szAddress));
	}

	static bool GetMACAddr(char * pszMACBuffer, char * pszIPBuffer = NULL)
	{

		unsigned long uBufferLength = 0;//得到结构体大小,用于GetAdaptersInfo参数
		PIP_ADAPTER_INFO pIpAdapterInfo = NULL; //PIP_ADAPTER_INFO结构体指针存储本机网卡信息

		unsigned long uStatus = GetAdaptersInfo(pIpAdapterInfo, &uBufferLength);//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量

		if (ERROR_BUFFER_OVERFLOW == uStatus) //如果函数返回的是ERROR_BUFFER_OVERFLOW则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
		{
			pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[uBufferLength];
			uStatus = GetAdaptersInfo(pIpAdapterInfo, &uBufferLength);//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		}

		if (ERROR_SUCCESS != uStatus)
			return false;

		PIP_ADAPTER_INFO pCurIpAdapterInfo = pIpAdapterInfo; //PIP_ADAPTER_INFO结构体指针存储本机网卡信息

		while (pCurIpAdapterInfo && MIB_IF_TYPE_ETHERNET == pCurIpAdapterInfo->Type)
		{
			if (pszMACBuffer)
			{
				for (UINT i = 0; i < 6; i++, pszMACBuffer += 3)
					sprintf(pszMACBuffer, "%02x%c", pIpAdapterInfo->Address[i], (i == pIpAdapterInfo->AddressLength - 1) ? '\0' : '-');
			}

			if (pszIPBuffer)
				memcpy(pszIPBuffer, pIpAdapterInfo->IpAddressList.IpAddress.String, 16);

			break;
		}

		if (pIpAdapterInfo)
			delete pIpAdapterInfo;

		return true;
	}

	static void MACConverter(char * pszMACString, byte * pszMACBuffer)
	{
		for (UINT i = 0; i < 6; i++, pszMACBuffer += 3)
			sscanf(pszMACString, "%02x-%02x-%02x-%02x-%02x-%02x", pszMACBuffer, pszMACBuffer + 1, pszMACBuffer + 2, pszMACBuffer + 3, pszMACBuffer + 4, pszMACBuffer + 5);

	}

	static void MACConverter(byte * pszMACBuffer, char * pszMACString)
	{
		for (UINT i = 0; i < 6; i++, pszMACString += 3)
			sprintf(pszMACString, "%02x%c", pszMACBuffer[i], (i == 5) ? '\0' : '-');
	}

	enum ENUM_RANDOM_TYPE
	{
		E_RANMOMTYPE_STRING,
		E_RANMOMTYPE_INTEGER,
		E_RANMOMTYPE_RANDOM,
		E_RANMOMTYPE_UPR_STRING,
		E_RANMOMTYPE_LWR_STRING,
	};

	static tchar * GetRandomString(tchar * pszSourctString, unsigned long uLength, ENUM_RANDOM_TYPE eRandomType)
	{
		BOOL bIsMix = FALSE;
		BOOL bIsStringupr = FALSE;
		BOOL bIsStringMix = FALSE;

		unsigned long uRandom = 0;
		unsigned long uCharBase = 0;

		POINT ptPoint = { 0 };
		srand((int)::GetActiveWindow() + ::GetCaretBlinkTime() + (int)::GetCursorPos(&ptPoint) + ptPoint.x + ptPoint.y + ::GetCurrentThreadId() + (int)::GetOpenClipboardWindow());

		for (unsigned long i = 0; i < uLength; i++)
		{
			switch (eRandomType)
			{
			case E_RANMOMTYPE_RANDOM:
				bIsMix = random(2);
				if (bIsMix)
				{
					uCharBase = '0';
					bIsStringMix = FALSE;
					uRandom = random(10);
				}
				else
				{
					bIsStringMix = TRUE;
					uRandom = random(26);
				}
				break;
			case E_RANMOMTYPE_UPR_STRING:
				uCharBase = 'A';
			case E_RANMOMTYPE_LWR_STRING:
				uCharBase = 'a';
				uRandom = random(26);
				break;
			case E_RANMOMTYPE_STRING:
				bIsStringMix = TRUE;
				uRandom = random(26);
				break;
			case E_RANMOMTYPE_INTEGER:
				uCharBase = '0';
				bIsStringMix = FALSE;
				uRandom = random(10);
				break;
			default:
				break;
			}

			if (bIsStringMix)
				uCharBase = (1 == random(2)) ? 'A' : 'a';

			pszSourctString[i] = char(uCharBase + uRandom);
		}

		return pszSourctString;
	}

	static bool PathRemoveFileName(__inout tchar * pszFilePath)
	{
		return TRUE == ::PathRemoveFileSpec(pszFilePath);
	}

	static bool PathAddFileName(__inout tchar * pszFilePath, const tchar * pszFileName)
	{
		::PathAddBackslash(pszFilePath);
		_tcscat(pszFilePath, pszFileName);
		return true;
	}

	static bool PathRenameFileName(__inout tchar * pszFilePath, const tchar * pszFileName)
	{
		return PathRemoveFileName(pszFilePath) && PathAddFileName(pszFilePath, pszFileName);
	}

	static bool PathRenameFile(_In_ LPCTSTR lpExistingFileName, _In_opt_ LPCTSTR lpNewFileName)
	{
		TCHAR szNewFileName[MAX_PATH + 1] = { 0 };

		_tcscpy(szNewFileName, lpExistingFileName);
		PathRenameFileName(szNewFileName, lpNewFileName);

		return TRUE == MoveFileEx(lpExistingFileName, szNewFileName, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}

	static byte * GetFile(const char * pszFilePath, int * pnFilesize)
	{
		byte * pFuleContext = NULL;

		FILE * pfHandle = fopen(pszFilePath, "rb");

		if (NULL == pfHandle)
			return NULL;

		fseek(pfHandle, 0, SEEK_END);
		*pnFilesize = ftell(pfHandle);
		fseek(pfHandle, 0, SEEK_SET);

		pFuleContext = (byte *)malloc(*pnFilesize);

		fread(pFuleContext, *pnFilesize, 1, pfHandle);

		fclose(pfHandle);

		return pFuleContext;
	}

	static bool CreateDir(const tchar *ptszPath)
	{
		bool bIsOK = true;
		tchar tszDirectory[MAX_PATH + 1] = { 0 };

		_tcscpy(tszDirectory, ptszPath);

		tchar * ptszCurrnetDiretory = tszDirectory;

		while (ptszCurrnetDiretory = _tcschr(++ptszCurrnetDiretory, _T('/')))
			ptszCurrnetDiretory[0] = _T('\\');

		ptszCurrnetDiretory = tszDirectory;

		while (ptszCurrnetDiretory = _tcschr(++ptszCurrnetDiretory, _T('\\')))
		{
			ptszCurrnetDiretory[0] = _T('\0');

			if (FALSE == CreateDirectory(tszDirectory, NULL) && ERROR_ALREADY_EXISTS != GetLastError())
				bIsOK = false;

			ptszCurrnetDiretory[0] = _T('\\');
		}

		if (TRUE == CreateDirectory(tszDirectory, NULL) || ERROR_ALREADY_EXISTS == GetLastError())
			bIsOK = true;

		return bIsOK;
	}

	static bool SetFile(const char * pszFilePath, const void * pFileData, int nFilesize)
	{
		char szSaveFilePath[MAX_PATH + 1] = { 0 };

		strcpy_s(szSaveFilePath, pszFilePath);
		ExpandEnvironmentStringsA(pszFilePath, szSaveFilePath, MAX_PATH);

		FILE * pfHandle = fopen(szSaveFilePath, "wb");

		if (NULL == pfHandle)
			return false;

		fwrite(pFileData, nFilesize, 1, pfHandle);

		fclose(pfHandle);

		return true;
	}

	static bool PathFileExists(const tchar * pszFileFullName)
	{
		tchar szFilePathBuffer[MAX_PATH + 1] = { 0 };

		_tcscpy_s(szFilePathBuffer, pszFileFullName);
		ExpandEnvironmentStrings(pszFileFullName, szFilePathBuffer, MAX_PATH);

		return TRUE == ::PathFileExists(szFilePathBuffer);
	}

	static bool MoveFile(_In_ LPCTSTR lpExistingFileName, _In_opt_ LPCTSTR lpNewFileName)
	{
		return TRUE == MoveFileEx(lpExistingFileName, lpNewFileName, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}

	static HINSTANCE GetModuleHandleByAddr(const void * pAddr)
	{
		MEMORY_BASIC_INFORMATION miMemoryInfo = { 0 };

		VirtualQuery(pAddr, &miMemoryInfo, sizeof(MEMORY_BASIC_INFORMATION));

		return (HMODULE)miMemoryInfo.AllocationBase;
	}

	static const tchar * GetPerformDirectory()
	{
		static tchar szPerformDirectory[MAX_PATH + 1] = { 0 };

		if (_T('\0') == szPerformDirectory)
		{
			if (0 == ::GetModuleFileName(Common::GetModuleHandleByAddr(Common::GetModuleHandleByAddr), szPerformDirectory, MAX_PATH))
				return NULL;

			tchar * pszFileNameStart = _tcsrchr(szPerformDirectory, _T('\\'));

			if (NULL == pszFileNameStart)
				pszFileNameStart = _tcsrchr(szPerformDirectory, _T('\\'));

			if (NULL == pszFileNameStart)
				return NULL;

			pszFileNameStart[1] = _T('\0');
		}

		return szPerformDirectory;
	}

	static DWORD GetProcessIDFromProcessName(LPCTSTR name)
	{
		DWORD dwProcessId = 0;
		PROCESSENTRY32 pe32 = { 0 };
		HANDLE hProcessSnap = INVALID_HANDLE_VALUE;

		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32First(hProcessSnap, &pe32))
			return 0;

		do
		{
			if (_tcsicmp(pe32.szExeFile, name) == 0)
			{
				dwProcessId = pe32.th32ProcessID;
				break;
			}

			pe32.dwSize = sizeof(PROCESSENTRY32);
		} while (Process32Next(hProcessSnap, &pe32));

		CloseHandle(hProcessSnap);

		return dwProcessId;
	}

	static void * GetProcAddress(LPCTSTR pszDllName, LPCSTR pszProcName)
	{

		HINSTANCE hModule = (HINSTANCE)GetModuleHandle(pszDllName);

		if (NULL == hModule)
			hModule = LoadLibrary(pszDllName);

		return ::GetProcAddress(hModule, pszProcName);

	}

	static const tchar * GetCurrentProcessName()
	{
		bool bRet = false;
		static tchar szProcessPath[MAX_PATH + 1] = { 0 };

		if (::GetModuleFileName(NULL, szProcessPath, MAX_PATH))
		{
			const tchar * pszCurrentProcessName = _tcsrchr(szProcessPath, _T('\\'));

			if (pszCurrentProcessName)
				return ++pszCurrentProcessName;
		}

		return NULL;
	}

	static bool IsCurrentProcess(unsigned long uProcessId)
	{
		return GetCurrentProcessId() == uProcessId;
	}

	static bool IsCurrentProcess(const tchar * pszProcessName)
	{
		if (NULL == pszProcessName)
			return false;

		const tchar * pszCurrentProcessName = GetCurrentProcessName();

		if (NULL == pszCurrentProcessName) {
			return false;
		}

		return  (0 == _tcsicmp(pszCurrentProcessName, pszProcessName));
	}

	//////////////////////////////////////////////////////////////////////////
	// 默认回调函数
	static bool CallbackFindFolder(LPCTSTR lpFilePath, LPCTSTR lpFileName, PVOID pParameter)
	{
		return FALSE;
	}

	static bool CallbackFindFile(LPCTSTR lpFilePath, LPCTSTR lpFileName, PVOID pParameter)
	{
		return FALSE;
	}

	typedef bool(*PFINDFILE)(LPCTSTR lpFilePath, LPCTSTR lpFileName, PVOID pParameter);
	typedef bool(*PFINDFOLDER)(LPCTSTR lpFolderPath, LPCTSTR lpFolderName, PVOID pParameter);

	static bool InternalFindFile(LPCTSTR sFindPath, LPCTSTR sFindFileName, ULONGLONG &uCountFolder, ULONGLONG &uCountFile, PFINDFILE pCallbackFindFile, PFINDFOLDER pCallbackFindFolder, BOOL bFuzzy, BOOL bDirectory, PVOID pFileParameter, PVOID pDirectoryParameter)
	{
		HANDLE hFind;
		BOOL fFinished = FALSE;
		WIN32_FIND_DATA FindFileData;
		TCHAR sPath[MAX_PATH], sFormatFileName[MAX_PATH + 3] = _T("*");

		lstrcpy(sFormatFileName, sFindPath);

		if (bFuzzy){
			lstrcat(sFormatFileName, _T("\\*"));
			lstrcat(sFormatFileName, sFindFileName);
			lstrcat(sFormatFileName, _T("*"));
		}
		else {
			PathAddBackslash(sFormatFileName);
			lstrcat(sFormatFileName, sFindFileName);
		}

		hFind = FindFirstFile(sFormatFileName, &FindFileData);

		if (hFind == INVALID_HANDLE_VALUE) {
			return FALSE;
		}
		else  {
			while (!fFinished)
			{
				lstrcpy(sPath, sFindPath);
				PathAddBackslash(sPath);
				//lstrcat(sPath, _T("//")); 
				//PathAddExtension(sPath,FindFileData.cFileName);
				//lstrcat(sPath, FindFileData.cFileName); 

				if (FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)
				{
					if (0 != lstrcmp(FindFileData.cFileName, _T(".")) && 0 != lstrcmp(FindFileData.cFileName, _T(".."))) {
						if (pCallbackFindFolder(sPath, FindFileData.cFileName, pDirectoryParameter))
							++uCountFolder;
					}
				}
				else
				{
					if (pCallbackFindFile(sPath, FindFileData.cFileName, pFileParameter))
						++uCountFile;
				}

				if (!FindNextFile(hFind, &FindFileData)) {
					if (GetLastError() == ERROR_NO_MORE_FILES)
						fFinished = TRUE;
					else
						break;
				}
			}

			FindClose(hFind);
		}

		return TRUE;
	}

	static bool InternalFindFolder(LPCTSTR sPath, LPCTSTR sFindFileName, ULONGLONG &uCountFolder, ULONGLONG &uCountFile, PFINDFILE pCallbackFindFile, PFINDFOLDER pCallbackFindFolder, BOOL bFuzzy, BOOL bDirectory, PVOID pFileParameter, PVOID pDirectoryParameter)
	{
		TCHAR sTemp[MAX_PATH];
		TCHAR sFormatFileName[MAX_PATH];
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		BOOL fFinished = FALSE;

		InternalFindFile(sPath, sFindFileName, uCountFolder, uCountFile, pCallbackFindFile, pCallbackFindFolder, bFuzzy, bDirectory, pFileParameter, pDirectoryParameter);

		lstrcpy(sFormatFileName, sPath);
		lstrcat(sFormatFileName, _T("\\*"));
		hFind = FindFirstFile(sFormatFileName, &FindFileData);

		if (hFind == INVALID_HANDLE_VALUE)  {
			return FALSE;
		}
		else {
			while (!fFinished)
			{
				if (FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)
				{
					if (0 != lstrcmp(FindFileData.cFileName, _T(".")) && 0 != lstrcmp(FindFileData.cFileName, _T("..")) && bDirectory){
						lstrcpy(sTemp, sPath);
						PathAddBackslash(sTemp);
						PathAddExtension(sTemp, FindFileData.cFileName);
						InternalFindFolder(sTemp, sFindFileName, uCountFolder, uCountFile, pCallbackFindFile, pCallbackFindFolder, bFuzzy, bDirectory, pFileParameter, pDirectoryParameter);
					}
				}

				if (!FindNextFile(hFind, &FindFileData)){
					if (GetLastError() == ERROR_NO_MORE_FILES)
						return TRUE;
					else
						return FALSE;
				}
			}

			FindClose(hFind);
		}

		return TRUE;
	}

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
	static ULONGLONG FindFiles(LPCTSTR lpFilePath, LPCTSTR lpFileName, PFINDFILE pCallbackFindFile = NULL, PFINDFOLDER pCallbackFindFolder = NULL, BOOL bFuzzy = FALSE, BOOL bDirectory = FALSE, PVOID pFileParameter = NULL, PVOID pDirectoryParameter = NULL)
	{
		ULONGLONG    uCountFolder(0);
		ULONGLONG    uCountFile(0);
		if (NULL == pCallbackFindFile || NULL == pCallbackFindFolder)
		{
			if (NULL == pCallbackFindFile && NULL == pCallbackFindFolder)
				InternalFindFolder(lpFilePath, lpFileName, uCountFolder, uCountFile, CallbackFindFile, CallbackFindFolder, bFuzzy, bDirectory, pFileParameter, pDirectoryParameter);
			else if (NULL == pCallbackFindFile)
				InternalFindFolder(lpFilePath, lpFileName, uCountFolder, uCountFile, CallbackFindFile, pCallbackFindFolder, bFuzzy, bDirectory, pFileParameter, pDirectoryParameter);
			else
				InternalFindFolder(lpFilePath, lpFileName, uCountFolder, uCountFile, pCallbackFindFile, CallbackFindFolder, bFuzzy, bDirectory, pFileParameter, pDirectoryParameter);

			return uCountFolder + uCountFile;
		}
		InternalFindFolder(lpFilePath, lpFileName, uCountFolder, uCountFile, pCallbackFindFile, pCallbackFindFolder, bFuzzy, bDirectory, pFileParameter, pDirectoryParameter);

		return uCountFolder + uCountFile;
	}

	///////////////////////////////////////////////////////////////////////
	//unicode  ascii
	static char * WSTR2STR(const wchar_t * wszStrcode, char * szStrBuffer, int * uDestlen = NULL)
	{
		int iDestlenbuf = MAX_PATH;
		char * szStrReturn = NULL;

		int iAsciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wszStrcode, -1, NULL, 0, NULL, NULL);

		if (iAsciisize == ERROR_NO_UNICODE_TRANSLATION || iAsciisize == 0)
			return NULL;

		if (NULL == uDestlen)
			uDestlen = &iDestlenbuf;

		if (NULL == szStrBuffer)
		{
			*uDestlen = iAsciisize;
			szStrBuffer = (char *)malloc(iAsciisize * sizeof(char));
		}

		if (*uDestlen >= iAsciisize)
		{
			szStrReturn = szStrBuffer;
			iAsciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wszStrcode, -1, szStrBuffer, *uDestlen, NULL, NULL);
		}

		*uDestlen = iAsciisize;

		return szStrReturn;
	}

	///////////////////////////////////////////////////////////////////////
	//ascii  Unicode
	static wchar_t * STR2WSTR(const char * szStrascii, wchar_t * wszStrBuffer, int * uDestlen = NULL)
	{
		int iDestlenbuf = MAX_PATH;
		wchar_t * wszStrReturn = NULL;

		int iWidesize = MultiByteToWideChar(CP_ACP, 0, szStrascii, -1, NULL, 0);

		if (iWidesize == ERROR_NO_UNICODE_TRANSLATION || iWidesize == 0)
			return NULL;

		if (NULL == uDestlen)
			uDestlen = &iDestlenbuf;

		if (NULL == wszStrBuffer)
		{
			*uDestlen = iWidesize;
			wszStrBuffer = (wchar_t *)malloc(iWidesize * sizeof(wchar_t));
		}

		if (*uDestlen >= iWidesize)
		{
			wszStrReturn = wszStrBuffer;
			iWidesize = ::MultiByteToWideChar(CP_ACP, 0, szStrascii, -1, wszStrReturn, iWidesize);
		}

		*uDestlen = iWidesize;

		return wszStrReturn;
	}
	
	static const wchar_t * TSTR2WSTR(const tchar * s, wchar_t * b, int * uDestlen = NULL)
	{
#ifdef UNICODE
		if (NULL == b)
			return s;

		wcscpy(b, s);
		return b;
#else
		return STR2WSTR(s, b, uDestlen);
#endif
	}

	static const char * TSTR2STR(const tchar * s, char * b, int * uDestlen)
	{
#ifdef UNICODE
		return WSTR2STR(s, b, uDestlen);
#else
		if (NULL == b)
			return s;

		strcpy(b, s);
		return b;
#endif
	}
	
	static const tchar * STR2TSTR(const char * s, tchar * b, int * uDestlen = NULL)
	{
#ifdef UNICODE
		return STR2WSTR(s, b, uDestlen);
#else
		if (NULL == b)
			return s;

		_tcscpy(b, s);
		return b;
#endif
	}

	static const tchar * WSTR2TSTR(const wchar_t * s, tchar * b, int * uDestlen = NULL)
	{
#ifdef UNICODE
		if (NULL == b)
			return s;

		_tcscpy(b, s);
		return b;
#else
		return WSTR2STR(s, b, uDestlen);
#endif
	}
}