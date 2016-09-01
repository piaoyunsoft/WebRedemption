#include <windows.h>
#include "src\detours.h"


BOOL UpdateProcessWithDll(HANDLE hProcess, LPCSTR pszDllName)
{
	return DetourUpdateProcessWithDll(hProcess, &pszDllName, 1);
}
