#pragma once
#include "..\CommonControl\Commondef.h"

#define SIZE_RECOVERY					10

bool RecoveryHook(void * pProcAddr);
bool RecoveryHook(const tchar * pszDllName, const char * pszFunctionName);

bool RecoveryIATHook(const tchar * pszDllName, const char * pszFunctionName);

bool RecoveryInlineHook(void * pProcAddr);
bool RecoveryInlineHook(const tchar * pszDllName, void * pProcAddr);