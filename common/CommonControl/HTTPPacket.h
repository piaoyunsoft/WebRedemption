#pragma once
#include "Commondef.h"
#include "AutoBuffer.h"

#define MAX_LENGTH_URL					4096

#define HEADER_PROTOCOL_HTTP				"http://"
#define HEADER_PROTOCOL_HTTPS				"https://"

#define HEADER_QUEST_HTTP_GET								"GET "
#define HEADER_QUEST_HTTP_POST								"POST "
#define HEADER_STATUS_HTTP_HTTP								"HTTP/"

#define HEADER_NODE_HTTP_HOST																"Host"
#define HEADER_NODE_HTTP_COOKIE																"Cookie"
#define HEADER_NODE_HTTP_REFERER															"Referer"
#define HEADER_NODE_HTTP_CONNECTION															"Connection"
#define HEADER_NODE_HTTP_CONTENT_TYPE														"Content-Type"
#define HEADER_NODE_HTTP_CONTENT_LENGTH														"Content-Length"
#define HEADER_NODE_HTTP_ACCEPT_ENCODING													"Accept-Encoding"
#define HEADER_NODE_HTTP_CONTENT_ENCODING													"Content-Encoding"
#define HEADER_NODE_HTTP_TRANSFER_ENCODING													"Transfer-Encoding"

#define STATUS_HTTP_SUCCESS																			200

#define STATUS_HTTP_FOUND																			302
#define STATUS_HTTP_NOTMODIFIED																304
#define STATUS_HTTP_MOVEDPERMANENTLY													301

#define STATUS_HTTP_NOTFOUND																	404

#define DEFAULT_HEADER_HTTP										"GET / HTTP/1.1\r\nHost:localhost\r\n\r\n"

class CHTTPHeader{
public:
	CHTTPHeader(CAutoBuffer & cAutoBuffer);
	CHTTPHeader(const char * pszPacketContext = NULL, CAutoBuffer * pAutoBuffer = NULL);
	~CHTTPHeader();

protected:
	void Init(const char * pszPacketContext = NULL, CAutoBuffer * pAutoBuffer = NULL);

public:
	bool StuffNode(const char * pszNodeName,const char cStuffChar = ' ');
	bool ResetNode(const char * pszNodeName);
	bool DeleteNode(const char * pszNodeName);

public:
	unsigned long GetNode(const char * pszNodeName, char * pszNodeValue = NULL);
	unsigned long GetURL(char * pszURLValue = NULL);
	unsigned long GetHost(char * pszHostValue = NULL);
	unsigned long GetPath(char * pszPathValue = NULL);
	unsigned long GetReferer(char * pszRefererValue = NULL);
	unsigned long GetConnection(char * pszConnectionValue = NULL);

	unsigned long GetHTTPStatus();

	double GetHTTPVersion();
	unsigned long GetContentLength();

	const char * GetHeader();
	unsigned long GetHeaderLength();

public:
	bool SetNode(const char * pszNodeName, const char * pszNodeValue);
	bool SetURL(const char * pszURL);
	bool SetHost(const char * pszHost);
	bool SetPath(const char * pszPath);
	bool SetReferer(const char * pszReference);
	bool SetConnection(const char * pszConnection);

	bool SetHTTPVersion(double fVersion);
	bool SetContentLength(unsigned long lContextLength);

	char * SetHeaderLength(unsigned long uBufferLength);
	const char * SetHeader(const char * pszHTTPHeader, unsigned long uHeaderLength = 0);

protected:
	char * GetNodeStart(const char * pszNodeName);

	bool ResolveQuest(char ** pszQuestType, char ** pszPathStart, char ** pszPathEnd, char ** pszVersionStart, char ** pszQuestEnd);
	bool ResolveStatus(char ** pszVersionStart, char ** pszVersionEnd, char ** pszStatusStart, char ** pszStatusEnd);
	bool ResolveNode(const char * pszNodeName, char ** pszNodeStart, char ** pszNodeValueStart, char ** pszNodeEnd);

	const char * JumpWhitespace(const char * pszString);
	void TrimBlank(char * pszString, bool bReserve = true);

	void CreateNode(const char * pszNodeName, const char * pszNodeValue);

	char * GetFullNodeBuffer(unsigned long uBufferLength);
	char * GetNodeNameBuffer(unsigned long uBufferLength);

	const char * stristr(_In_z_ const char * _Str, _In_z_ const char * _SubStr);

protected:
	char * m_pszHTTPHeader;

	char * m_pszHeaderBuffer;
	char * m_pszNodeNameBuffer;

	CAutoBuffer * m_cAutoBuffer;
};