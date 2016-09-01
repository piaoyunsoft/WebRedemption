#pragma once
#include <stdio.h>
#include "Commondef.h"

//
//NOTICE
//
//Success	成功提示
//Outputs	输出提示
//Warning	警告提示
//Errors	错误提示

#define LOGErrorsW __FILE__,__LINE__,__FUNCTION__," ERROR "
#define LOGErrorsA __FILE__,__LINE__,__FUNCTION__," ERROR "

#define LOGSuccessW __FILE__,__LINE__,__FUNCTION__,"SUCCESS"
#define LOGSuccessA __FILE__,__LINE__,__FUNCTION__,"SUCCESS"

#define LOGWarningW __FILE__,__LINE__,__FUNCTION__,"WARNING"
#define LOGWarningA __FILE__,__LINE__,__FUNCTION__,"WARNING"

#define LOGOutputsW __FILE__,__LINE__,__FUNCTION__,"OUTPUTS"
#define LOGOutputsA __FILE__,__LINE__,__FUNCTION__,"OUTPUTS"

#ifdef UNICODE
#define LOGErrors LOGErrorsW
#define LOGSuccess LOGSuccessW
#define LOGWarning LOGWarningW
#define LOGOutputs LOGOutputsW
#else
#define LOGErrors __FILE__,__LINE__,__FUNCTION__,_T(" ERROR ")
#define LOGSuccess __FILE__,__LINE__,__FUNCTION__,_T("SUCCESS")
#define LOGWarning __FILE__,__LINE__,__FUNCTION__,_T("WARNING")
#define LOGOutputs __FILE__,__LINE__,__FUNCTION__,_T("OUTPUTS")
#endif

#define BITS_OF_LOGS_STEP 10

class CDebug
{
public:
	CDebug(const tchar * lpLogFileName);
	~CDebug(void);

	static void SetPath(const tchar * lpLogFilePath);

public:
#ifdef UNICODE
#define Print	PrintW
#else
#define Print	PrintA
#endif

	void PrintA(const char * lpSourceFile, unsigned long dwSourceLine, const char * pszFunctionName, const char * pszOutLevel, _In_z_ _Printf_format_string_ const char * lpOutputString, ...);
	void PrintW(const char * lpSourceFile, unsigned long dwSourceLine, const char * pszFunctionName, const char * pszOutLevel, _In_z_ _Printf_format_string_ const wchar_t * lpOutputString, ...);

	LPCTSTR ShowError(DWORD dwErrorId = GetLastError());  

	void SetName(const tchar * lpPrintString);

private:
	FILE * GetPrintHandle(bool IsUnicode = false);
	void PrintHeader(const tchar * lpSourceFile, unsigned long dwSourceLine, const tchar * pszFunctionName, const tchar * pszOutLevel);

	void Lock()
	{
		EnterCriticalSection(&m_csLogLock);
	}

	void Unlock()
	{
		LeaveCriticalSection(&m_csLogLock);
	}

private:
	SYSTEMTIME m_sTime;
	HLOCAL m_pErrorMsg;

	tchar * m_pszBuffer;
	unsigned long m_uBufferLen;

	FILE * m_hPrintFileHandle;
	tchar m_szLogName[MAX_PATH + 1];
	static tchar m_szLogPath[MAX_PATH + 1];

	CRITICAL_SECTION m_csLogLock;
};
