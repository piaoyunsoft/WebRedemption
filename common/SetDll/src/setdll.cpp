//////////////////////////////////////////////////////////////////////////////
//
//  Detours Test Program (setdll.cpp of setdll.exe)
//
//  Microsoft Research Detours Package, Version 3.0.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include "detours.h"

#include <Aclapi.h>
#include <tchar.h>
#include "..\\SetDll_Inferface.h"


//////////////////////////////////////////////////////////////////////////////
//
static CHAR     s_szDllPath[MAX_PATH] = "";

//////////////////////////////////////////////////////////////////////////////
//
static BOOL CALLBACK ListBywayCallback(PVOID pContext,PCHAR pszFile,PCHAR *ppszOutFile)
{
    (void)pContext;

    *ppszOutFile = pszFile;
    if (pszFile) {
        //printf("    %s\n", pszFile);
    }
    return TRUE;
}

static BOOL CALLBACK ListFileCallback(PVOID pContext,PCHAR pszOrigFile,PCHAR pszFile,PCHAR *ppszOutFile)
{
    (void)pContext;

    *ppszOutFile = pszFile;
    //printf("    %s -> %s\n", pszOrigFile, pszFile);
    return TRUE;
}

static BOOL CALLBACK AddBywayCallback(PVOID pContext, PCHAR pszFile,PCHAR *ppszOutFile)
{
    PBOOL pbAddedDll = (PBOOL)pContext;
    if (!pszFile && !*pbAddedDll) {                     // Add new byway.
        *pbAddedDll = TRUE;
        *ppszOutFile = s_szDllPath;
    }
    return TRUE;
}

BOOL SetFile(__in LPCSTR pszOrgPath, const char * pszSectionName, bool IsRemove)
{
    BOOL bIsSucc = FALSE;
	HANDLE hTempFile = INVALID_HANDLE_VALUE;
	HANDLE hChangeFile = INVALID_HANDLE_VALUE;
    PDETOUR_BINARY pBinary = NULL;

	CHAR szTempFilePath[MAX_PATH];
	CHAR szBackupFilePath[MAX_PATH];

	szTempFilePath[0] = '\0';
	szBackupFilePath[0] = '\0';

	sprintf_s(szTempFilePath, "%s~", pszOrgPath);
	::CopyFile(pszOrgPath, szTempFilePath, TRUE);

	sprintf_s(szTempFilePath, "%s.%x.bin", pszOrgPath, GetTickCount());
	sprintf_s(szBackupFilePath, "%s.%x.bak", pszOrgPath, GetTickCount());

	do 
	{
		hChangeFile = CreateFile(pszOrgPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hChangeFile) {
			break;
		}

		hTempFile = CreateFile(szTempFilePath, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (INVALID_HANDLE_VALUE == hTempFile) {
			break;
		}

		pBinary = DetourBinaryOpen(hChangeFile, pszSectionName);
		if (NULL == pBinary) {
			break;
		}

		CloseHandle(hChangeFile);
		hChangeFile = INVALID_HANDLE_VALUE;

		DetourBinaryResetImports(pBinary);

		BOOL bAddedDll = FALSE;
		if (false == IsRemove && FALSE == DetourBinaryEditImports(pBinary, &bAddedDll, AddBywayCallback, NULL, NULL, NULL)) {
			printf("DetourBinaryEditImports failed: %d\n", GetLastError());
		}

		//if (FALSE == DetourBinaryEditImports(pBinary, NULL, ListBywayCallback, ListFileCallback, NULL, NULL)) {
		//	printf("DetourBinaryEditImports failed: %d\n", GetLastError());
		//}

		if (FALSE == DetourBinaryWrite(pBinary, hTempFile, pszSectionName)) {
			printf("DetourBinaryWrite failed: %d\n", GetLastError());
			break;
		}

		if (FALSE == DetourBinaryClose(pBinary)) {
			break;
		}

		pBinary = NULL;
		CloseHandle(hTempFile);
		hTempFile = INVALID_HANDLE_VALUE;

		DWORD dwError = ERROR_SUCCESS;

		if (FALSE == DeleteFile(szBackupFilePath) && (dwError = GetLastError()) != ERROR_FILE_NOT_FOUND) {
				printf("Warning: Couldn't delete %s: %d\n", szBackupFilePath, dwError);
				break;
		}

		if (FALSE == MoveFile(pszOrgPath, szBackupFilePath)) {
			printf("Error: Couldn't back up %s to %s: %d\n", pszOrgPath, szBackupFilePath, GetLastError());
			break;
		}
		if (FALSE == MoveFile(szTempFilePath, pszOrgPath)) {
			printf("Error: Couldn't install %s as %s: %d\n", szTempFilePath, pszOrgPath, GetLastError());
			break;
		}

		bIsSucc = true;
		DeleteFile(szTempFilePath);
	} while (false);

	if (pBinary) {
		DetourBinaryClose(pBinary);
		pBinary = NULL;
	}

	if (hTempFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hTempFile);
		hTempFile = INVALID_HANDLE_VALUE;
	}

	if (hChangeFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hChangeFile);
		hChangeFile = INVALID_HANDLE_VALUE;
	}

	return bIsSucc;
}


static void * GetProcAddress2(LPCTSTR pszDllName, LPCSTR pszProcName)
{
	HINSTANCE hModule = GetModuleHandle(pszDllName);

	if (NULL == hModule)
		hModule = LoadLibrary(pszDllName);

	return ::GetProcAddress(hModule, pszProcName);
}

void c2w(wchar_t *pwstr, size_t len, const char *str)
{
	if (str)
	{
		size_t nu = strlen(str);
		size_t n = (size_t)MultiByteToWideChar(CP_ACP, 0, (const char *)str, (int)nu, NULL, 0);
		if (n >= len)
			n = len - 1;

		MultiByteToWideChar(CP_ACP, 0, (const char *)str, (int)nu, pwstr, (int)n);
		pwstr[n] = 0;
	}
}


BOOL WINAPI DisableWFP(const char * pszFileName)
{
	BOOL bRetval = FALSE;
	OSVERSIONINFO osviVersionInfo;
	wchar_t wszFileName[MAX_PATH + 1] = { 0 };
	c2w(wszFileName, MAX_PATH, pszFileName);

	ZeroMemory(&osviVersionInfo, sizeof(OSVERSIONINFO));
	osviVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osviVersionInfo);
	if (osviVersionInfo.dwMajorVersion == 5 && osviVersionInfo.dwMinorVersion == 1)
	{
		typedef DWORD(__stdcall *CPP) (DWORD param1, PWCHAR param2, DWORD param3);

		HINSTANCE hMod = LoadLibrary(_T("sfc_os.dll"));
		if (!hMod)return FALSE;

		CPP SetSfcFileException = (CPP)GetProcAddress(hMod, (LPCSTR)5);

		bRetval = SetSfcFileException(0, wszFileName, -1) == 0 ? TRUE : FALSE;

	}
	else if (osviVersionInfo.dwMajorVersion > 5)
	{
		PACL pACL = NULL;			//权限描述令牌
		PSID pSIDAdmin = NULL;
		PSID pSIDEveryone = NULL;

		do
		{
			DWORD dwRes;
			const int NUM_ACES = 2;
			EXPLICIT_ACCESSW ea[NUM_ACES];
			SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
			SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

			BOOL(WINAPI * AllocateAndInitializeSid) (PSID_IDENTIFIER_AUTHORITY, BYTE, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, PSID *);
			*(void **)&AllocateAndInitializeSid = GetProcAddress2(_T("Advapi32.dll"), "AllocateAndInitializeSid");

			if (NULL == AllocateAndInitializeSid)
				AllocateAndInitializeSid = ::AllocateAndInitializeSid;

			DWORD(WINAPI * SetNamedSecurityInfo)(LPWSTR, SE_OBJECT_TYPE, SECURITY_INFORMATION, PSID, PSID, PACL, PACL);
			*(void **)&SetNamedSecurityInfo = GetProcAddress2(_T("Advapi32.dll"), "SetNamedSecurityInfoW");

			if (NULL == SetNamedSecurityInfo)
				SetNamedSecurityInfo = ::SetNamedSecurityInfoW;

			if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSIDEveryone)) // 创建Everyone用户组的SID.
			{
				break;
			}

			if (!AllocateAndInitializeSid(&SIDAuthNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pSIDAdmin)) //创建Administrators用户组的SID.
			{
				break;
			}

			ZeroMemory(&ea, NUM_ACES * sizeof(EXPLICIT_ACCESSW));
			// 设置所有权限给 Everyone.
			ea[0].grfAccessPermissions = GENERIC_ALL;
			ea[0].grfAccessMode = SET_ACCESS;
			ea[0].grfInheritance = NO_INHERITANCE;
			ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
			ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
			ea[0].Trustee.ptstrName = (LPWSTR)pSIDEveryone;
			// 设置所有权限给 Administrators.
			ea[1].grfAccessPermissions = GENERIC_ALL;
			ea[1].grfAccessMode = SET_ACCESS;
			ea[1].grfInheritance = NO_INHERITANCE;
			ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
			ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
			ea[1].Trustee.ptstrName = (LPWSTR)pSIDAdmin;
			if (ERROR_SUCCESS != SetEntriesInAclW(NUM_ACES, ea, NULL, &pACL))	//生成新的令牌
			{
				break;
			}

			dwRes = SetNamedSecurityInfoW(wszFileName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL);	//再次尝试改变当前对象的 DACL.
			if (ERROR_SUCCESS == dwRes)
			{
				bRetval = TRUE;
				break;
			}
			if (dwRes != ERROR_ACCESS_DENIED)
			{
				break;
			}


			dwRes = SetNamedSecurityInfoW(wszFileName, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, pSIDAdmin, NULL, NULL, NULL);	// 更改这个对象的所有者为Administrators.
			if (dwRes != ERROR_SUCCESS)
			{
				break;
			}

			dwRes = SetNamedSecurityInfoW(wszFileName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL);	// 修改对象的权限.
			if (dwRes != ERROR_SUCCESS)
			{
				break;
			}
			else
			{
				bRetval = TRUE;
			}

		} while (FALSE);

		if (pSIDAdmin)
			FreeSid(pSIDAdmin);
		if (pSIDEveryone)
			FreeSid(pSIDEveryone);
		if (pACL)
			LocalFree(pACL);
	}

	return bRetval;
}

BOOL EnablePrivilege(LPCTSTR lpszPrivilegeName, BOOL bEnable)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES |
		TOKEN_QUERY | TOKEN_READ, &hToken))
		return FALSE;
	if (!LookupPrivilegeValue(NULL, lpszPrivilegeName, &luid))
		return TRUE;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = (bEnable) ? SE_PRIVILEGE_ENABLED : 0;

	AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, NULL, NULL);

	CloseHandle(hToken);

	return (GetLastError() == ERROR_SUCCESS);

}

//////////////////////////////////////////////////////////////////////// main.
//
BOOL WINAPI AddDllToFile(LPCSTR pszHostFileFullPath, LPCSTR pszAddDllName, const char * pszSectionName/* = ".ssoor"*/,bool IsRemove/* = false*/)
{
    PCHAR pszFilePart = NULL;

		if ((strchr(pszAddDllName, ':') != NULL || strchr(pszAddDllName, '\\') != NULL))
		{
			if (!GetFullPathName(pszAddDllName, sizeof(s_szDllPath), s_szDllPath, &pszFilePart))
				return FALSE;
		}
		else 
		{
#ifdef _CRT_INSECURE_DEPRECATE
			sprintf_s(s_szDllPath, sizeof(s_szDllPath), "%s", pszAddDllName);
#else
			sprintf(s_szDllPath, "%s", pszAddDllName);
#endif
		}

		if (pszHostFileFullPath[0] == 0 && s_szDllPath[0] == 0)
		{
			return false;
		}

		if (FALSE == EnablePrivilege(SE_DEBUG_NAME, TRUE)) {
			return false;
		}

		if (FALSE == EnablePrivilege(SE_TAKE_OWNERSHIP_NAME, TRUE)) {
			return false;
		}

		DisableWFP(pszHostFileFullPath);

		return SetFile(pszHostFileFullPath, pszSectionName, IsRemove);
}

// End of File
