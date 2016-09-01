#ifndef _COMMON_WINTERNL_
#define _COMMON_WINTERNL_
#include <windows.h>
#include <winternl.h>

#ifndef NT_SUCCESS
typedef __success(return >= 0) LONG NTSTATUS;

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#ifndef NT_ERROR
#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)
#endif

#endif