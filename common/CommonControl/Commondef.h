#pragma once

#ifndef interface
#define interface               __STRUCT__
#endif

#ifdef UNICODE
typedef  wchar_t tchar;
#ifdef _XSTRING_
typedef std::wstring tstring;
#endif
#else
typedef char tchar;
#ifdef _XSTRING_
typedef std::string tstring;
#endif
#endif


typedef unsigned char byte;

#define random(x) ((0 == (x)) ? 0 : (rand()%(x)))

#define MAX_LOG_LEN													0x1000

#define MAX_RETRY_COUNT											30
#define MAX_ENCRY_KEY_LEN										20

#define MAX_REMARK_LEN												512

#define MAX_URL_LEN													1024
#define MAX_HOST_LEN												31

#define MAX_IP4_LEN													16
#define MAX_IP4_STRING_LEN									16
#define MAX_MAC_LEN												25

#define MAX_MD5_LEN												66
#define MAX_CLSID_LEN												40

#define MAX_PATH														260

#ifdef _DEBUG
#ifdef _M_AMD64
#define DIRECTORY_LIB_INTERNAL "..\\..\\InternalLib_Debug_x64\\"
#else
#define DIRECTORY_LIB_INTERNAL "..\\..\\InternalLib_Debug\\"
#endif
#else
#ifdef _M_AMD64
#define DIRECTORY_LIB_INTERNAL "..\\..\\InternalLib_x64\\"
#else
#define DIRECTORY_LIB_INTERNAL "..\\..\\InternalLib\\"
#endif
#endif
