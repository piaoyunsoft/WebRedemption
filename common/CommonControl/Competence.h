#pragma once

#include "Commondef.h"

#pragma comment(lib,"iphlpapi.lib")

namespace Common{
	unsigned long IP2LONG(const char *ip);
	bool GetMACAddr(byte * pszMACBuffer);
	void MACConverter(byte * pszMACBuffer, char * pszMACString);
	void MACConverter(char * pszMACString, byte * pszMACBuffer);
	bool GetMACAddr(char * pszMACBuffer, char * pszIPBuffer);

}


namespace Common {

	typedef unsigned char byte;

	static unsigned int GetCrc32(const void * InStr, int len) {
		//生成Crc32的查询表   
		unsigned int Crc32Table[256];
		int i, j;
		unsigned int Crc;
		for (i = 0; i < 256; i++) {
			Crc = i;
			for (j = 0; j < 8; j++) {
				if (Crc & 1)
					Crc = (Crc >> 1) ^ 0xEDB88320;
				else
					Crc >>= 1;
			}
			Crc32Table[i] = Crc;
		}

		//开始计算CRC32校验值   
		Crc = 0xffffffff;
		for (int i = 0; i<len; i++) {
			Crc = (Crc >> 8) ^ Crc32Table[(Crc & 0xFF) ^ ((const char *)InStr)[i]];
		}

		Crc ^= 0xFFFFFFFF;
		return Crc;
	}


	static byte * GetXor(byte * pData, int nDatalen, const byte * pKey, int nKeylen)
	{
		int i = 0, k = 0;
		for (i = 0; i < nDatalen; i++)
		{
			if (k == nKeylen)
				k = 0;

			pData[i] = pData[i] ^ pKey[k];
		}

		return pData;
	}

	static void exchange(byte *a, byte *b)
	{
		*a ^= *b;
		*b ^= *a;
		*a ^= *b;
	}

	static byte * Encrypt(const void * pData, int nDatalen, const void * pbyKey1, int nKey1len, const void * pbyKey2, int nKey2len, int * nEncryptlen)
	{
		int i = 0;
		int nCrclen = 255;
		int nKey1CRC = 0;
		int nConnectCRC[5] = { 0 };
		byte * pEncryptdata = NULL;
		byte * pEncryptblock = NULL;
		const byte * pbySourceData = NULL;

		pbySourceData = (const byte *)pData;

		int nNewDatalen = nDatalen + sizeof(nConnectCRC);

		if (nDatalen < 255 * 2)
			nCrclen = nDatalen / 2;

		nConnectCRC[1] = GetCrc32(pbySourceData, nCrclen);
		nConnectCRC[2] = GetCrc32(pbySourceData + nDatalen - nCrclen, nCrclen);
		nConnectCRC[3] = nDatalen - GetCrc32(pbyKey2, nKey2len);
		nConnectCRC[4] = GetCrc32(pbySourceData + (nDatalen / 2) - (nCrclen / 2), nCrclen);
		nConnectCRC[0] = GetCrc32((byte *)&nConnectCRC[1], sizeof(int)* 4);

		nCrclen = nNewDatalen / sizeof(nConnectCRC)+1;
		nNewDatalen += (sizeof(nConnectCRC)-(nDatalen % sizeof(nConnectCRC)));

		nKey1CRC = GetCrc32(pbyKey1, nKey1len);
		GetXor((byte *)&nConnectCRC[1], sizeof(int)* 4, (byte *)&nKey1CRC, sizeof(int));

		*nEncryptlen = nNewDatalen;
		pEncryptdata = (byte *)malloc(*nEncryptlen);

		memset(pEncryptdata, 0, *nEncryptlen);

		pEncryptblock = pEncryptdata;
		memcpy(pEncryptdata, nConnectCRC, sizeof(nConnectCRC));
		memcpy(pEncryptdata + sizeof(nConnectCRC), pData, nDatalen);

		for (i = 0; i < sizeof(nConnectCRC); i++)
		{
			int n = 0;
			nDatalen = nCrclen / 2;

			for (n = 0; n < nDatalen; n++)
			{
				exchange(&pEncryptblock[n], &pEncryptblock[nCrclen - n - 1]);
			}

			pEncryptblock += nCrclen;
		}

		return pEncryptdata;
	}

	static byte * Decrypt(const void * pData, int nDatalen, const void * pbyKey1, int nKey1len, const void * pbyKey2, int nKey2len, int * nEncryptlen)
	{
		int i = 0;
		int nCrclen = 0;
		int nKey1CRC = 0;
		int nDecryptlen = 0;
		byte * pRetdatas = NULL;
		byte * pbyTempData = NULL;
		byte *  pDecryptdata = NULL;
		byte * pEncryptblock = NULL;
		int nConnectCRC[5] = { 0 };

		if (0 != nDatalen % sizeof(nConnectCRC))
			return NULL;

		pbyTempData = (byte *)malloc(nDatalen);
		memcpy(pbyTempData, pData, nDatalen);

		pEncryptblock = pbyTempData;
		nCrclen = nDatalen / sizeof(nConnectCRC);

		for (i = 0; i < sizeof(nConnectCRC); i++)
		{
			int n = 0;
			nDatalen = nCrclen / 2;

			for (n = 0; n < nDatalen; n++)
			{
				exchange(&pEncryptblock[nCrclen - n - 1], &pEncryptblock[n]);
			}

			pEncryptblock += nCrclen;
		}

		memcpy(nConnectCRC, pbyTempData, sizeof(nConnectCRC));

		//////////////////////////////////////////////////////////////////////////
		// 效验加密头

		pDecryptdata = NULL;
		nKey1CRC = GetCrc32(pbyKey1, nKey1len);

		do
		{
			if (nConnectCRC[0] != GetCrc32(GetXor((byte *)&nConnectCRC[1], sizeof(int)* 4, (byte *)&nKey1CRC, sizeof(int)), sizeof(int)* 4))
				break;

			//////////////////////////////////////////////////////////////////////////
			// 获取解密文本长度

			nDecryptlen = nConnectCRC[3] + GetCrc32(pbyKey2, nKey2len);

			pDecryptdata = (byte *)malloc(nDecryptlen);
			memcpy(pDecryptdata, pbyTempData + sizeof(nConnectCRC), nDecryptlen);


			//////////////////////////////////////////////////////////////////////////
			// 效验解密数据

			nCrclen = 255;

			if (nDecryptlen < 255 * 2)
				nCrclen = nDecryptlen / 2;

			if (nConnectCRC[1] != GetCrc32(pDecryptdata, nCrclen))
				break;

			if (nConnectCRC[2] != GetCrc32(pDecryptdata + nDecryptlen - nCrclen, nCrclen))
				break;

			if (nConnectCRC[4] != GetCrc32(pDecryptdata + (nDecryptlen / 2) - (nCrclen / 2), nCrclen))
				break;

			pRetdatas = pDecryptdata;
			*nEncryptlen = nDecryptlen;
		} while (0);

		if (pbyTempData)
			free(pbyTempData);

		if (NULL != pDecryptdata && NULL == pRetdatas)
			free(pDecryptdata);

		return pRetdatas;
	}
}