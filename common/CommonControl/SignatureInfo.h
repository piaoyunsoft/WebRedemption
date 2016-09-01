#pragma once
//***************************************************************************************
//
//       Filename:  Verify.h
//
//    Description:  CVerify 数字签名接口封装类
//
//        Version:  1.0
//        Created:  2011-09-19
//       Revision:  none
//
//         Author:  陈彦旭 (yanxu.chen@gmail.com)
//****************************************************************************************

#include <string>
#if _MSC_VER >1020
using namespace std;
#endif

#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <stdio.h>
#include <tchar.h>

#pragma comment(lib, "crypt32.lib")

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

typedef struct {
	LPWSTR lpszProgramName;
	LPWSTR lpszPublisherLink;
	LPWSTR lpszMoreInfoLink;

} SPROG_PUBLISHERINFO, *PSPROG_PUBLISHERINFO;

class CSignatureInfo
{
public:
	CSignatureInfo();
	~CSignatureInfo();

public:
	wstring m_strProgramName;
	wstring m_strPublisherLink;
	wstring m_strMoreInfoLink;
	wstring m_strSignCompany;
	wstring m_strTimestamp;
	wstring m_strSubjectName;
};

class CVerify
{
public:
	CVerify(void);
	~CVerify(void);

public:
	static BOOL GetSignedStore(const wchar_t* szFileName, CSignatureInfo& info);
	static BOOL GetProgAndPublisherInfo(PCMSG_SIGNER_INFO pSignerInfo, PSPROG_PUBLISHERINFO Info);
	static BOOL GetDateOfTimeStamp(PCMSG_SIGNER_INFO pSignerInfo, SYSTEMTIME *st);
	static BOOL GetCertificateInfo(PCCERT_CONTEXT pCertContext, CSignatureInfo& info);
	static BOOL GetTimeStampSignerInfo(PCMSG_SIGNER_INFO pSignerInfo, PCMSG_SIGNER_INFO *pCounterSignerInfo);
	static LPWSTR AllocateAndCopyWideString(LPCWSTR inputString);
	static BOOL  GetProductInfo(LPCWSTR modulename, LPWSTR szCompanay, LPWSTR szSoftName, LPWSTR szVersion,
		LPWSTR szFileVersion, LPWSTR szFileDes, WORD &langID, WORD &charsetID);
};
