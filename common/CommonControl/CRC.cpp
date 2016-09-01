#include "CRC.h"
/*
*  函数名：GetCrc32
*  函数原型：unsigned int GetCrc32(char* InStr,unsigned int len)
*      参数：InStr  ---指向需要计算CRC32值的字符串
*          len  ---为InStr的长度
*      返回值为计算出来的CRC32结果。
*
*  函数名：GetCrc16
*  函数原型：unsigned short GetCrc16(char* InStr,unsigned int len)
*      参数：InStr  ---指向需要计算CRC32值的字符串
*          len  ---为InStr的长度
*      返回值为计算出来的CRC32结果。
*
*    2009/03/26   Edit By iawen
*
*/

unsigned int GetCrc32(char* InStr, unsigned int len){
	//生成Crc32的查询表   
	unsigned int crc_table[256];
	int i, j;
	unsigned int crc;

	for (i = 0; i < 256; i++) {
		crc = (unsigned int)i;
		for (j = 0; j < 8; j++) {
			if (crc & 1)
				crc = 0xedb88320L ^ (crc >> 1);
			else
				crc = crc >> 1;
		}
		crc_table[i] = crc;
	}

	//开始计算CRC32校验值   
	crc = 0xffffffff;
	for (int i = 0; i < len; i++){
		crc = crc_table[(crc ^ InStr[i]) & 0xff] ^ (crc >> 8);
	}

	//Crc ^= 0xFFFFFFFF;
	return crc;
}

unsigned short GetCrc16(char* InStr, unsigned int len){
	//生成Crc16的查询表   
	unsigned short Crc16Table[256];
	unsigned int i, j;
	unsigned short Crc;
	for (i = 0; i < 256; i++)
	{
		Crc = i;
		for (j = 0; j < 8; j++)
		{
			if (Crc & 0x1)
				Crc = (Crc >> 1) ^ 0xA001;
			else
				Crc >>= 1;
		}
		Crc16Table[i] = Crc;
	}

	//开始计算CRC16校验值   
	Crc = 0x0000;
	for (i = 0; i < len; i++){
		Crc = (Crc >> 8) ^ Crc16Table[(Crc & 0xFF) ^ InStr[i]];

	}
	//Crc ^= 0x0000;     
	return Crc;
}