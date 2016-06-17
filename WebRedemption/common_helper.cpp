#include "common_helper.h"

const char * __inet_ntop(int af, in_addr src_addr, char *dst, socklen_t cnt)
{
	if (af == AF_INET)
	{
		struct sockaddr_in in;
		memset(&in, 0, sizeof(in));
		in.sin_family = AF_INET;
		memcpy(&in.sin_addr, &src_addr, sizeof(struct in_addr));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	else if (af == AF_INET6)
	{
		struct sockaddr_in6 in;
		memset(&in, 0, sizeof(in));
		in.sin6_family = AF_INET6;
		memcpy(&in.sin6_addr, &src_addr, sizeof(struct in_addr6));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	return NULL;
}

const wchar_t * __inet_ntopw(int af, in_addr src_addr, wchar_t *dst, socklen_t cnt)
{
	if (af == AF_INET)
	{
		struct sockaddr_in in;
		memset(&in, 0, sizeof(in));
		in.sin_family = AF_INET;
		memcpy(&in.sin_addr, &src_addr, sizeof(struct in_addr));
		GetNameInfoW((struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	else if (af == AF_INET6)
	{
		struct sockaddr_in6 in;
		memset(&in, 0, sizeof(in));
		in.sin6_family = AF_INET6;
		memcpy(&in.sin6_addr, &src_addr, sizeof(struct in_addr6));
		GetNameInfoW((struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	return NULL;
}



BOOL GetWSAExFunction(GUID&funGuid, void** ppFunction) {//本函数利用参数返回函数指针
	SOCKET skTemp = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED); //随便创建一个SOCKET供WSAIoctl使用 并不一定要像下面这样创建
	if (INVALID_SOCKET == skTemp) { //通常表示没有正常的初始化WinSock环境
		return FALSE;
	}

	*ppFunction = NULL;
	DWORD dwBytes = 0;
	::WSAIoctl(skTemp, SIO_GET_EXTENSION_FUNCTION_POINTER, &funGuid, sizeof(funGuid), ppFunction, sizeof(*ppFunction), &dwBytes, NULL, NULL);
	::closesocket(skTemp);

	return NULL != *ppFunction;
}
