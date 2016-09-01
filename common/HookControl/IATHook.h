#pragma once

#ifdef _M_AMD64
#ifndef _WIN64
#define _WIN64
#endif
#endif

namespace HookControl {

	typedef bool(*__pfnIATHookNotifyRoutine)(IN PIMAGE_THUNK_DATA pCurrentFuncAddrThunk, IN LPCSTR pszCurrentFuncName, LPCVOID pConnext);

	//
	// 修改IAT表的函数，通过所在模块的基地址hModule找到输入表的数据目录项，获取IAT的首指针，查询目标函数地址所在的位置，然后修改为钩子函数的地址。
	//
	bool IATHook(IN HMODULE hHookModule, IN LPCTSTR pszTargetFileName, IN LPCVOID pTargetFuncAddr, IN LPCVOID pReplaceFuncAddr);

	//
	// 修改IAT表的函数，通过所在模块的基地址hModule找到输入表的数据目录项，获取IAT的首指针，查询目标函数地址所在的位置，然后修改为钩子函数的地址。
	//
	bool IATHook(IN HMODULE hHookModule, IN LPCTSTR pszTargetFileName, IN LPCSTR pTargetFuncName, IN LPCVOID pReplaceFuncAddr);

	int SetIATHookNotifyRoutine(IN HMODULE hHookModule, IN LPCTSTR pszTargetDllName, __pfnIATHookNotifyRoutine pfnIATHookNotifyRoutine, IN LPCVOID pConnext = NULL);

	bool IATHooks(IN HMODULE hHookModule, IN LPCVOID pTargetFuncAddr, IN LPCVOID pReplaceFuncAddr);
}