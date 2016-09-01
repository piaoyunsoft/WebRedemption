#pragma once
#include <windows.h>
#include <tchar.h>

#include "common_winternl.h"

namespace Common {
	void CreateWindowHideThread(_In_ HWND hWnd, _In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight);
	void CreateWindowRedrowThread(_In_ HWND hWnd, LPCSTR pszFileName, _In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight);

	bool CreateImageViewByHWND(LPCSTR pszFileName, HWND hWebBrowserHandle, _In_ int X, _In_ int Y, _In_ int nWidth = 0, _In_ int nHeight = 0);

	static bool IsClassName(HWND hWnd,const TCHAR * pszClassName) {
		TCHAR szBuf[MAX_PATH + 1] = { 0 };

		if (0 != ::GetClassName(hWnd, szBuf, MAX_PATH) && 0 ==  _tcsicmp(szBuf, pszClassName)) {
			return true;
		}

		return false;
	}
}

