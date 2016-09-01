#pragma once
#include  <stdio.h>
#include <Windows.h>
#include  "Competence.h"
#include <iphlpapi.h>

#pragma comment(lib,"iphlpapi.lib")

namespace Common{

	unsigned long IP2LONG(const char *ip)
	{
		char szAddress[MAX_IP4_LEN] = { 0 };

		sscanf(ip, "%d.%d.%d.%d", szAddress, szAddress + 1, szAddress + 2, szAddress + 3);

		return *((unsigned long*)(&szAddress));
	}

	bool GetMACAddr(char * pszMACBuffer, char * pszIPBuffer /*= NULL*/)
	{
		bool bIsOK = false;
		unsigned long uBufferLength = 0;//得到结构体大小,用于GetAdaptersInfo参数
		PIP_ADAPTER_INFO pIpAdapterInfo = NULL; //PIP_ADAPTER_INFO结构体指针存储本机网卡信息

		unsigned long uStatus = GetAdaptersInfo(pIpAdapterInfo, &uBufferLength);//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量

		if (ERROR_BUFFER_OVERFLOW == uStatus) //如果函数返回的是ERROR_BUFFER_OVERFLOW则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
		{
			pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[uBufferLength];
			uStatus = GetAdaptersInfo(pIpAdapterInfo, &uBufferLength);//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		}

		do 
		{
			if (ERROR_SUCCESS != uStatus)
				break;

			for (UINT i = 0; i < 6; i++, pszMACBuffer += 3)
				sprintf(pszMACBuffer, "%02x%c", pIpAdapterInfo->Address[i], (i == pIpAdapterInfo->AddressLength - 1) ? '\0' : '-');

			if (pszIPBuffer)
				memcpy(pszIPBuffer, pIpAdapterInfo->IpAddressList.IpAddress.String, 16);

			bIsOK = true;
		} while (false);

		if (pIpAdapterInfo)
			delete pIpAdapterInfo;

		return bIsOK;
	}

	void MACConverter(char * pszMACString, byte * pszMACBuffer)
	{
		for (UINT i = 0; i < 6; i++, pszMACBuffer += 3)
			sscanf(pszMACString, "%02x-%02x-%02x-%02x-%02x-%02x", pszMACBuffer, pszMACBuffer + 1, pszMACBuffer + 2, pszMACBuffer + 3, pszMACBuffer + 4, pszMACBuffer + 5);

	}

	void MACConverter(byte * pszMACBuffer, char * pszMACString)
	{
		for (UINT i = 0; i < 6; i++, pszMACString += 3)
			sprintf(pszMACString, "%02x%c", pszMACBuffer[i], (i == 5) ? '\0' : '-');
	}

	bool GetMACAddr(byte * pszMACBuffer)
	{
		unsigned long uBufferLength = 0;//得到结构体大小,用于GetAdaptersInfo参数
		PIP_ADAPTER_INFO pIpAdapterInfo = NULL; //PIP_ADAPTER_INFO结构体指针存储本机网卡信息

		unsigned long uStatus = GetAdaptersInfo(pIpAdapterInfo, &uBufferLength);//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量

		if (ERROR_BUFFER_OVERFLOW == uStatus) //如果函数返回的是ERROR_BUFFER_OVERFLOW则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
		{
			pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[uBufferLength];
			uStatus = GetAdaptersInfo(pIpAdapterInfo, &uBufferLength);//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		}

		if (ERROR_SUCCESS != uStatus)
			return false;

		if (pszMACBuffer)
			memcpy(pszMACBuffer, pIpAdapterInfo->Address, 8);

		if (pIpAdapterInfo)
			delete pIpAdapterInfo;

		return true;
	}
};
