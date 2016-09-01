#include <windows.h>
#include <tchar.h>
#include "HookHelp.h"
#include <tlhelp32.h>
#include <shlwapi.h>
#include <stdio.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <Aclapi.h>
#include "..\CommonControl\Commondef.h"
#pragma comment(lib,"shlwapi.lib")

#pragma data_seg(".HookHelp_ShareData")
HHOOK     g_HookHelpWindowHook = NULL;
#pragma data_seg()
#pragma comment(linker,"/Section:.HookHelp_ShareData,RWS")

namespace HookHelp {

	char * g_szBuffer = NULL;
	wchar_t * g_wszBuffer = NULL;
	int g_uszBufferSize = 0;
	int g_uwszBufferSize = 0;
	CRITICAL_SECTION g_csStrBufferLock = { 0 };

	bool InitializeHookHelp()
	{
		InitializeCriticalSection(&g_csStrBufferLock);
		return true;
	}
	bool bLock = InitializeHookHelp();

	///////////////////////////////////////////////////////////////////////
	//unicode  ascii

	const char * WSTR2STR(const wchar_t * wszStrcode, char * szStrBuffer)
	{
		char * szStrReturn = szStrBuffer;
		int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wszStrcode, -1, NULL, 0, NULL, NULL);

		if (asciisize == ERROR_NO_UNICODE_TRANSLATION || asciisize == 0)
			return NULL;

		EnterCriticalSection(&g_csStrBufferLock);
		if (NULL == szStrBuffer)
		{
			if (g_uszBufferSize < asciisize)
			{
				delete g_szBuffer;
				g_szBuffer = new char[asciisize];
				g_uszBufferSize = asciisize;
			}
			szStrReturn = g_szBuffer;
		}
		int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wszStrcode, -1, szStrReturn, asciisize, NULL, NULL);
		LeaveCriticalSection(&g_csStrBufferLock);

		if (convresult != asciisize)
			return szStrReturn;

		return szStrReturn;
	}

	///////////////////////////////////////////////////////////////////////
	//ascii  Unicode

	const wchar_t * STR2WSTR(const char * szStrascii, wchar_t * wszStrBuffer)
	{
		wchar_t * wszStrReturn = wszStrBuffer;
		int widesize = MultiByteToWideChar(CP_ACP, 0, szStrascii, -1, NULL, 0);

		if (widesize == ERROR_NO_UNICODE_TRANSLATION || widesize == 0)
			return NULL;

		EnterCriticalSection(&g_csStrBufferLock);
		if (NULL == wszStrBuffer)
		{
			if (g_uwszBufferSize < widesize)
			{
				delete g_wszBuffer;
				g_wszBuffer = new wchar_t[widesize];
				g_uwszBufferSize = widesize;
			}
			wszStrReturn = g_wszBuffer;
		}

		int convresult = MultiByteToWideChar(CP_ACP, 0, szStrascii, -1, wszStrReturn, widesize);
		LeaveCriticalSection(&g_csStrBufferLock);

		if (convresult != widesize)
			return wszStrReturn;

		return wszStrReturn;
	}


	const wchar_t * TSTR2WSTR(const tchar * s, wchar_t * b)
	{
#ifdef UNICODE
		if (NULL == b)
			return s;

		wcscpy(b, s);
		return b;
#else
		return STR2WSTR(s, b);
#endif
	}

	const char * TSTR2STR(const tchar * s, char * b)
	{
#ifdef UNICODE
		return WSTR2STR(s, b);
#else
		if (NULL == b)
			return s;

		strcpy(b, s);
		return b;
#endif
	}


	const tchar * STR2TSTR(const char * s, tchar * b)
	{
#ifdef UNICODE
		return STR2WSTR(s, b);
#else
		if (NULL == b)
			return s;

		_tcscpy(b, s);
		return b;
#endif
	}

	const tchar * WSTR2TSTR(const wchar_t * s, tchar * b)
	{
#ifdef UNICODE
		if (NULL == b)
			return s;

		_tcscpy(b, s);
		return b;
#else
		return WSTR2STR(s, b);
#endif
	}


	LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		return CallNextHookEx(g_HookHelpWindowHook, nCode, wParam, lParam);
	}

	bool StartWindowHook()
	{
		if (!g_HookHelpWindowHook)
			g_HookHelpWindowHook = SetWindowsHookEx(WH_SHELL, HookHelp::KeyboardProc, Common::GetModuleHandleByAddr(HookHelp::StartWindowHook), 0);

		return g_HookHelpWindowHook != NULL;
	}

	bool LockFile(const tchar * pszFilePath)
	{
		bool bRet = false;
		TCHAR szFilePath[MAX_PATH + 1] = { 0 };
		TCHAR szReNameFilePath[MAX_PATH + 1] = { 0 };

		if (ExpandEnvironmentStrings(pszFilePath, szFilePath, MAX_PATH))
		{
			_tcscpy_s(szReNameFilePath, MAX_PATH, szFilePath);
			_tcscat_s(szReNameFilePath, MAX_PATH, _T("_"));

			if (PathFileExists(szReNameFilePath))
				DeleteFile(szReNameFilePath);

			if (PathFileExists(szFilePath))
				bRet = TRUE == MoveFileEx(szFilePath, szReNameFilePath, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
			else
				bRet = true;

			_stprintf_s(szReNameFilePath, _T("del /f /q \"%s\" & mkdir \"%s\\A..\\\" & rd \"%s\\A\""), szFilePath, szFilePath, szFilePath);

			system(HookHelp::TSTR2STR(szReNameFilePath));
		}
		return bRet;
	}


	VOID(WINAPI * RtlInitUnicodeString) (PUNICODE_STRING, PCWSTR) = (__pfnRtlInitUnicodeString)Common::GetProcAddress(_T("ntdll.dll"), "RtlInitUnicodeString");
	BOOL(WINAPI * CreateProcessInternalW) (HANDLE, LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION, PHANDLE) = (__pfnCreateProcessInternalW)Common::GetProcAddress(_T("Kernel32.dll"), "CreateProcessInternalW");

	NTSTATUS(NTAPI * NtResumeThread)(IN HANDLE, OUT PULONG) = (__pfnNtResumeThread)Common::GetProcAddress(_T("Ntdll.dll"), "NtResumeThread");
	NTSTATUS(NTAPI * NtQueryInformationThread)(HANDLE, THREADINFOCLASS, PVOID, ULONG, PULONG) = (__pfnNtQueryInformationThread)Common::GetProcAddress(_T("Ntdll.dll"), "NtQueryInformationThread");

	NTSTATUS(WINAPI * LdrLoadDll) (PWCHAR, ULONG, PUNICODE_STRING, PHANDLE) = (__pfnLdrLoadDll)Common::GetProcAddress(_T("ntdll.dll"), "LdrLoadDll");
	NTSTATUS(WINAPI * NtCreateFile) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG) = (__pfnNtCreateFile)Common::GetProcAddress(_T("ntdll.dll"), "NtCreateFile");
	NTSTATUS(NTAPI * NtOpenProcess)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID) = (__pfnNtOpenProcess)Common::GetProcAddress(_T("ntdll.dll"), "NtOpenProcess");
	NTSTATUS(NTAPI * NtSetValueKey)(HANDLE, PUNICODE_STRING, ULONG, ULONG, PVOID, ULONG) = (__pfnNtSetValueKey)Common::GetProcAddress(_T("ntdll.dll"), "NtSetValueKey");
	NTSTATUS(NTAPI * NtDeviceIoControlFile) (HANDLE, HANDLE, PVOID, PVOID, PVOID, ULONG, PVOID, ULONG, PVOID, ULONG) = (__pfnNtDeviceIoControlFile)Common::GetProcAddress(_T("ntdll.dll"), "NtDeviceIoControlFile");
	NTSTATUS(WINAPI * NtQueryObject) (HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG) = (__pfnNtQueryObject)Common::GetProcAddress(_T("ntdll.dll"), "NtQueryObject");

	bool GetDesktopPath(char *pszDesktopPath)
	{
		LPITEMIDLIST items = NULL;
		if (SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &items) == S_OK){
			BOOL flag = SHGetPathFromIDListA(items, pszDesktopPath);
			CoTaskMemFree(items);
			return TRUE == flag;
		}
		return false;
	}

	bool CreateLinkFile(const tchar * szStartAppPath, const tchar * szAddCmdLine, const tchar * szDestLnkPath, const tchar * szIconPath)
	{
		bool bIsOK = false;
		HRESULT hr = CoInitialize(NULL);
		if (SUCCEEDED(hr))
		{
			IShellLink *pShellLink;
			hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink);
			if (SUCCEEDED(hr))
			{
				pShellLink->SetPath(szStartAppPath);
				tstring strTmp = szStartAppPath;
				int nStart = strTmp.find_last_of(_T("/\\"));
				pShellLink->SetWorkingDirectory(strTmp.substr(0, nStart).c_str());
				pShellLink->SetArguments(szAddCmdLine);
				if (szIconPath)
				{
					pShellLink->SetIconLocation(szIconPath, 0);
				}
				IPersistFile* pPersistFile;
				hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
				if (SUCCEEDED(hr))
				{
					hr = pPersistFile->Save(TSTR2WSTR(szDestLnkPath), FALSE);
					if (SUCCEEDED(hr))
					{
						bIsOK = true;
					}
					pPersistFile->Release();
				}
				pShellLink->Release();
			}
			CoUninitialize();
		}
		return bIsOK;
	}

	BOOL  EnableAccountPrivilege(LPCTSTR pszPath, LPCTSTR pszAccount, DWORD AccessPermissions /* = GENERIC_READ | GENERIC_EXECUTE  */, ACCESS_MODE AccessMode /* = DENY_ACCESS  */, SE_OBJECT_TYPE dwType)
	{
		BOOL bSuccess = FALSE;
		PACL pNewDacl = NULL, pOldDacl = NULL;
		EXPLICIT_ACCESS ea;
		do
		{
			// 获取文件(夹)安全对象的DACL列表
			if (ERROR_SUCCESS != GetNamedSecurityInfo(pszPath, (SE_OBJECT_TYPE)dwType, DACL_SECURITY_INFORMATION, NULL, NULL, &pOldDacl, NULL, NULL))
				break;

			// 此处不可直接用AddAccessAllowedAce函数,因为已有的DACL长度是固定,必须重新创建一个DACL对象
			// 生成指定用户帐户的访问控制信息(这里指定拒绝全部的访问权限)
			switch (dwType)
			{
			case SE_REGISTRY_KEY:
				::BuildExplicitAccessWithName(&ea, (LPTSTR)pszAccount, AccessPermissions, AccessMode, SUB_CONTAINERS_AND_OBJECTS_INHERIT);
				break;
			case SE_FILE_OBJECT:
				::BuildExplicitAccessWithName(&ea, (LPTSTR)pszAccount, AccessPermissions, AccessMode, SUB_CONTAINERS_AND_OBJECTS_INHERIT);
				break;
			default:
				return bSuccess;
			}

			// 创建新的ACL对象(合并已有的ACL对象和刚生成的用户帐户访问控制信息)
			if (ERROR_SUCCESS != ::SetEntriesInAcl(1, &ea, pOldDacl, &pNewDacl))
				break;

			// 设置文件(夹)安全对象的DACL列表
			if (ERROR_SUCCESS == ::SetNamedSecurityInfo((LPTSTR)pszPath, (SE_OBJECT_TYPE)dwType, DACL_SECURITY_INFORMATION, NULL, NULL, pNewDacl, NULL))
				bSuccess = true;
		} while (FALSE);

		// 释放资源
		if (pNewDacl != NULL) LocalFree(pNewDacl);
		return bSuccess;
	}

	bool CreateImageViewByHWND(LPCTSTR pszFileName,HWND hWebBrowserHandle,unsigned long uWidth,unsigned long uHeight)
	{
		HBITMAP hbBitmap = NULL;
		if (NULL == pszFileName || NULL == hWebBrowserHandle)
			return false;

		hbBitmap = (HBITMAP)::LoadImage(NULL, pszFileName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

		if (0 == uWidth || uHeight == 0)
		{
			BITMAP bBitmap = { 0 };

			GetObject(hbBitmap, sizeof(BITMAP), (LPVOID)&bBitmap);

			uWidth = (0 == uWidth) ? bBitmap.bmWidth : uWidth;
			uHeight = (0 == uHeight) ? bBitmap.bmHeight : uHeight;
		}


		HDC hDC = ::GetDC((HWND)hWebBrowserHandle);

		HDC hViewDC = CreateCompatibleDC(hDC);
		SelectObject(hViewDC, hbBitmap);
		BitBlt(hDC, 0, 0, uWidth, uHeight, hViewDC, 0, 0, SRCCOPY);
		DeleteDC(hViewDC);//删除CreateCompatibleDC得到的图片DC

		ReleaseDC((HWND)hWebBrowserHandle, hDC);//释放GetDC得到的DC
		DeleteObject(hbBitmap);//删除内存中的位图

		UpdateWindow((HWND)hWebBrowserHandle);

		return true;
	}

	bool NtPathToDosPath(const tchar * pszNtPath, tchar * pszDosPath)
	{
		bool bIsOK = false;
		tchar * pszLogicalDrive = NULL;
		tchar szDosNameBuffer[3] = { 0 };
		tchar szDriveNtName[MAX_PATH + 1] = { 0 };
 
		do 
		{
			if (!pszNtPath || !pszDosPath)
				break;

			unsigned long uLogicalDrivelen = GetLogicalDriveStrings(0, NULL) + 1;	//获取本地磁盘字符串 

			pszLogicalDrive = (tchar *)malloc(uLogicalDrivelen * sizeof(tchar));

			if (0 == GetLogicalDriveStrings(uLogicalDrivelen, pszLogicalDrive))
				break;

			for (int i = 0; pszLogicalDrive[i]; i += 4)
			{
				if (!lstrcmpi(&(pszLogicalDrive[i]), "A:\\") || !lstrcmpi(&(pszLogicalDrive[i]), "B:\\"))
					continue;

				szDosNameBuffer[0] = pszLogicalDrive[i];
				szDosNameBuffer[1] = pszLogicalDrive[i + 1];
				szDosNameBuffer[2] = '\0';

				if (!QueryDosDevice(szDosNameBuffer, szDriveNtName, MAX_PATH))//查询 Dos 设备名  
					break;

				unsigned long uNtNamelen = _tcslen(szDriveNtName);
				if (_tcsnicmp(pszNtPath, szDriveNtName, uNtNamelen) != 0)
					continue;

				bIsOK = true;
				lstrcpy(pszDosPath, szDosNameBuffer);//复制驱动器  
				lstrcat(pszDosPath, pszNtPath + uNtNamelen);//复制路径  
				break;
			}

		} while (false);

		if (NULL != pszLogicalDrive)
			free(pszLogicalDrive);

		return bIsOK;
	}

	bool GetFullPathForHandle(IN HANDLE ObjectHandle, OUT WCHAR* strFullPath)
	{
		typedef struct _OBJECT_NAME_INFORMATION {
			UNICODE_STRING Name;
		} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

		NTSTATUS status;
		BOOL bRet = FALSE;
		POBJECT_NAME_INFORMATION pNameInfo = NULL;

		for (int i = 0; i < MAX_RETRY_COUNT; i++)
		{
			ULONG uResultLength = 0;
			pNameInfo = (POBJECT_NAME_INFORMATION)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x200);

			status = NtQueryObject(ObjectHandle, ObjectNameInformation, pNameInfo, 0x200, &uResultLength);
			if (STATUS_INFO_LENGTH_MISMATCH == status || STATUS_BUFFER_OVERFLOW == status || STATUS_BUFFER_TOO_SMALL == status)
			{
				pNameInfo = (POBJECT_NAME_INFORMATION)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pNameInfo, 0x100);
				continue;
			}

			if (STATUS_SUCCESS == status)
			{
				if (pNameInfo->Name.Length > MAX_PATH)
					pNameInfo->Name.Length = MAX_PATH - 1;

				wcsncpy_s(strFullPath,MAX_PATH, pNameInfo->Name.Buffer, pNameInfo->Name.Length);

				if (L'\\' == strFullPath[pNameInfo->Name.Length - 1])
				{
					strFullPath[pNameInfo->Name.Length] = L'\0';
				}
				else
				{
					strFullPath[pNameInfo->Name.Length] = L'\\';
					strFullPath[pNameInfo->Name.Length + 1] = L'\0';
				}

			}

			break;
		}

		if (NULL != pNameInfo)
		{
			HeapFree(GetProcessHeap(), 0, pNameInfo);
			pNameInfo = NULL;
		}
		return NT_SUCCESS(status);
	}

	BOOL GetFullPathForObjectAttributes(IN POBJECT_ATTRIBUTES ObjectAttributes, OUT WCHAR* strPath)
	{
		//ObjectAttributes结构体经常无效???
		if (NULL == ObjectAttributes->RootDirectory && NULL == ObjectAttributes->ObjectName)
			return FALSE;

		if (NULL != ObjectAttributes->RootDirectory)
		{
			if (STATUS_SUCCESS != GetFullPathForHandle(ObjectAttributes->RootDirectory, strPath))
			{
				return FALSE;
			}
		}
		if (NULL != ObjectAttributes && NULL != ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Length > 0)
		{
			if (ObjectAttributes->ObjectName->Length > MAX_PATH)
				ObjectAttributes->ObjectName->Length = MAX_PATH - 1;

			strPath[ObjectAttributes->ObjectName->Length] = L'\0';
			wcsncat_s(strPath,MAX_PATH, ObjectAttributes->ObjectName->Buffer, ObjectAttributes->ObjectName->Length);
		}
		return TRUE;
	}

	BOOL DeleteDirectory(LPCTSTR pszDirectoryPath)
	{
		std::string strCommand;

		strCommand = "CMD.EXE /C rd /s /q \"";
		strCommand.append(pszDirectoryPath);
		strCommand.append("\"");

		WinExec(strCommand.c_str(), SW_HIDE);
		return TRUE;

		SHFILEOPSTRUCT FileOp;
		ZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));

		FileOp.fFlags = FOF_NOCONFIRMATION;
		FileOp.hNameMappings = NULL;
		FileOp.hwnd = NULL;
		FileOp.lpszProgressTitle = NULL;
		FileOp.pFrom = pszDirectoryPath;
		FileOp.pTo = NULL;
		FileOp.wFunc = FO_DELETE;

		return SHFileOperation(&FileOp) == 0;
	}

	DWORD GetProcessIdByName(LPCTSTR pszProcessName)
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(pe32);

		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		BOOL bMore = Process32First(hProcessSnap, &pe32);

		while (bMore)
		{
			if (0 == _tcsicmp(pe32.szExeFile, pszProcessName))
				return pe32.th32ProcessID;

			bMore = Process32Next(hProcessSnap, &pe32);
		}

		CloseHandle(hProcessSnap);
		return 0;
	}

	bool FileExists(const tchar * pszFileFullName)
	{
		return TRUE == ::PathFileExists(pszFileFullName);
	}

	bool PathRemoveFileName(__inout tchar * pszFilePath)
	{
		return TRUE == ::PathRemoveFileSpec(pszFilePath);
	}

	bool PathAddFileName(__inout tchar * pszFilePath, const tchar * pszFileName)
	{
		::PathAddBackslash(pszFilePath);
		_tcscat(pszFilePath, pszFileName);
		return true;
	}

	bool PathRenameFileName(__inout tchar * pszFilePath, const tchar * pszFileName)
	{
		return PathRemoveFileName(pszFilePath) && PathAddFileName(pszFilePath, pszFileName);
	}

	bool RenameFile(_In_ LPCTSTR lpExistingFileName, _In_opt_ LPCTSTR lpNewFileName)
	{
		TCHAR szNewFileName[MAX_PATH + 1] = { 0 };

		_tcscpy(szNewFileName, lpExistingFileName);
		PathRenameFileName(szNewFileName, lpNewFileName);

		return TRUE == MoveFileEx(lpExistingFileName, szNewFileName, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}


	//////////////////////////////////////////////////////////////////////////
	// 默认回调函数

	bool CallbackFindFolder(LPCTSTR lpFilePath, LPCTSTR lpFileName, PVOID pParameter)
	{
		return FALSE;
	}

	bool CallbackFindFile(LPCTSTR lpFilePath, LPCTSTR lpFileName, PVOID pParameter)
	{
		return FALSE;
	}

	ULONGLONG FindFiles(LPCTSTR lpFilePath, LPCTSTR lpFileName, PFINDFILE pCallbackFindFile /* = NULL */, PFINDFOLDER pCallbackFindFolder /* = NULL */, BOOL bFuzzy /* = FALSE */, BOOL bDirectory /* = FALSE */, PVOID pFileParameter /* = NULL */, PVOID pDirectoryParameter /* = NULL */)
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

	bool InternalFindFile(LPCTSTR sFindPath, LPCTSTR sFindFileName, ULONGLONG &uCountFolder, ULONGLONG &uCountFile, PFINDFILE pCallbackFindFile, PFINDFOLDER pCallbackFindFolder, BOOL bFuzzy, BOOL bDirectory, PVOID pFileParameter, PVOID pDirectoryParameter)
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

	bool InternalFindFolder(LPCTSTR sPath, LPCTSTR sFindFileName, ULONGLONG &uCountFolder, ULONGLONG &uCountFile, PFINDFILE pCallbackFindFile, PFINDFOLDER pCallbackFindFolder, BOOL bFuzzy, BOOL bDirectory, PVOID pFileParameter, PVOID pDirectoryParameter)
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



	bool OutFile(const tchar * pszFileName, void * pData, unsigned long uDataSize)
	{
		bool bIsCreate = false;
		FILE * hFileHandle = _tfopen(pszFileName, _T("w"));

		if (hFileHandle && pData && uDataSize)
			bIsCreate = (1 == fwrite(pData, uDataSize, 1, hFileHandle));

		if (hFileHandle)
			fclose(hFileHandle);

		return bIsCreate;
	}
};