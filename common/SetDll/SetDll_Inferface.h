#pragma once 

BOOL WINAPI DisableWFP(const char * pszFileName);

BOOL UpdateProcessWithDll(HANDLE hProcess, LPCSTR pszDllName);

BOOL WINAPI AddDllToFile(LPCSTR pszHostFileFullPath, LPCSTR pszAddDllName, const char * pszSectionName = ".ssoor", bool IsRemove = false);