#pragma once
#include <tchar.h>

#include "Commondef.h"

class CTBuffer
{
public:
	CTBuffer(unsigned long lBufferLeng = 1)
	{
		m_lBufferLength = 0;

		Alloc(lBufferLeng);
	}
	~CTBuffer()
	{
		if (m_pBuffer)
			delete[] m_pBuffer;
	};

public:
	tchar * Alloc(unsigned long lLength)
	{
		if (lLength == 0)
			lLength = 1;

		if (m_lBufferLength < lLength)
		{
			tchar * pszTemp = NULL;
			if (m_pBuffer)
				pszTemp = m_pBuffer;

			m_lBufferLength = lLength;
			m_pBuffer = new tchar[lLength + 1];

			if (pszTemp)
			{
				_tcscpy(m_pBuffer, pszTemp);
				delete[] pszTemp;
			}

			m_pBuffer[lLength] = _T('\0');
		}

		return m_pBuffer;
	}

	tchar * operator ()(unsigned long lLength)
	{
		return this->Alloc(lLength);
	}

	operator tchar *() const
	{
		return this->m_pBuffer;
	}

	operator tchar() const
	{
		return this->m_pBuffer[0];
	}

	CTBuffer& operator+=(const tchar ch)
	{
		unsigned long lLength = _tcslen(m_pBuffer);
		this->Alloc(lLength + 2);

		m_pBuffer[lLength] = ch;
		m_pBuffer[lLength + 1] = _T('\0');
		return *this;
	}
	CTBuffer& operator+=(const tchar *str)
	{
		unsigned long lLength = _tcslen(m_pBuffer);
		this->Alloc(lLength + _tcslen(str));
		_tcscat(m_pBuffer, str);
		return *this;
	}

	const tchar * operator =(const tchar * pszContext)
	{
		if (NULL == pszContext)
			pszContext = _T("\0");

		if (pszContext == m_pBuffer)
			return m_pBuffer;

		return _tcscpy(this->Alloc(_tcslen(pszContext)), pszContext);
	}

	const tchar * operator =(const tchar ch)
	{
		this->Alloc(2);
		m_pBuffer[0] = ch;
		m_pBuffer[1] = _T('\0');

		return m_pBuffer;
	}

	tchar * operator=(CTBuffer & cTBuffer)
	{
		return _tcscpy(this->Alloc(_tcslen(cTBuffer)), cTBuffer);
	}

	friend bool operator==(CTBuffer &Str, int str)
	{
		if (0 != str)
			return (char)str == Str[0];

		return (0 == _tcslen(Str)) ? true : false;
	}

	friend bool operator==(int str ,CTBuffer &Str)
	{
		if (0 != str)
			return (char)str == Str[0];

		return (0 == _tcslen(Str)) ? true : false;
	}

	friend bool operator==(CTBuffer &Str, const tchar *str)
	{
		return (0 == _tcscmp(Str, str)) ? true : false;
	}

	friend bool operator==(const tchar *str, CTBuffer &Str)
	{
		return (0 == _tcscmp(Str, str)) ? true : false;
	}

	friend bool operator==(CTBuffer &Str, tchar *str)
	{
		return (0 == _tcscmp(Str, str)) ? true : false;
	}

	friend bool operator==(tchar *str, CTBuffer &Str)
	{
		return (0 == _tcscmp(Str, str)) ? true : false;
	}

protected:
	unsigned long m_lBufferLength;
	tchar * m_pBuffer;
};

class CTString
{
public:
	CTString(); 
	CTString(const tchar *pStr);
	CTString(CTString &CStringSrc);
	CTString(const tchar ch, const unsigned int uiRepeat = 1);
	CTString(const tchar *pStr, unsigned int uiLen, unsigned int uiBegin = 0);
	CTString(CTString &CStringSrc, unsigned int uiLen, unsigned int uiBegin = 0);
	~CTString();
	unsigned int GetLength(void);
	tchar *GetBuffer(void);
	bool IsEmpty(void);
	void Empty(void);
	tchar GetAt(unsigned int uiIndex);
	void SetAt(unsigned int uiIndex, tchar ch);
	//用于比较的函数 
	int Compare(const tchar *str);
	int Compare(CTString &Str);
	int CompareNoCase(const tchar *str);
	int CompareNoCase(CTString &Str);
	//字符串截取函数 
	CTString Right(unsigned long uiLength);
	CTString Left(unsigned long uiLength);
	CTString Mid(unsigned long uiBegin);
	tchar * Mid(unsigned long uiBegin, unsigned long uiLength);
	//大小转换函数 
	CTString &Upper(void);
	CTString &Lower(void);
	CTString &Reverse(void);
	//字符串修饰(包括置换,删除,添加等) 
	CTString &Replace(tchar chOld, tchar chNew);
	CTString &Replace(tchar *pOld, tchar *pNew);
	CTString &Insert(unsigned int uiIndex, const tchar ch);
	CTString &Insert(unsigned int uiIndex,const tchar *str);
	CTString &Remove(tchar ch);
	CTString &Delete(unsigned int uiIndex, unsigned int uiCount = 1);
	CTString &TrimLeft(void);
	CTString &TrimLeft(tchar ch);
	CTString &TrimLeft(tchar *str);
	CTString &TrimRight(void);
	CTString &TrimRight(tchar ch);
	CTString &TrimRight(tchar *str);
	//查找函数 
	int Find(const tchar ch, unsigned int uiBegin = 0);
	int Find(const tchar *str, unsigned int uiBegin = 0);
	int ReverseFind(const tchar ch);
	int FindOneOf(const tchar *str);
	/*声明成员函数 End*/


	operator tchar *() const
	{
		return this->m_pString;
	}

	operator tchar() const
	{
		return this->m_pString[0];
	}

	/*声明重载的操作符 Begin*/
	//作为成员函数 
	CTString& operator= (const tchar ch);
	CTString& 	 operator= (const tchar *str);
	CTString& 	 operator= (CTString &Str);
	CTString& 	 	 operator+=(const tchar ch);
	CTString& 	 operator+=(const tchar *str);
	CTString& 	 operator+=(CTString &Str);
	const tchar& operator[](unsigned int n);
	//作为友元 
	friend CTString operator+ (CTString &Str1, CTString &Str2);
	friend CTString operator+ (CTString &Str, const tchar *str);
	friend CTString operator+ (const tchar *str, CTString &Str);
	friend CTString operator+ (CTString &Str, tchar ch);
	friend CTString operator+ (tchar ch, CTString &Str);
	friend bool operator==(CTString &Str1, CTString &Str2);
	friend bool operator==(CTString &Str, const tchar *str);
	friend bool operator==(const tchar *str, CTString &Str);
	friend bool operator!=(const CTString &Str1, const CTString &Str2);
	friend bool operator!=(const CTString &Str, const tchar *str);
	friend bool operator!=(const tchar *str, const CTString &Str);
	friend bool operator< (const CTString &Str1, const CTString &Str2);
	friend bool operator< (const CTString &Str, const tchar *str);
	friend bool operator< (const tchar *str, const CTString &Str);
	friend bool operator> (const CTString &Str1, const CTString &Str2);
	friend bool operator> (const CTString &Str, const tchar *str);
	friend bool operator> (const tchar *str, const CTString &Str);
	friend bool operator<=(const CTString &Str1, const CTString &Str2);
	friend bool operator<=(const CTString &Str, const tchar *str);
	friend bool operator<=(const tchar *str, const CTString &Str);
	friend bool operator>=(const CTString &Str1, const CTString &Str2);
	friend bool operator>=(const CTString &Str, const tchar *str);
	friend bool operator>=(const tchar *str, const CTString &Str);
	/*声明重载的操作符 End*/
private:
	CTBuffer m_pBuffer;
	CTBuffer m_pString; //指向存储空间的首地址 
	CTString &Trim(int mode, tchar ch);
};


inline void MbsToWcs(const char * lpSource, const wchar_t * lpDest);
inline void WcsToMbs(const wchar_t * lpSource, const tchar * lpDest);

const char * TSTR2STR(const tchar * s, char * b = NULL);
const tchar * STR2TSTR(const char * s, tchar * b = NULL);
const tchar * WSTR2TSTR(const wchar_t * s, tchar * b = NULL);
const wchar_t * TSTR2WSTR(const tchar * s, wchar_t * b = NULL);
inline const char * WSTR2STR(const wchar_t * wszStrcode, char * szStrBuffer = NULL);
inline const wchar_t * STR2WSTR(const char * szStrascii, wchar_t * wszStrBuffer = NULL);
