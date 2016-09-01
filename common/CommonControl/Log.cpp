#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <shlwapi.h>


#include "Log.h"
#include "TString.h"
#include "Commonfun.h"

//#pragma comment (lib, "psapi.lib")
#pragma comment (lib, "Shlwapi.lib")



// #ifdef _DEBUG
// #define FORMAT_HEADER_PRINT	"%02d:%02d:%02d.%03d [%s] FILENAME:[%s] LINE:%d "
// #else
#define FORMAT_HEADER_PRINT			"%02d:%02d:%02d.%03d [%s] [% 5u] "
//#endif

tchar CDebug::m_szLogPath[MAX_PATH + 1] = { 0 };

CDebug::CDebug(LPCTSTR lpLogFileName) : m_pErrorMsg(NULL)
{
	tchar szFilePath[MAX_PATH + 1] = { 0 };

	m_uBufferLen = 0;
	m_pszBuffer = NULL;
	m_pErrorMsg = NULL;
	m_hPrintFileHandle = NULL;
	memset(m_szLogPath, 0, MAX_PATH);
	memset(m_szLogName, 0, MAX_PATH);

	InitializeCriticalSection(&m_csLogLock);

	if (lpLogFileName)
		_tcscpy(m_szLogName, lpLogFileName);
	else
		_tcscpy(m_szLogName, _T("DEFAULT"));

	this->SetPath(NULL);
}

CDebug::~CDebug(void)
{
	if (m_pszBuffer)
		free(m_pszBuffer);

	if (m_pErrorMsg)
		LocalFree(m_pErrorMsg);

	if (m_hPrintFileHandle)
		fclose(m_hPrintFileHandle);

	DeleteCriticalSection(&m_csLogLock);
}

void CDebug::SetName(const tchar * lpLogFileName)
{
	if (NULL == lpLogFileName)
		return;

	_tcsncpy(m_szLogName, lpLogFileName, MAX_PATH);
}

void CDebug::SetPath(LPCTSTR lpLogFilePath)
{
	if (NULL == lpLogFilePath)
		lpLogFilePath = _T("%APPDATA%\\SSOOR");

	if (0 == ExpandEnvironmentStringsA(lpLogFilePath, m_szLogPath, MAX_PATH))
		_tcsncpy(m_szLogPath, lpLogFilePath, MAX_PATH);
}

LPCTSTR CDebug::ShowError(DWORD dwErrorId)
{
	if (NULL != m_pErrorMsg)
	{
		LocalFree(m_pErrorMsg);
		m_pErrorMsg = NULL;
	}

	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, dwErrorId, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPTSTR)&m_pErrorMsg, 0, NULL))
		Print(LOGErrors, _T("%s"), m_pErrorMsg);

	return (LPCTSTR)m_pErrorMsg;
}

FILE * CDebug::GetPrintHandle(bool IsUnicode /* = false */)
{
	if (NULL == m_hPrintFileHandle)
	{
		GetLocalTime(&m_sTime);
		tchar szLogFullPath[MAX_PATH + 1] = { 0 };

		if (m_szLogPath[0] != _T('\0')) {
			if (0 == ExpandEnvironmentStrings(_T("%APPDATA%\\SSOOR"), m_szLogPath, MAX_PATH)) {
				_tcscpy(m_szLogPath, _T("%APPDATA%\\SSOOR"));
			}
		}

		CreateDirectory(m_szLogPath, NULL);
		_stprintf(szLogFullPath, _T("%s\\%s_%04d%02d%02d.log"), m_szLogPath, m_szLogName, m_sTime.wYear, m_sTime.wMonth, m_sTime.wDay);

		_tfopen_s(&m_hPrintFileHandle, szLogFullPath, _T("at+"));
	}

	return (NULL == m_hPrintFileHandle) ? stderr : m_hPrintFileHandle;
}

void CDebug::PrintHeader(const tchar * lpSourceFile, DWORD dwSourceLine, const tchar * pszFunctionName, LPCTSTR pszOutLevel)
{
	GetLocalTime(&m_sTime);

	__try
	{
		_ftprintf(this->GetPrintHandle(), FORMAT_HEADER_PRINT, m_sTime.wHour, m_sTime.wMinute, m_sTime.wSecond, m_sTime.wMilliseconds, pszOutLevel, ::GetCurrentProcessId(), lpSourceFile, dwSourceLine);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		fprintf(this->GetPrintHandle(), "00:00:00.000 [%s] ", pszOutLevel);
	}
}

void CDebug::PrintA(const char * lpSourceFile, DWORD dwSourceLine, const char * pszFunctionName, LPCSTR pszOutLevel, _In_z_ _Printf_format_string_ const char * lpOutputString, ...)
{
	this->Lock();
	this->PrintHeader(lpSourceFile, dwSourceLine, pszFunctionName, pszOutLevel);

	__try
	{
		va_list vlArgs;
		va_start(vlArgs, lpOutputString);
		int nLength = _vscprintf(lpOutputString, vlArgs);

		if (nLength > m_uBufferLen)
		{
			m_uBufferLen = ((nLength >> BITS_OF_LOGS_STEP) + 1) << BITS_OF_LOGS_STEP;

			if (m_pszBuffer)
				delete[] m_pszBuffer;

			m_pszBuffer = new TCHAR[m_uBufferLen--];
			*m_pszBuffer = NULL;
			m_pszBuffer[m_uBufferLen] = NULL;
		}

		vsprintf(m_pszBuffer, lpOutputString, vlArgs);
		va_end(vlArgs);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		strncpy(m_pszBuffer, lpOutputString, m_uBufferLen);
	}

	::OutputDebugStringA(m_pszBuffer);
	fprintf(this->GetPrintHandle(), "%s\r\n", m_pszBuffer);

	if (m_hPrintFileHandle)
	{
		fclose(m_hPrintFileHandle);
		m_hPrintFileHandle = NULL;
	}

	this->Unlock();
}


void CDebug::PrintW(const char * lpSourceFile, DWORD dwSourceLine, const char * pszFunctionName, LPCSTR pszOutLevel, _In_z_ _Printf_format_string_ const wchar_t * lpOutputString, ...)
{
	this->Lock();
	this->PrintHeader(lpSourceFile, dwSourceLine, pszFunctionName, pszOutLevel);

	__try
	{
		va_list vlArgs;
		va_start(vlArgs, lpOutputString);
		int nLength = _vscwprintf(lpOutputString, vlArgs);

		if (nLength > m_uBufferLen)
		{
			m_uBufferLen = ((nLength >> BITS_OF_LOGS_STEP) + 1) << BITS_OF_LOGS_STEP;

			if (m_pszBuffer)
				delete[] m_pszBuffer;

			m_pszBuffer = (LPSTR)new WCHAR[m_uBufferLen--];
			*m_pszBuffer = NULL;
			m_pszBuffer[m_uBufferLen] = NULL;
		}

		_vswprintf((LPWSTR)m_pszBuffer, lpOutputString, vlArgs);
		va_end(vlArgs);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		wcsncpy((LPWSTR)m_pszBuffer, lpOutputString, m_uBufferLen);
	}

	::OutputDebugStringW((LPWSTR)m_pszBuffer);
	fwprintf(this->GetPrintHandle(true), L"%s\r\n", (LPWSTR)m_pszBuffer);

	if (m_hPrintFileHandle)
	{
		fclose(m_hPrintFileHandle);
		m_hPrintFileHandle = NULL;
	}

	this->Unlock();
}
