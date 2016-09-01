#pragma once

#include <iostream>
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>

class CProcess
{
public:

	//Process(HANDLE hProcess);
	CProcess(DWORD processID);
	CProcess::CProcess(const CProcess& instance);
	CProcess& CProcess::operator=(const CProcess& instance);
	~CProcess();
	
	LPVOID AllocMemory(DWORD size) const;
	LPVOID AllocMemory(DWORD size, DWORD allocationType) const;
	LPVOID AllocMemory(DWORD size, LPVOID desiredAddress, DWORD allocationType) const;

	bool FreeMemory(LPVOID pAddress) const;
	bool WriteMemory(LPVOID pAddress, LPCVOID data, DWORD size) const;
	bool ReadMemory(LPVOID pAddress, LPVOID buffer, DWORD size) const;

	bool QueryMemory(LPVOID pAddress, MEMORY_BASIC_INFORMATION * pmbiMemoryinfo) const;
	DWORD ProtectMemory(LPVOID pAddress, SIZE_T size, DWORD protect) const;

	void WaitForThread();
	bool StartThread(LPVOID pAddress, LPVOID param);

	std::vector<MODULEENTRY32> CProcess::GetModules();

	uintptr_t GetImageBase();
	uintptr_t GetImageBase(HANDLE hThread);

private:
	bool DuplicateHandle(HANDLE hSrc, HANDLE* hDest);
	void ThrowSysError(const char* msg, DWORD lastError);

	HANDLE m_hThread;
	HANDLE m_hProcess;

	DWORD m_dwProcessID;
	DWORD m_dwErrorcode;
};

// handle error
class ProcessHandleException : public std::runtime_error
{
public:
	ProcessHandleException::ProcessHandleException(const std::string& msg) : std::runtime_error(msg) {};
};

// anything with memory
class ProcessMemoryException : public std::runtime_error
{
public:
	ProcessMemoryException::ProcessMemoryException(const std::string& msg, LPVOID address) : std::runtime_error(msg), address_(address) {};
	LPVOID getAddress() { return address_; };
private:
	LPVOID address_;
};

// access memory
class MemoryAccessException : public std::runtime_error
{
public:
	MemoryAccessException::MemoryAccessException(const std::string& msg) : std::runtime_error(msg) {};	
};

// allocate
class MemoryAllocationException : public std::runtime_error
{
public:
	MemoryAllocationException::MemoryAllocationException(const std::string& msg) : std::runtime_error(msg) {};	
};

// query memory
class MemoryQueryException : public std::runtime_error
{
public:
	MemoryQueryException::MemoryQueryException(const std::string& msg) : std::runtime_error(msg) {};	
};

// protect memory
class MemoryProtectException : public ProcessMemoryException
{
public:
	MemoryProtectException::MemoryProtectException(const std::string& msg, LPVOID address) : ProcessMemoryException(msg, address) {};	
};