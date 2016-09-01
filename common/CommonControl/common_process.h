#pragma once
#include <windows.h>
#include "common_winternl.h"

namespace Common {
	inline PPEB GetCurrentProcessPEB()
	{
		return PPEB(__readfsdword(0x30));
	}

	DWORD GetParentProcessId(DWORD dwProcessId);
	bool GetProcessPath(__in DWORD dwProcessID, __out LPTSTR pszProcessPath, size_t sizeMaxLen);

	DWORD GetProcessIDFromProcessNameEx(LPCTSTR name);

	size_t GetProcessIDsFromProcessName(LPCTSTR name, OUT DWORD ** ppdwProcessIDs);
	size_t GetProcessIDsFromProcessNameEx(LPCTSTR name, OUT DWORD ** ppdwProcessIDs);

	bool SuspendProcess(HANDLE hProcess);
	inline bool SuspendProcess(DWORD dwProcessID) {
		HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwProcessID);
		if (NULL == hProcess) {
			return false;
		}

		return SuspendProcess(hProcess);
	}

	bool ResumeProcess(HANDLE hProcess);
	inline bool ResumeProcess(DWORD dwProcessID) {
		HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwProcessID);
		if (NULL == hProcess) {
			return false;
		}

		return ResumeProcess(hProcess);
	}

	bool KillProcess(HANDLE hProcess, ULONG uExitStatus = 0xFFFFFFFF);
	inline bool KillProcess(DWORD dwProcessID) {
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);
		if (NULL == hProcess) {
			return false;
		}

		return KillProcess(hProcess);
	}

	typedef bool(*CALLBACK_ENUM_PROCESS_TREE) (DWORD dwAncestorProcessID, DWORD dwParentProcessID, DWORD dwProcessID);

	bool EnumProcessTree(DWORD dwProcessID, CALLBACK_ENUM_PROCESS_TREE pfnCallback);
	bool EnumProcessTreeEx(DWORD dwProcessID, CALLBACK_ENUM_PROCESS_TREE pfnCallback);
}
