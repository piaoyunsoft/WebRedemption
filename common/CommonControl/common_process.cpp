#include "common_process.h"

#include <stdio.h>
#include <tchar.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <Shlwapi.h>

#include "ntstatus.h"
#include "common_winternl.h"

namespace Common {
	typedef NTSTATUS(NTAPI *__pfnNtSuspendProcess)(IN HANDLE ProcessHandle);
	typedef NTSTATUS(NTAPI *__pfnNtResumeProcess)(IN HANDLE ProcessHandle);
	typedef NTSTATUS(NTAPI *__pfnNtTerminateProcess)(_In_opt_ HANDLE ProcessHandle, _In_ NTSTATUS ExitStatus);

	typedef LONG(WINAPI * __pfnNtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

	DWORD GetParentProcessId(DWORD dwProcessId)
	{
		LONG                      status = 0;
		DWORD                     dwParentPID = 0;
		PROCESS_BASIC_INFORMATION pbi = { 0 };

		__pfnNtQueryInformationProcess pfnNtQueryInformationProcess = (__pfnNtQueryInformationProcess)GetProcAddress(GetModuleHandle(_T("ntdll")), "NtQueryInformationProcess");

		if (!pfnNtQueryInformationProcess)
			return 0;

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);

		if (NULL == hProcess) {
			return 0;
		}

		if (NT_SUCCESS(pfnNtQueryInformationProcess(hProcess, ProcessBasicInformation, (PVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL))) {
			dwParentPID = (DWORD)pbi.Reserved3;//InheritedFromUniqueProcessId
		}

		CloseHandle(hProcess);

		return dwParentPID;
	}

	size_t GetProcessIDsFromProcessName(LPCTSTR name, OUT DWORD ** ppdwProcessIDs)
	{
		size_t sizeIndex = 0;
		DWORD dwProcessIds[255] = { 0 };

		HANDLE hProcessSnap = INVALID_HANDLE_VALUE;

		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };

		if (!Process32First(hProcessSnap, &pe32))
			return 0;

		do
		{
			if (_tcsicmp(pe32.szExeFile, name) == 0)
			{
				dwProcessIds[sizeIndex++] = pe32.th32ProcessID;
				if (sizeIndex >= 255) {
					break;
				}
			}

			pe32.dwSize = sizeof(PROCESSENTRY32);
		} while (Process32Next(hProcessSnap, &pe32));

		CloseHandle(hProcessSnap);

		if (0 != sizeIndex) {
			*ppdwProcessIDs = (DWORD *)malloc(sizeof(DWORD) * sizeIndex);

			memcpy(*ppdwProcessIDs, dwProcessIds, sizeof(DWORD) * sizeIndex);
		}

		return sizeIndex;
	}

	DWORD GetProcessIDFromProcessNameEx(LPCTSTR name) {

		DWORD dwOutProcessID = 0;
		DWORD * pdwProcessIDs = NULL;
		size_t sizeProcessCount = Common::GetProcessIDsFromProcessNameEx(name, &pdwProcessIDs);

		if (0 != sizeProcessCount) {
			dwOutProcessID = pdwProcessIDs[0];
		}

		free(pdwProcessIDs);

		return dwOutProcessID;
	}

	size_t GetProcessIDsFromProcessNameEx(LPCTSTR name, OUT DWORD ** ppdwProcessIDs)
	{
		size_t sizeIndex = 0;
		DWORD dwProcessIds[255] = { 0 };

		LPCTSTR pszEnumProcessName = NULL;
		TCHAR szProcessPath[MAX_PATH + 1] = { 0 };
		for (DWORD dwEnumProcessID = 0; dwEnumProcessID <= 65536; dwEnumProcessID += 4) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwEnumProcessID);

			if (NULL == hProcess) {
				continue;
			}

			DWORD dwViewProcessID = ::GetProcessId(hProcess); // 重新获取一下进程ID ，有时候能打开的进程ID并不是真正展示的进程ID 

			CloseHandle(hProcess);

			szProcessPath[0] = _T('\0');
			if (Common::GetProcessPath(dwViewProcessID, szProcessPath, MAX_PATH)) {
				pszEnumProcessName = _tcsrchr(szProcessPath, _T('\\'));

				if (NULL == pszEnumProcessName) {
					continue;
				}

				pszEnumProcessName++;
			}

			if (pszEnumProcessName && 0 == _tcsicmp(pszEnumProcessName, name)) {
				dwProcessIds[sizeIndex++] = dwViewProcessID;
				if (sizeIndex >= 255) {
					break;
				}
			}
		}

		if (0 != sizeIndex) {
			*ppdwProcessIDs = (DWORD *)malloc(sizeof(DWORD) * sizeIndex);

			memcpy(*ppdwProcessIDs, dwProcessIds, sizeof(DWORD) * sizeIndex);
		}


		return sizeIndex;
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

	bool NtSuspendProcess(HANDLE hProcess) {
		NTSTATUS ntStatus = STATUS_SUCCESS;
		__pfnNtSuspendProcess pfnNtSuspendProcess = (__pfnNtSuspendProcess)GetProcAddress(GetModuleHandle(_T("ntdll")), "NtSuspendProcess");

		if (NULL == pfnNtSuspendProcess) {
			return false;
		}

		if (NT_SUCCESS(ntStatus = pfnNtSuspendProcess(hProcess))) {
			return true;
		}

		if (STATUS_ACCESS_DENIED == ntStatus) {
			hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, ::GetProcessId(hProcess));
			if (NULL == hProcess) {
				return false;
			}

			bool bIsSuspend = NT_SUCCESS(ntStatus = pfnNtSuspendProcess(hProcess));

			CloseHandle(hProcess);
			return bIsSuspend;
		}

		return false;
	}

	bool SuspendProcess(HANDLE hProcess) {
		DWORD dwProcessID = ::GetProcessId(hProcess);

		if (NtSuspendProcess(hProcess)) {
			return true;
		}

		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessID); // Get the list of threads in the system.

		if (hSnapshot == INVALID_HANDLE_VALUE) {
			return false;
		}

		HANDLE hThread = NULL;
		THREADENTRY32 teThreadEntry = { sizeof(teThreadEntry) }; // Walk the list of threads.
		for (BOOL bIsOK = Thread32First(hSnapshot, &teThreadEntry); bIsOK; bIsOK = Thread32Next(hSnapshot, &teThreadEntry)) {

			if (teThreadEntry.th32OwnerProcessID == dwProcessID) { // Is this thread in the desired process?
				hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, teThreadEntry.th32ThreadID); // Attempt to convert the thread ID into a handle.

				if (hThread != NULL) {
					SuspendThread(hThread); // Suspend the thread.

					CloseHandle(hThread);
				}
			}
		}

		CloseHandle(hSnapshot);
		return true;
	}

	bool NtResumeProcess(HANDLE hProcess) {
		NTSTATUS ntStatus = STATUS_SUCCESS;
		__pfnNtResumeProcess pfnNtResumeProcess = (__pfnNtResumeProcess)GetProcAddress(GetModuleHandle(_T("ntdll")), "NtResumeProcess");

		if (NULL == pfnNtResumeProcess) {
			return false;
		}

		if (NT_SUCCESS(ntStatus = pfnNtResumeProcess(hProcess))) {
			return true;
		}

		if (STATUS_ACCESS_DENIED == ntStatus) {
			hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, ::GetProcessId(hProcess));
			if (NULL == hProcess) {
				return false;
			}

			bool bIsSuspend = NT_SUCCESS(ntStatus = pfnNtResumeProcess(hProcess));

			CloseHandle(hProcess);
			return bIsSuspend;
		}

		return false;
	}

	bool ResumeProcess(HANDLE hProcess) {
		DWORD dwProcessID = ::GetProcessId(hProcess);

		if (NtResumeProcess(hProcess)) {
			return true;
		}

		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessID); // Get the list of threads in the system.

		if (hSnapshot == INVALID_HANDLE_VALUE) {
			return false;
		}

		HANDLE hThread = NULL;
		THREADENTRY32 teThreadEntry = { sizeof(teThreadEntry) }; // Walk the list of threads.
		for (BOOL bIsOK = Thread32First(hSnapshot, &teThreadEntry); bIsOK; bIsOK = Thread32Next(hSnapshot, &teThreadEntry)) {

			if (teThreadEntry.th32OwnerProcessID == dwProcessID) { // Is this thread in the desired process?
				hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, teThreadEntry.th32ThreadID); // Attempt to convert the thread ID into a handle.

				if (hThread != NULL) {
					ResumeThread(hThread); // resume the thread.

					CloseHandle(hThread);
				}
			}
		}

		CloseHandle(hSnapshot);
		return true;
	}

	bool NtTerminateProcess(_In_opt_ HANDLE hProcessHandle, _In_ NTSTATUS ExitStatus) {
		NTSTATUS ntStatus = STATUS_SUCCESS;
		__pfnNtTerminateProcess pfnNtTerminateProcess = (__pfnNtTerminateProcess)GetProcAddress(GetModuleHandle(_T("ntdll")), "NtTerminateProcess");

		if (NULL == pfnNtTerminateProcess) {
			return false;
		}

		if (NT_SUCCESS(ntStatus = pfnNtTerminateProcess(hProcessHandle, ExitStatus))) {
			return true;
		}

		if (STATUS_ACCESS_DENIED == ntStatus) {
			hProcessHandle = OpenProcess(PROCESS_TERMINATE, FALSE, ::GetProcessId(hProcessHandle));
			if (NULL == hProcessHandle) {
				return false;
			}

			bool bIsTerminal = NT_SUCCESS(ntStatus = pfnNtTerminateProcess(hProcessHandle, ExitStatus));

			CloseHandle(hProcessHandle);
			return bIsTerminal;
		}

		return false;
	}

	bool KillProcess(HANDLE hProcess,ULONG uExitStatus/* = 0xFFFFFFFF*/) {
		if (NtTerminateProcess(hProcess, uExitStatus)) {
			return true;
		}

		if (FALSE == ::TerminateProcess(hProcess, uExitStatus) && ERROR_ACCESS_DENIED == ::GetLastError()) {
			hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, ::GetProcessId(hProcess));
			if (NULL == hProcess) {
				return false;
			}

			bool bIsTerminal =  TRUE == ::TerminateProcess(hProcess, uExitStatus);

			CloseHandle(hProcess);
			return bIsTerminal;
		}

		return false;
	}

	bool Internal_EnumProcessTrue(DWORD dwAncestorProcessID, DWORD dwProcessID, CALLBACK_ENUM_PROCESS_TREE pfnCallback) {
		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, dwProcessID);

		if (hProcessSnap == INVALID_HANDLE_VALUE) {
			return false;
		}

		PROCESSENTRY32 info;
		info.dwSize = sizeof(PROCESSENTRY32);

		BOOL bMore = Process32First(hProcessSnap, &info);

		while (bMore) {
			if (dwProcessID == info.th32ParentProcessID) {
				if (false == pfnCallback(dwAncestorProcessID, info.th32ParentProcessID, info.th32ProcessID)) {
					return false;
				}

				if (false == Internal_EnumProcessTrue(dwAncestorProcessID, info.th32ProcessID, pfnCallback)) {
					return false;
				}
			}

			bMore = Process32Next(hProcessSnap, &info);
		}

		return true;
	}

	bool EnumProcessTree(DWORD dwProcessID, CALLBACK_ENUM_PROCESS_TREE pfnCallback) {
		if (false == pfnCallback(dwProcessID, GetParentProcessId(dwProcessID), dwProcessID)) {
			return false;
		}

		return Internal_EnumProcessTrue(dwProcessID, dwProcessID, pfnCallback);
	}

	bool Internal_EnumProcessTrueEx(DWORD dwAncestorProcessID, DWORD dwProcessID, CALLBACK_ENUM_PROCESS_TREE pfnCallback) {
		for (DWORD dwEnumProcessID = 0; dwEnumProcessID <= 65536; dwEnumProcessID += 4) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwEnumProcessID);

			if (NULL == hProcess) {
				continue;
			}

			DWORD dwViewProcessID = ::GetProcessId(hProcess); // 重新获取一下进程ID ，有时候能打开的进程ID并不是真正展示的进程ID 
			DWORD dwParentProcessID = GetParentProcessId(dwViewProcessID);

			CloseHandle(hProcess);

			if (dwParentProcessID == dwProcessID) {
				if (false == pfnCallback(dwAncestorProcessID, dwParentProcessID, dwViewProcessID)) {
					return false;
				}

				if (false == Internal_EnumProcessTrue(dwAncestorProcessID, dwViewProcessID, pfnCallback)) {
					return false;
				}
			}
		}

		return true;
	}

	bool EnumProcessTreeEx(DWORD dwProcessID, CALLBACK_ENUM_PROCESS_TREE pfnCallback) {
		if (false == pfnCallback(dwProcessID, GetParentProcessId(dwProcessID), dwProcessID)) {
			return false;
		}

		return Internal_EnumProcessTrueEx(dwProcessID, dwProcessID, pfnCallback);
	}
}