#include <stdio.h>
#include "HTTPPacket.h"

CHTTPHeader::CHTTPHeader(const char * pszPacketContext /* = NULL */, CAutoBuffer * pAutoBuffer /* = NULL */)
{
	Init(pszPacketContext, pAutoBuffer);
}

CHTTPHeader::CHTTPHeader(CAutoBuffer & cAutoBuffer)
{
	Init(NULL, &cAutoBuffer);
}

CHTTPHeader::~CHTTPHeader()
{
	if (m_pszHTTPHeader)
		m_cAutoBuffer->UnlockBuffer(m_pszHTTPHeader);
	if (m_pszHeaderBuffer)
		m_cAutoBuffer->UnlockBuffer(m_pszHeaderBuffer);
	if (m_pszNodeNameBuffer)
		m_cAutoBuffer->UnlockBuffer(m_pszHTTPHeader);
}

void CHTTPHeader::Init(const char * pszPacketContext /* = NULL */, CAutoBuffer * pAutoBuffer /* = NULL */)
{
	m_pszHTTPHeader = NULL;
	m_pszHeaderBuffer = NULL;
	m_pszNodeNameBuffer = NULL;

	if (pAutoBuffer)
		m_cAutoBuffer = pAutoBuffer;
	else
		m_cAutoBuffer = new CAutoBuffer();

	SetHeader(pszPacketContext);
}

const char * CHTTPHeader::GetHeader()
{
	return m_pszHTTPHeader;
}

const char * CHTTPHeader::SetHeader(const char * pszHTTPHeader, unsigned long uHeaderLength /* = 0 */)
{
	const char * pszHeaderEnd = NULL;

	if (NULL == pszHTTPHeader)
		pszHTTPHeader = DEFAULT_HEADER_HTTP;

	if (0 == uHeaderLength)
		uHeaderLength = strlen(pszHTTPHeader);

	do 
	{
		if (0 != _strnicmp(pszHTTPHeader, HEADER_QUEST_HTTP_GET, strlen(HEADER_QUEST_HTTP_GET)) &&
			0 != _strnicmp(pszHTTPHeader, HEADER_QUEST_HTTP_POST, strlen(HEADER_QUEST_HTTP_GET)) &&
			0 != _strnicmp(pszHTTPHeader, HEADER_STATUS_HTTP_HTTP, strlen(HEADER_STATUS_HTTP_HTTP))
			)
		{
			break;
		}

		SetHeaderLength(uHeaderLength + 2);

		for (int i = 0; i < uHeaderLength - 2; i++)
		{
			if ('\n' == pszHTTPHeader[i] && (('\r' == pszHTTPHeader[i + 1] && '\n' == pszHTTPHeader[i + 2]) || '\n' == pszHTTPHeader[i + 1]))
			{
				m_pszHTTPHeader[i] = pszHTTPHeader[i];
				m_pszHTTPHeader[i + 1] = pszHTTPHeader[i + 1];
				m_pszHTTPHeader[i + 2] = ('\r' == pszHTTPHeader[i + 1]) ? pszHTTPHeader[i + 2] : '\0';

				m_pszHTTPHeader[i + 3] = '\0';
				pszHeaderEnd = ('\r' == pszHTTPHeader[i + 1]) ? &pszHTTPHeader[i + 3] : &pszHTTPHeader[i + 2];
				break;
			}

			m_pszHTTPHeader[i] = pszHTTPHeader[i];
		}
	} while (false);

	return pszHeaderEnd;
}

unsigned long CHTTPHeader::GetHeaderLength()
{
	return strlen(m_pszHTTPHeader);
}

//////////////////////////////////////////////////////////////////////////

const char * CHTTPHeader::JumpWhitespace(const char * pszString)
{
	const char *  pszRetString = pszString;

	for (; pszRetString[0] == ' '; pszRetString++)
		continue;
	for (; pszRetString[0] == '\t'; pszRetString++)
		continue;

	return pszRetString;
}

void CHTTPHeader::TrimBlank(char * pszString, bool bReserve /* = true */)
{
	unsigned long uStrlen = strlen(pszString);
	for (int i = uStrlen; i > 0; i--)
	{
		if (' ' == pszString[i] && (false == bReserve || ' ' == pszString[i - 1]))
		{
			memmove(&pszString[i], &pszString[i + 1], uStrlen - i + 1);
			uStrlen--;
		}
	}

	if (false == bReserve &&' ' == pszString[0])
		memmove(&pszString[0], &pszString[1], uStrlen + 1);
}

const char * CHTTPHeader::stristr(_In_z_ const char * _Str, _In_z_ const char * _SubStr)
{
	char * pszString = (char *)m_cAutoBuffer->LockBuffer(strlen(_Str)+1);
	char * pszSubString = (char *)m_cAutoBuffer->LockBuffer(strlen(_SubStr)+1);

	if (NULL == pszString || NULL == pszSubString)
		return NULL;

	strcpy(pszString, _Str);
	strcpy(pszSubString, _SubStr);

	_strlwr(pszString);
	_strlwr(pszSubString);

	const char * pszRetString = strstr(pszString, pszSubString);

	if (pszRetString)
		pszRetString = _Str + (pszRetString - pszString);

	m_cAutoBuffer->UnlockBuffer(pszString);
	m_cAutoBuffer->UnlockBuffer(pszSubString);

	return pszRetString;
}


char * CHTTPHeader::GetNodeNameBuffer(unsigned long uBufferLength)
{
	if (m_pszNodeNameBuffer)
		m_cAutoBuffer->UnlockBuffer(m_pszNodeNameBuffer);

	m_pszNodeNameBuffer = (char *)m_cAutoBuffer->LockBuffer(uBufferLength + 1);

	return m_pszNodeNameBuffer;
}

char * CHTTPHeader::SetHeaderLength(unsigned long uBufferLength)
{
	if (m_pszHeaderBuffer)
		m_cAutoBuffer->UnlockBuffer(m_pszHeaderBuffer);

	m_pszHeaderBuffer = (char *)m_cAutoBuffer->LockBuffer(uBufferLength + 1);

	if (m_pszHTTPHeader)
	{
		strcpy(m_pszHeaderBuffer, m_pszHTTPHeader);
		m_cAutoBuffer->UnlockBuffer(m_pszHTTPHeader);
	}

	m_pszHTTPHeader = m_pszHeaderBuffer;
	m_pszHeaderBuffer = NULL;

	return m_pszHTTPHeader;
}

char * CHTTPHeader::GetNodeStart(const char * pszNodeName)
{
	char * pszNodeStart = m_pszHTTPHeader;
	char * pszNodeNameBuffer = GetNodeNameBuffer(strlen(pszNodeName) + 3);

	do
	{
		if (NULL == pszNodeStart || NULL == pszNodeNameBuffer)
			break;

		sprintf(pszNodeNameBuffer, "\n%s", pszNodeName);

		pszNodeStart = (char *)stristr(pszNodeStart, pszNodeNameBuffer);

		if (NULL == pszNodeStart)
			break;

		pszNodeStart++;

		if (JumpWhitespace(pszNodeStart + strlen(pszNodeName))[0] == ':')
			break;

	} while (NULL != pszNodeStart);

	return pszNodeStart;
}

bool CHTTPHeader::ResolveNode(const char * pszNodeName, char ** pszNodeStart, char ** pszNodeValueStart, char ** pszNodeEnd)
{
	bool bIsOK = false;

	char * pszNodeEndBuffer = NULL;
	char * pszNodeStartBuffer = NULL;
	char * pszNodeValueStartBuffer = NULL;

	if (NULL == pszNodeStart)
		pszNodeStart = &pszNodeStartBuffer;
	if (NULL == pszNodeValueStart)
		pszNodeValueStart = &pszNodeValueStartBuffer;
	if (NULL == pszNodeEnd)
		pszNodeEnd = &pszNodeEndBuffer;

	do
	{
		*pszNodeStart = GetNodeStart(pszNodeName);

		if (NULL == *pszNodeStart)
			break;

		*pszNodeEnd = strchr(*pszNodeStart, '\n');

		if (NULL == *pszNodeEnd)
			break;

		*pszNodeEnd = ('\r' == (*pszNodeEnd - 1)[0]) ? *pszNodeEnd - 1 : *pszNodeEnd;

		if (0 != _strnicmp(*pszNodeStart, pszNodeName, strlen(pszNodeName)))
			break;

		*pszNodeValueStart = *pszNodeStart + strlen(pszNodeName);

		*pszNodeValueStart = (char *)JumpWhitespace(*pszNodeValueStart);

		if (':' != *pszNodeValueStart[0])
			break;

		*pszNodeValueStart = (' ' == (++*pszNodeValueStart)[0]) ? *pszNodeValueStart + 1 : *pszNodeValueStart;

		bIsOK = true;
	} while (false);

	return bIsOK;
}

bool CHTTPHeader::ResolveQuest(char ** pszQuestType, char ** pszPathStart, char ** pszPathEnd, char ** pszVersionStart, char ** pszQuestEnd)
{
	bool bIsOK = false;
	char * pszPathEndBuffer = NULL;
	char * pszQuestEndBuffer = NULL;
	char * pszPathStartBuffer = NULL;
	char * pszQuestTypeBuffer = NULL;
	char * pszVersionStartBuffer = NULL;

	char * pszHTTPHeader = (char *)GetHeader();

	if (NULL == pszPathEnd)
		pszPathEnd = &pszPathEndBuffer;
	if (NULL == pszQuestEnd)
		pszQuestEnd = &pszQuestEndBuffer;
	if (NULL == pszPathStart)
		pszPathStart = &pszPathStartBuffer;
	if (NULL == pszQuestType)
		pszQuestType = &pszQuestTypeBuffer;
	if (NULL == pszVersionStart)
		pszVersionStart = &pszVersionStartBuffer;

	do 
	{
		if (NULL == pszHTTPHeader)
			break;

		if (0 == _strnicmp(pszHTTPHeader, HEADER_QUEST_HTTP_GET, strlen(HEADER_QUEST_HTTP_GET)))
		{
			*pszQuestType = HEADER_QUEST_HTTP_GET;
			*pszPathStart = pszHTTPHeader + strlen(HEADER_QUEST_HTTP_GET);
		}
		else if (0 == _strnicmp(pszHTTPHeader, HEADER_QUEST_HTTP_POST, strlen(HEADER_QUEST_HTTP_GET)))
		{
			*pszQuestType = HEADER_QUEST_HTTP_POST;
			*pszPathStart = pszHTTPHeader + strlen(HEADER_QUEST_HTTP_POST);
		}

		if (NULL == *pszPathStart)
			break;

		*pszPathEnd = strstr(*pszPathStart, " HTTP");

		if (NULL == *pszPathEnd)
			break;

		*pszVersionStart = *pszPathEnd + strlen(" HTTP");

		*pszVersionStart = (char *)JumpWhitespace(*pszVersionStart);

		if ('/' != *pszVersionStart[0])
			break;

		*pszVersionStart = (char *)JumpWhitespace(*pszVersionStart + 1);

		*pszQuestEnd = strchr(*pszPathStart, '\n');

		if (NULL == *pszQuestEnd)
			break;

		*pszQuestEnd = ('\r' == (*pszQuestEnd - 1)[0]) ? *pszQuestEnd - 1 : *pszQuestEnd;

		bIsOK = true;
	} while (false);
	
	return bIsOK;
}

bool CHTTPHeader::ResolveStatus(char ** pszVersionStart, char ** pszVersionEnd, char ** pszStatusStart, char ** pszStatusEnd)
{
	bool bIsOK = false;

	char * pszVersionStartBuffer = NULL;
	char * pszVersionEndBuffer = NULL;
	char * pszStatusStartBuffer = NULL;
	char * pszStatusEndBuffer = NULL;

	char * pszHTTPHeader = (char *)GetHeader();

	if (NULL == pszVersionStart)
		pszVersionStart = &pszVersionStartBuffer;
	if (NULL == pszVersionEnd)
		pszVersionEnd = &pszVersionEndBuffer;
	if (NULL == pszStatusStart)
		pszStatusStart = &pszStatusStartBuffer;
	if (NULL == pszStatusEnd)
		pszStatusEnd = &pszStatusEndBuffer;

	do
	{
		if (NULL == pszHTTPHeader)
			break;

		if (0 != _strnicmp(pszHTTPHeader, HEADER_STATUS_HTTP_HTTP, strlen(HEADER_STATUS_HTTP_HTTP)))
			break;

		*pszVersionStart = pszHTTPHeader + strlen(HEADER_STATUS_HTTP_HTTP);

		*pszVersionEnd = strchr(*pszVersionStart, ' ');

		if (NULL == *pszVersionEnd)
			break;

		*pszStatusStart = (char *)JumpWhitespace(*pszVersionEnd);

		*pszStatusEnd = strchr(*pszStatusStart, ' ');

		if (NULL == *pszStatusEnd)
			break;

		bIsOK = true;
	} while (false);

	return bIsOK;
}

void CHTTPHeader::CreateNode(const char * pszNodeName, const char * pszNodeValue)
{
	int iNewNodeLength = 0;
	char * pszHeaderEnd = strrchr(m_pszHTTPHeader, '\n');

	if (NULL == pszHeaderEnd)
	{
		iNewNodeLength = strlen(pszNodeName) + strlen(pszNodeValue) + 5 + 2;

		pszHeaderEnd = &m_pszHTTPHeader[strlen(m_pszHTTPHeader)];
	}
	else
	{
		iNewNodeLength = strlen(pszNodeName) + strlen(pszNodeValue) + 5;

		pszHeaderEnd = ('\r' == (pszHeaderEnd - 1)[0]) ? pszHeaderEnd - 1 : pszHeaderEnd;
	}

	sprintf(pszHeaderEnd, "%s : %s\r\n\r\n", pszNodeName, pszNodeValue);
}

//////////////////////////////////////////////////////////////////////////

unsigned long CHTTPHeader::GetNode(const char * pszNodeName, char * pszNodeValue /* = NULL */)
{
	int iNodeLength = 0;

	char * pszNodeEnd = NULL;
	char * pszNodeValueStart = NULL;

	do 
	{
		if (false == ResolveNode(pszNodeName, NULL, &pszNodeValueStart, &pszNodeEnd))
			break;

		iNodeLength = pszNodeEnd - pszNodeValueStart;

		if (NULL == pszNodeValue)
			break;

		strncpy(pszNodeValue, pszNodeValueStart, iNodeLength);
		pszNodeValue[iNodeLength] = '\0';
	} while (false);

	return iNodeLength;
}

unsigned long CHTTPHeader::GetURL(char * pszURLValue /* = NULL */)
{
	unsigned long uValueLength = 0;

	do 
	{
		uValueLength = strlen(HEADER_PROTOCOL_HTTP);
		uValueLength += this->GetHost();
		uValueLength += this->GetPath();

		if (NULL == pszURLValue)
			break;

		strcpy(pszURLValue, HEADER_PROTOCOL_HTTP);
		this->GetHost(pszURLValue + strlen(pszURLValue));
		this->GetPath(pszURLValue + strlen(pszURLValue));
	} while (false);

	return uValueLength;
}

unsigned long CHTTPHeader::GetHost(char * pszHostValue /* = NULL */)
{
	return this->GetNode(HEADER_NODE_HTTP_HOST, pszHostValue);
}

unsigned long CHTTPHeader::GetPath(char * pszPathValue /* = NULL */)
{
	unsigned long uValueLength = 0;

	do 
	{
		char * pszPathEnd = NULL;
		char * pszPathStart = NULL;

		if (false == this->ResolveQuest(NULL,&pszPathStart, &pszPathEnd, NULL,NULL))
			break;

		uValueLength = pszPathEnd - pszPathStart;

		if (0 == uValueLength || NULL == pszPathValue)
			break;

		strncpy(pszPathValue, pszPathStart, uValueLength);
		pszPathValue[uValueLength] = '\0';

	} while (false);

	return uValueLength;
}

double CHTTPHeader::GetHTTPVersion()
{
	double fVersion = 0;
	unsigned long uValueLength = 0;

	do
	{
		char * pszVersionEnd = NULL;
		char * pszVersionStart = NULL;

		if (false == this->ResolveQuest(NULL, NULL, NULL, &pszVersionStart, &pszVersionEnd))
			break;

		uValueLength = pszVersionEnd - pszVersionStart;

		if (0 == uValueLength && MAX_PATH > uValueLength)
			break;

		char szVersionBuffer[MAX_PATH + 1] = { 0 };

		strncpy(szVersionBuffer, pszVersionStart, uValueLength);

		szVersionBuffer[uValueLength] = '\0';

		fVersion = atof(szVersionBuffer);

	} while (false);

	return fVersion;
}

unsigned long CHTTPHeader::GetReferer(char * pszRefererValue /* = NULL */)
{
	return this->GetNode(HEADER_NODE_HTTP_REFERER, pszRefererValue);
}

unsigned long CHTTPHeader::GetConnection(char * pszConnectionValue /* = NULL */)
{
	return this->GetNode(HEADER_NODE_HTTP_CONNECTION,pszConnectionValue);
}

unsigned long CHTTPHeader::GetContentLength()
{
	char szContentBuffer[MAX_PATH + 1] = { 0 };
	unsigned long uValueLength = this->GetNode(HEADER_NODE_HTTP_CONTENT_LENGTH, szContentBuffer);

	if (0 == uValueLength)
		return 0;

	return atoi(szContentBuffer);
}

//////////////////////////////////////////////////////////////////////////

unsigned long CHTTPHeader::GetHTTPStatus()
{
	unsigned long uStatusLength = 0;

	do
	{
		char * pszStatusEnd = NULL;
		char * pszStatusStart = NULL;
		char * pszStatusBuffer = NULL;

		if (false == this->ResolveStatus(NULL, NULL, &pszStatusStart, &pszStatusEnd))
			break;

		uStatusLength = pszStatusEnd - pszStatusStart;

		if (0 == uStatusLength)
			break;

		pszStatusBuffer = (char *)m_cAutoBuffer->LockBuffer(uStatusLength + 1);

		strncpy(pszStatusBuffer, pszStatusStart, uStatusLength);
		uStatusLength = atoi(pszStatusBuffer);

		m_cAutoBuffer->UnlockBuffer(pszStatusBuffer);
	} while (false);

	return uStatusLength;
}

bool CHTTPHeader::StuffNode(const char * pszNodeName,const char cStuffChar)
{
	bool bIsOK = false;
	char * pszSaveNodeBuffer = NULL;

	char * pszNodeEnd = NULL;
	char * pszNodeStart = NULL;
	char * pszNodeValueStart = NULL;

	do
	{
		if (false == ResolveNode(pszNodeName, &pszNodeStart, &pszNodeValueStart, &pszNodeEnd))
		{
			bIsOK = true;
			break;
		}

		memset(pszNodeStart, cStuffChar, pszNodeEnd - pszNodeStart);

		pszNodeStart -= 2;

		pszNodeStart[0] = (pszNodeStart[0] == '\r' || pszNodeStart[0] == '\n') ? cStuffChar : pszNodeStart[0];
		pszNodeStart[1] = (pszNodeStart[1] == '\r' || pszNodeStart[1] == '\n') ? cStuffChar : pszNodeStart[1];

		bIsOK = true;
	} while (false);

	if (pszSaveNodeBuffer)
		m_cAutoBuffer->UnlockBuffer(pszSaveNodeBuffer);

	return bIsOK;
}


bool CHTTPHeader::DeleteNode(const char * pszNodeName)
{
	bool bIsOK = false;

	char * pszNodeEnd = NULL;
	char * pszNodeStart = NULL;

	do
	{
		if (false == ResolveNode(pszNodeName, &pszNodeStart, NULL, &pszNodeEnd))
			break;

		int iNodeLength = pszNodeEnd - pszNodeStart;

		iNodeLength += ('\r' == pszNodeEnd[0]) ? 2 : 1;

		memmove(pszNodeEnd - iNodeLength, pszNodeEnd, strlen(pszNodeEnd) + 1);

		bIsOK = true;
	} while (false);

	return bIsOK;
}

//////////////////////////////////////////////////////////////////////////

bool CHTTPHeader::SetNode(const char * pszNodeName, const char * pszNodeValue)
{
	bool bIsOK = false;
	char * pszSaveNodeBuffer = NULL;

	char * pszNodeEnd = NULL;
	char * pszNodeStart = NULL;
	char * pszNodeValueStart = NULL;

	do
	{
		if (false == ResolveNode(pszNodeName, &pszNodeStart, &pszNodeValueStart, &pszNodeEnd))
		{
			bIsOK = true;
			CreateNode(pszNodeName, pszNodeValue);
			break;
		}

		int iOldNodeLength = pszNodeEnd - pszNodeStart;

		pszSaveNodeBuffer = (char *)m_cAutoBuffer->LockBuffer(iOldNodeLength + strlen(pszNodeValue));

		strncpy(pszSaveNodeBuffer, pszNodeStart, pszNodeValueStart - pszNodeStart);
		strcat(pszSaveNodeBuffer, pszNodeValue);

		int iNewNodeLength = strlen(pszSaveNodeBuffer);

		SetHeaderLength(GetHeaderLength() + strlen(pszNodeValue));

		if (false == ResolveNode(pszNodeName, &pszNodeStart, &pszNodeValueStart, &pszNodeEnd))
			break;

		if (iNewNodeLength > iOldNodeLength)
			memmove(pszNodeEnd + (iNewNodeLength - iOldNodeLength), pszNodeEnd, strlen(pszNodeEnd) + 1);
		else
			memmove(pszNodeEnd - (iOldNodeLength - iNewNodeLength), pszNodeEnd, strlen(pszNodeEnd) + 1);

		memcpy(pszNodeStart, pszSaveNodeBuffer, iNewNodeLength);
		bIsOK = true;
	} while (false);

	if (pszSaveNodeBuffer)
		m_cAutoBuffer->UnlockBuffer(pszSaveNodeBuffer);

	return bIsOK;
}

bool CHTTPHeader::SetURL(const char * pszURL)
{
	bool bIsOK = false;
	char szHostBuffer[MAX_PATH + 1] = { 0 };

	do
	{
		if (NULL == pszURL || 0 != _strnicmp(pszURL, HEADER_PROTOCOL_HTTP, strlen(HEADER_PROTOCOL_HTTP)))
			break;

		const char * pszHostStart = pszURL + strlen(HEADER_PROTOCOL_HTTP);

		if (NULL == pszHostStart)
			break;

		const char * pszHostEnd = strchr(pszHostStart, '/');

		if (NULL == pszHostEnd)
			break;
		
		strncpy(szHostBuffer, pszHostStart, pszHostEnd - pszHostStart);
		
		this->SetHost(szHostBuffer);
		this->SetPath(pszHostEnd);
		bIsOK = true;
	} while (false);

	return bIsOK;

}

bool CHTTPHeader::SetHost(const char * pszHost)
{
	return this->SetNode(HEADER_NODE_HTTP_HOST, pszHost);
}

bool CHTTPHeader::SetPath(const char * pszPath)
{
	bool bIsOK = false;
	do
	{
		char * pszPathEnd = NULL;
		char * pszPathStart = NULL;
		char * pszQuestType = NULL;

		if (false == this->ResolveQuest(&pszQuestType,&pszPathStart, &pszPathEnd, NULL,NULL))
			break;

		int iOldPathLength = pszPathEnd - pszPathStart;
		int iNewPathLength = strlen(pszPath);

		SetHeaderLength(GetHeaderLength() + iNewPathLength);

		if (false == this->ResolveQuest(&pszQuestType, &pszPathStart, &pszPathEnd, NULL, NULL))
			break;

		if (iNewPathLength > iOldPathLength)
			memmove(pszPathEnd + (iNewPathLength - iOldPathLength), pszPathEnd, strlen(pszPathEnd) + 1);
		else
			memmove(pszPathEnd - (iOldPathLength - iNewPathLength), pszPathEnd, strlen(pszPathEnd) + 1);

		memcpy(pszPathStart, pszPath, iNewPathLength);
		bIsOK = true;
	} while (false);

	return bIsOK;
}

bool CHTTPHeader::SetHTTPVersion(double fVersion)
{
	bool bIsOK = false;
	do
	{
		char * pszVersionEnd = NULL;
		char * pszVersionStart = NULL;

		if (false == this->ResolveQuest(NULL, NULL, NULL, &pszVersionStart, &pszVersionEnd))
			break;

		int iOldVersionLength = pszVersionEnd - pszVersionStart;

		char szVersionBuffer[MAX_PATH + 1] = { 0 };

		sprintf(szVersionBuffer,"%.1f",fVersion);
		int iNewVersionLength = strlen(szVersionBuffer);

		SetHeaderLength(GetHeaderLength() + iNewVersionLength);

		if (false == this->ResolveQuest(NULL, NULL, NULL, &pszVersionStart, &pszVersionEnd))
			break;

		if (iNewVersionLength > iOldVersionLength)
			memmove(pszVersionEnd + (iNewVersionLength - iOldVersionLength), pszVersionEnd, strlen(pszVersionEnd) + 1);
		else
			memmove(pszVersionEnd - (iOldVersionLength - iNewVersionLength), pszVersionEnd, strlen(pszVersionEnd) + 1);

		memcpy(pszVersionStart, szVersionBuffer, iNewVersionLength);
		bIsOK = true;
	} while (false);

	return bIsOK;
}

bool CHTTPHeader::SetReferer(const char * pszReference)
{
	return this->SetNode(HEADER_NODE_HTTP_REFERER, pszReference);
}

bool CHTTPHeader::SetConnection(const char * pszConnection)
{
	return this->SetNode(HEADER_NODE_HTTP_CONNECTION, pszConnection);
}

bool CHTTPHeader::SetContentLength(unsigned long lContextLength)
{
	char szContentLengthBuffer[MAX_PATH + 1] = { 0 };

	_itoa(lContextLength, szContentLengthBuffer, 10);

	return this->SetNode(HEADER_NODE_HTTP_CONTENT_LENGTH, szContentLengthBuffer);
}
