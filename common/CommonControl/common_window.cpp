#include "common_window.h"

namespace {

	struct _redrow_view {
		HWND hWnd;
		RECT rect;
		const char * pszImagePath;
	};

	DWORD WINAPI Thread_HideView(_redrow_view* pInfo) {
		_redrow_view tiInfo = *pInfo;

		::ShowWindow(tiInfo.hWnd, SW_HIDE);

		while (IsWindow(tiInfo.hWnd)) {
			::ShowWindow(tiInfo.hWnd, SW_HIDE);
			Sleep(25);
		}

		return 0;
	}

	DWORD WINAPI Thread_RedrowView(_redrow_view* pInfo) {
		_redrow_view tiInfo = *pInfo;

		Common::CreateImageViewByHWND(tiInfo.pszImagePath, tiInfo.hWnd, tiInfo.rect.left, tiInfo.rect.top, tiInfo.rect.right, tiInfo.rect.bottom);

		while (IsWindow(tiInfo.hWnd)) {
			Common::CreateImageViewByHWND(tiInfo.pszImagePath, tiInfo.hWnd, tiInfo.rect.left, tiInfo.rect.top, tiInfo.rect.right, tiInfo.rect.bottom);
			Sleep(25);
		}

		return 0;
	}
}

namespace Common {

	void CreateWindowHideThread(_In_ HWND hWnd, _In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight) {
		static _redrow_view tiInfo = { 0 };

		tiInfo.hWnd = hWnd;
		tiInfo.rect.left = X;
		tiInfo.rect.top = Y;
		tiInfo.rect.right = nWidth;
		tiInfo.rect.bottom = nHeight;

		DWORD dwThreadID = 0;
		CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_HideView, (void *)&tiInfo, 0, &dwThreadID));
	}

	void CreateWindowRedrowThread(_In_ HWND hWnd, LPCSTR pszFileName, _In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight) {
		static _redrow_view tiInfo = { 0 };

		tiInfo.hWnd = hWnd;
		tiInfo.rect.left = X;
		tiInfo.rect.top = Y;
		tiInfo.rect.right = nWidth;
		tiInfo.rect.bottom = nHeight;

		tiInfo.pszImagePath = pszFileName;

		DWORD dwThreadID = 0;
		CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_RedrowView, (void *)&tiInfo, 0, &dwThreadID));
	}

	bool CreateImageViewByHWND(LPCSTR pszFileName, HWND hWebBrowserHandle, _In_ int X, _In_ int Y, _In_ int nWidth /* = 0 */, _In_ int nHeight /* = 0 */)	{
		HBITMAP hbBitmap = NULL;
		if (NULL == pszFileName || NULL == hWebBrowserHandle)
			return false;

		char szSaveFilePath[MAX_PATH + 1] = { 0 };

		strcpy_s(szSaveFilePath, pszFileName);
		ExpandEnvironmentStringsA(pszFileName, szSaveFilePath, MAX_PATH);

		hbBitmap = (HBITMAP)::LoadImage(NULL, szSaveFilePath, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

		if (0 == nWidth || nHeight == 0)
		{
			BITMAP bBitmap = { 0 };

			GetObject(hbBitmap, sizeof(BITMAP), (LPVOID)&bBitmap);

			nWidth = (0 == nWidth) ? bBitmap.bmWidth : nWidth;
			nHeight = (0 == nHeight) ? bBitmap.bmHeight : nHeight;
		}


		HDC hDC = ::GetDC((HWND)hWebBrowserHandle);

		HDC hViewDC = CreateCompatibleDC(hDC);
		SelectObject(hViewDC, hbBitmap);
		BitBlt(hDC, X, Y, nWidth, nHeight, hViewDC, 0, 0, SRCCOPY);
		DeleteDC(hViewDC);//删除CreateCompatibleDC得到的图片DC

		ReleaseDC((HWND)hWebBrowserHandle, hDC);//释放GetDC得到的DC
		DeleteObject(hbBitmap);//删除内存中的位图

		UpdateWindow((HWND)hWebBrowserHandle);

		return true;
	}

}