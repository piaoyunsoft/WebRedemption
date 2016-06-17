#pragma once
#include <ws2tcpip.h>
#include <winternl.h>
#include <stdio.h>

#include <CommonControl/Commonfun.h>

BOOL GetWSAExFunction(GUID&funGuid, void** ppFunction);

const char * __inet_ntop(int af, in_addr src_addr, char *dst, socklen_t cnt);
const wchar_t * __inet_ntopw(int af, in_addr src_addr, wchar_t *dst, socklen_t cnt);

inline bool FindModule(_In_opt_ LPCTSTR lpModuleName, _Out_ HMODULE * phModule)
{
	return TRUE == GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, lpModuleName, phModule);
}

inline bool LockModule(_In_opt_ LPCTSTR lpModuleName, _Out_ HMODULE * phModule)
{
	return TRUE == GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, lpModuleName, phModule);
}

inline PPEB GetCurrentProcessPEB()
{
	return PPEB(__readfsdword(0x30));
}

inline PRTL_USER_PROCESS_PARAMETERS GetCurrentProcessParameters() {
	PPEB pProcessPEB = GetCurrentProcessPEB();

	if (NULL == pProcessPEB || NULL == pProcessPEB->ProcessParameters) {
		return NULL;
	}

	if (NULL == pProcessPEB->ProcessParameters->CommandLine.Buffer || NULL == pProcessPEB->ProcessParameters->ImagePathName.Buffer) {
		return NULL;
	}

	return pProcessPEB->ProcessParameters;
}

namespace Lists {
	static const TCHAR * pszModuleNameHitLists[] = {
		_T("chrome.dll") , /* chrome/360安全/360极速/uc/2345/猎豹/qq 浏览器*/
		_T("mxwebkit.dll") ,_T("xul.dll" ),  /* 遨游云 浏览器*/
		_T("webkitcore.dll") , /* 搜狗 核心浏览器*/
		_T("mxwebkit.dll")/* 遨游 核心浏览器*/,
		_T("fastproxy.dll"), _T("chromecore.dll"),/* 百度 核心浏览器*/
	};

	inline HINSTANCE ModuleNameHitTest() {
		HINSTANCE hHookInstance = NULL;

		for (int i = 0; i < (sizeof(pszModuleNameHitLists) / sizeof(pszModuleNameHitLists[0])); i++)
		{
			if (FindModule(pszModuleNameHitLists[i], &hHookInstance)) {
				return hHookInstance;
			}
		}

		return NULL;
	}

	static const TCHAR * ptszProcessNameHitLists[] = {
		_T("iexplore.exe")/* IE 核心浏览器*/,

		_T("maxthon.exe")/* 遨游 核心浏览器*/,
		_T("liebao.exe")/* 猎豹 核心浏览器*/,
		_T("360se.exe")/* 360安全 核心浏览器*/,
		_T("360chrome.exe")/* 360极速 核心浏览器*/,
		_T("chrome.exe")/* Chrome 核心浏览器*/,
		_T("qqbrowser.exe")/* QQ 核心浏览器*/,
		_T("twchrome.exe")/* 世界之窗 核心浏览器*/,
		_T("sogouexplorer.exe")/* 搜狗 核心浏览器*/,
		_T("baidubrowser.exe")/* 百度 核心浏览器*/, //会出现无法打开的情况
		_T("2345explorer.exe")/* 2345 核心浏览器*/,

		_T("f1browser.exe")/* F1 核心浏览器*/,

		_T("yidian.exe") /* 一点浏览器 湖南岳阳 应该是当地实名制公司的*/,
	};

	inline const TCHAR * ProcessNameHitTest() {
		HINSTANCE hHookInstance = NULL;

		for (int i = 0; i < (sizeof(ptszProcessNameHitLists) / sizeof(ptszProcessNameHitLists[0])); i++)
		{
			if (FindModule(ptszProcessNameHitLists[i], &hHookInstance)) {
				return ptszProcessNameHitLists[i];
			}
		}

		return NULL;
	}

	static const wchar_t * pszProcessPathHitLists[] = {
		L"\\application\\"/* F1 核心浏览器*/,
		L"浏览器\\"/* F1 核心浏览器*/,
		L"\\浏览器"/* F1 核心浏览器*/,
	};

	inline const size_t ProcessPathHitTest(TCHAR * pszCommandLineBuffer = NULL, size_t sizeBufferSize = 0) {
		HINSTANCE hHookInstance = NULL;
		const wchar_t * pszCurrentCommandLine = ::GetCommandLineW();

		PRTL_USER_PROCESS_PARAMETERS pProcessParameters = GetCurrentProcessParameters();

		if (pProcessParameters) {
			pszCurrentCommandLine = pProcessParameters->CommandLine.Buffer;
		}

		for (int i = 0; i < (sizeof(pszProcessPathHitLists) / sizeof(pszProcessPathHitLists[0])); i++)
		{
			if (wcsstr(pszCurrentCommandLine, pszProcessPathHitLists[i])) {
				if (pszCommandLineBuffer) {
					Common::WSTR2TSTR(pszCurrentCommandLine, pszCommandLineBuffer, (int*)&sizeBufferSize);
				}

				return  wcslen(pszCurrentCommandLine);
			}
		}

		return 0;
	}

}