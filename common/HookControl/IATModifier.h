#pragma once

#include <iostream>
#include <vector>
#include "IATProcess.h"

class IATModifier
{
public:

	IATModifier(const CProcess& process);
	~IATModifier();
	std::vector<IMAGE_IMPORT_DESCRIPTOR> readImportTable();
	bool SetImageBase(uintptr_t address);
	bool WriteIAT(const std::vector<std::string>& dlls);
	bool WriteIAT(const char * pszDllPath);
	IMAGE_NT_HEADERS readNTHeaders() const;

private:

	DWORD pad(DWORD val, DWORD amount) { return (val+amount) & ~amount; };
	DWORD padToDword(DWORD val) { return pad(val, 3); };
	void* allocateMemAboveBase(void* baseAddress, size_t size);
	CProcess m_cProcess;
	uintptr_t ntHeadersAddr_;

	unsigned int m_uImportDescrTblSize;
	PIMAGE_IMPORT_DESCRIPTOR m_pImportDescrTblAddr;
};