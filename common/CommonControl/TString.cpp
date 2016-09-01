#include "TString.h"
#include <tchar.h>
#include <ostream>
#include <iosfwd>
#include <windows.h>

CTString::CTString()
{
	m_pString(1);
	m_pString[0] = '\0';
}

CTString::CTString(const tchar *pStr)
{
	_tcscpy(m_pString(_tcslen(pStr) + 1), pStr);
}

CTString::CTString(CTString &CStringSrc){
	_tcscpy(m_pString(CStringSrc.GetLength() + 1), CStringSrc.m_pString);
}

CTString::CTString(const tchar ch, const unsigned int uiRepeat)
{
	int i = 0;

	for (m_pString(uiRepeat + 1); i < uiRepeat; i++)
		m_pString[i] = ch;

	m_pString[i] = _T('\0');
}

CTString::CTString(const tchar *pStr, unsigned int uiLength, unsigned int uiBegin)
{
	unsigned int uiLen = _tcslen(pStr);

	if (uiBegin > uiLen)
		uiBegin = uiLen;
	if (uiLength > uiLen - uiBegin)
		uiLength = uiLen - uiBegin;

	_tcsncpy(m_pString(uiLength + 1), pStr + uiBegin, uiLength);
	m_pString[uiLength] = _T('\0');
}

CTString::CTString(CTString &CStringSrc,unsigned int uiLength,unsigned int uiBegin)
{ 
	unsigned int uiLen = CStringSrc.GetLength();

	if (uiBegin > uiLen)
		uiBegin = uiLen;
	if (uiLength > uiLen - uiBegin)
		uiLength = uiLen - uiBegin;

	_tcsncpy(m_pString(uiLength + 1), (tchar *)CStringSrc.m_pString + uiBegin, uiLength);
	m_pString[uiLength] = _T('\0');
}

CTString::~CTString()
{
}

unsigned int CTString::GetLength(void){
	return _tcslen(m_pString);
}

tchar *CTString::GetBuffer(void){
	return m_pString;
}

bool CTString::IsEmpty(void){
	return GetLength() ? false : true;
}

void CTString::Empty(void){
	m_pString = _T('\0');
}

tchar CTString::GetAt(unsigned int uiIndex){
	if (uiIndex >= GetLength()){
		uiIndex = GetLength() - 1;
	}

	return *((tchar *)m_pString + uiIndex);
}

void CTString::SetAt(unsigned int uiIndex, tchar ch){
	if (uiIndex >= GetLength()){
		uiIndex = GetLength() - 1;
	}

	m_pString[uiIndex] = ch;
}

int CTString::Compare(const tchar *str){
	return _tcscmp(m_pString, str);
}

int CTString::Compare(CTString &Str){
	return _tcscmp(m_pString, Str.m_pString);
}

int CTString::CompareNoCase(const tchar *str){
	return _tcsicmp(m_pString, str);
}

int CTString::CompareNoCase(CTString &Str){
	return _tcsicmp(m_pString, Str.m_pString);
}

CTString CTString::Right(unsigned long uiLength)
{
	if (uiLength >= GetLength())
		return Mid(0, uiLength);

	return Mid(this->GetLength() - uiLength, uiLength);
}

CTString CTString::Left(unsigned long uiLength)
{
	return Mid(0, uiLength);
}

CTString CTString::Mid(unsigned long uiBegin){
	return Mid(uiBegin, GetLength());
}

tchar * CTString::Mid(unsigned long uiBegin, unsigned long uiLength)
{
	int iLen = GetLength();

	if (uiBegin > iLen)
		uiBegin = iLen;
	if (uiLength > (iLen - uiBegin))
		uiLength = iLen - uiBegin;

	_tcsncpy(m_pBuffer(uiLength + 1), (tchar *)m_pString + uiBegin, uiLength);
	m_pBuffer[uiLength] = _T('\0');

	return (tchar *)m_pBuffer;
}

CTString &CTString::Upper(void){
	m_pString = _tcsupr(m_pString);
	return *this;
}

CTString &CTString::Lower(void){
	m_pString = _tcslwr(m_pString);
	return *this;
}

CTString &CTString::Reverse(void){
	m_pString = _tcsrev(m_pString);
	return *this;
}

CTString &CTString::Replace(tchar chOld, tchar chNew){
	for (int i = 0; i < GetLength(); i++){
		if (GetAt(i) == chOld){
			SetAt(i, chNew);
		}
	}
	return *this;
}

CTString &CTString::Replace(tchar *pOld, tchar *pNew){
	unsigned int uiTmp;
	while (true){
		uiTmp = Find(pOld);
		if (uiTmp == -1){
			break;
		}
		Delete(uiTmp,_tcslen(pOld));
		Insert(uiTmp, pNew);
	}
	return *this;
}

CTString &CTString::Remove(tchar ch){
	bool flag;
	do{
		flag = false;
		for (int i = 0; i < GetLength(); i++){
			if (GetAt(i) == ch){
				Delete(i);
				flag = true;
			}
		}
	} while (flag);

	return *this;
}

CTString &CTString::Delete(unsigned int uiIndex, unsigned int uiCount){
	unsigned int uiLen = GetLength();

	if (uiIndex >= uiLen){
		return *this;
	}
	if (uiCount == 0){
		return *this;
	}

	if (uiCount > uiLen - uiIndex){
		uiCount = uiLen - uiIndex;
	}

	_tcsncpy(m_pBuffer(uiLen - uiCount + 1), m_pString, uiIndex);
	m_pBuffer[uiIndex] = _T('\0');
	_tcscat(m_pBuffer, (tchar *)m_pString + uiIndex + uiCount);

	m_pString = m_pBuffer;
	return *this;
}

CTString &CTString::Insert(unsigned int uiIndex,const tchar *str){
	unsigned int thisLen = GetLength();
	unsigned int uiLen = _tcslen(str);

	if (uiIndex > thisLen){
		uiIndex = thisLen;
	}

	_tcsncpy(m_pBuffer(thisLen + uiLen + 1), m_pString, uiIndex);
	m_pBuffer[uiIndex] = _T('\0');
	_tcscat(m_pBuffer, str);
	_tcscat(m_pBuffer, (tchar *)m_pString + uiIndex);

	m_pString = m_pBuffer;
	return *this;
}

CTString &CTString::Insert(unsigned int uiIndex, const tchar ch){
	unsigned int thisLen = GetLength();

	if (uiIndex > thisLen){
		uiIndex = thisLen;
	}

	_tcsncpy(m_pBuffer(thisLen + 1 + 1), m_pString, uiIndex);
	m_pBuffer[uiIndex] = ch;
	m_pBuffer[uiIndex + 1] = _T('\0');
	_tcscat(m_pBuffer, (tchar *)m_pString + uiIndex);

	m_pString = m_pBuffer;
	return *this;
}

//在对象字符串中,从索引uiBegin开始,返回ch第一次出现的位置,省略uiBegin使其为默认的0,未找到返回-1 
int CTString::Find(const tchar ch, unsigned int uiBegin){
	const tchar * pszFindStart = _tcschr((tchar *)m_pString + uiBegin, ch);

	if (pszFindStart == NULL){
		return -1;
	}

	return pszFindStart - m_pString;
}

//在对象字符串中,从索引uiBegin开始,返回字符串str第一次出现的位置,省略uiBegin使其为默认的0,未找到返回-1 
int CTString::Find(const tchar *str, unsigned int uiBegin){
	tchar * pszFindStart = _tcsstr((tchar *)m_pString + uiBegin, str);

	if (pszFindStart == NULL){
		return -1;
	}

	return pszFindStart - m_pString;
}

//反向查找字符ch,并返回在其在对象字符串中的索引位置,未找到返回-1 
int CTString::ReverseFind(const tchar ch){
	CTString tmp(*this);
	tmp.Reverse();
	if (Find(ch) == -1){
		return -1;
	}
	return GetLength() - 1 - tmp.Find(ch);
}

//查找str所指向的字符串包含的字符,返回第一次出现的索引值,未找到返回-1 
int CTString::FindOneOf(const tchar *str){
	const tchar * pszFindStart = _tcsstr(m_pString, str);
	if (NULL == pszFindStart)
		return -1;

	return pszFindStart - m_pString; 
}

//去除对象字符串左侧的字符ch 
CTString &CTString::TrimLeft(tchar ch){
	Trim(1, ch);
	return *this;
}

//去除对象字符串左侧的换行,空格,制表字符 
CTString &CTString::TrimLeft(void){
	Trim(1, _T('\n'));
	Trim(1, _T(' '));
	Trim(1, _T('\t'));
	return *this;
}

//去除对象字符串左侧的位于str所指向的字符串中的字符 
CTString &CTString::TrimLeft(tchar *str){
	for (int i = 0; i < strlen(str); i++){
		Trim(1, str[i]);
	}
	return *this;
}

//去除对象字符串右侧的换行,空格,制表字符 
CTString &CTString::TrimRight(void){
	Trim(2, _T('\n'));
	Trim(2, _T(' '));
	Trim(2, _T('\t'));
	return *this;
}

////去除对象字符串右侧的字符ch 
CTString &CTString::TrimRight(char ch){
	Trim(2, ch);
	return *this;
}

//去除对象字符串右侧的位于str所指向的字符串中的字符 
CTString &CTString::TrimRight(char *str){
	for (int i = 0; i < _tcslen(str); i++){
		Trim(2, str[i]);
	}
	return *this;
}

//1代表LEFT,2代表RIGHT 
CTString &CTString::Trim(int mode, tchar ch){
	unsigned int uiBegin = 0;
	unsigned int uiEnd = GetLength() - 1;

	if (mode == 1){
		while (m_pString[uiBegin] == ch && uiBegin <= uiEnd)
			uiBegin++;
	}
	else if (mode == 2){
		while (m_pString[uiEnd] == ch && uiEnd >= uiBegin)
			uiEnd--;
	}
	else{
		return *this;
	}

	unsigned int uiLen = uiEnd - uiBegin + 1;
	
	_tcsncpy(m_pBuffer(uiLen + 1), (tchar *)m_pString + uiBegin, uiLen);
	m_pBuffer[uiLen] = _T('\0');

	m_pString = m_pBuffer;
	return *this;
}
/*定义成员函数 End*/

/*定义重载的运算符 Begin*/
CTString& CTString::operator=(const tchar ch){

	m_pString(2);
	m_pString[0] = ch;

	return *this;
}

CTString& CTString::operator=(const tchar *str){
	strcpy(m_pString(strlen(str)), str);

	return *this;
}

CTString& CTString::operator=(CTString &Str){
	strcpy(m_pString(Str.GetLength()), Str.m_pString);

	return *this;
}

CTString operator+(CTString &Str, const tchar *str){
	CTString tmp = Str;
	tmp += str;
	return tmp;
}

CTString operator+(const tchar *str, CTString& Str){
	CTString tmp = str;
	tmp += Str;
	return tmp;
}

CTString operator+(CTString &Str1, CTString &Str2){
	CTString tmp = Str1;
	tmp += Str2;
	return tmp;
}

CTString operator+(CTString &Str, tchar ch){
	CTString tmp = Str;
	tmp += ch;
	return tmp;
}

CTString operator+(tchar ch, CTString &Str){
	CTString tmp = ch;
	tmp += Str;
	return tmp;
}

CTString& CTString::operator+=(const tchar ch){
	int thisLen = this->GetLength();

	m_pString(thisLen + 2);
	m_pString[thisLen] = ch;
	m_pString[thisLen + 1] = '\0';

	return *this;
}

CTString& CTString::operator+=(const tchar *str){
	if (NULL == str)
		return *this;

	_tcscat(m_pString(_tcslen(m_pString) + _tcslen(str) + 1), str);
	return *this;
}

CTString& CTString::operator+=(CTString &Str){
	_tcscat(m_pString(_tcslen(m_pString) + Str.GetLength() + 1), Str.m_pString);
	return *this;
}

const tchar &CTString::operator[](unsigned int n){
	if (n >= GetLength())
		n = GetLength() - 1;
	return *((char *)m_pString + n);
}

bool operator==(CTString &Str, const tchar *str){
	return _tcscmp(Str.m_pString, str) == 0;
}

bool operator==(const tchar *str, CTString &Str){
	return _tcscmp(str, Str.m_pString) == 0;
}

bool operator==(CTString &Str1, CTString &Str2){
	return _tcscmp(Str1.m_pString, Str2.m_pString) == 0;
}

bool operator!=(const CTString &Str, const tchar *str){
	return _tcscmp(Str.m_pString, str) != 0;
}

bool operator!=(const tchar *str, const CTString &Str){
	return _tcscmp(str, Str.m_pString) != 0;
}

bool operator!=(const CTString &Str1, const CTString &Str2){
	return _tcscmp(Str1.m_pString, Str2.m_pString) != 0;
}

bool operator<(const CTString &Str, const tchar *str){
	return _tcscmp(Str.m_pString, str) < 0;
}

bool operator<(const tchar *str, const CTString &Str){
	return _tcscmp(str, Str.m_pString) < 0;
}

bool operator<(const CTString &Str1, const CTString &Str2){
	return _tcscmp(Str1.m_pString, Str2.m_pString)<0;
}

bool operator>(const CTString &Str, const tchar *str){
	return _tcscmp(Str.m_pString, str)>0;
}

bool operator>(const tchar *str, const CTString &Str){
	return _tcscmp(str, Str.m_pString) > 0;
}

bool operator>(const CTString &Str1, const CTString &Str2) {
	return _tcscmp(Str1.m_pString, Str2.m_pString) > 0;
}

bool operator<=(const CTString &Str, const tchar *str) {
	return _tcscmp(Str.m_pString, str) <= 0;
}

bool operator<=(const tchar *str, const CTString &Str){
	return _tcscmp(str, Str.m_pString) <= 0;
}

bool operator<=(const CTString &Str1, const CTString &Str2){
	return _tcscmp(Str1.m_pString, Str2.m_pString) <= 0;
}

bool operator>=(const CTString &Str, const tchar *str){
	return _tcscmp(Str.m_pString, str) >= 0;
}

bool operator>=(const tchar *str, const CTString &Str){
	return _tcscmp(str, Str.m_pString) >= 0;
}

bool operator>=(const CTString &Str1, const CTString &Str2){
	return _tcscmp(Str1.m_pString, Str2.m_pString) >= 0;
}


char * g_szBuffer = NULL;
wchar_t * g_wszBuffer = NULL;
int g_uszBufferSize = 0;
int g_uwszBufferSize = 0;
CRITICAL_SECTION g_csStrBufferLock = { 0 };

bool InitializeHookHelp()
{
	InitializeCriticalSection(&g_csStrBufferLock);
	return true;
}
bool bLock = InitializeHookHelp();

void MbsToWcs(LPCSTR lpSource, LPWSTR lpDest)
{
	MultiByteToWideChar(CP_ACP, 0, lpSource, strlen(lpSource) + 1, lpDest, MAX_PATH / sizeof(lpDest[0]));
};

void WcsToMbs(LPCWSTR lpSource, LPSTR lpDest)
{
	WideCharToMultiByte(CP_ACP, 0, lpSource, -1, lpDest, MAX_PATH, NULL, NULL);
};

///////////////////////////////////////////////////////////////////////
//unicode  ascii

const char * WSTR2STR(const wchar_t * wszStrcode, char * szStrBuffer)
{
	char * szStrReturn = szStrBuffer;
	int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wszStrcode, -1, NULL, 0, NULL, NULL);

	if (asciisize == ERROR_NO_UNICODE_TRANSLATION || asciisize == 0)
		return NULL;

	EnterCriticalSection(&g_csStrBufferLock);
	if (NULL == szStrBuffer)
	{
		if (g_uszBufferSize < asciisize)
		{
			delete g_szBuffer;
			g_szBuffer = new char[asciisize];
			g_uszBufferSize = asciisize;
		}
		szStrReturn = g_szBuffer;
	}
	int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wszStrcode, -1, szStrReturn, asciisize, NULL, NULL);
	LeaveCriticalSection(&g_csStrBufferLock);

	if (convresult != asciisize)
		return szStrReturn;

	return szStrReturn;
}

///////////////////////////////////////////////////////////////////////
//ascii  Unicode

const wchar_t * STR2WSTR(const char * szStrascii, wchar_t * wszStrBuffer)
{
	wchar_t * wszStrReturn = wszStrBuffer;
	int widesize = MultiByteToWideChar(CP_ACP, 0, szStrascii, -1, NULL, 0);

	if (widesize == ERROR_NO_UNICODE_TRANSLATION || widesize == 0)
		return NULL;

	EnterCriticalSection(&g_csStrBufferLock);
	if (NULL == wszStrBuffer)
	{
		if (g_uwszBufferSize < widesize)
		{
			delete g_wszBuffer;
			g_wszBuffer = new wchar_t[widesize];
			g_uwszBufferSize = widesize;
		}
		wszStrReturn = g_wszBuffer;
	}

	int convresult = MultiByteToWideChar(CP_ACP, 0, szStrascii, -1, wszStrReturn, widesize);
	LeaveCriticalSection(&g_csStrBufferLock);

	if (convresult != widesize)
		return wszStrReturn;

	return wszStrReturn;
}


const wchar_t * TSTR2WSTR(const tchar * s, wchar_t * b)
{
#ifdef UNICODE
	if (NULL == b)
		return s;

	wcscpy(b, s);
	return b;
#else
	return STR2WSTR(s, b);
#endif
}

const char * TSTR2STR(const tchar * s, char * b)
{
#ifdef UNICODE
	return WSTR2STR(s, b);
#else
	if (NULL == b)
		return s;

	strcpy(b, s);
	return b;
#endif
}


const tchar * STR2TSTR(const char * s, tchar * b)
{
#ifdef UNICODE
	return STR2WSTR(s, b);
#else
	if (NULL == b)
		return s;

	_tcscpy(b, s);
	return b;
#endif
}

const tchar * WSTR2TSTR(const wchar_t * s, tchar * b)
{
#ifdef UNICODE
	if (NULL == b)
		return s;

	_tcscpy(b, s);
	return b;
#else
	return WSTR2STR(s, b);
#endif
}
