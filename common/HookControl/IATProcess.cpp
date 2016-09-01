#include "IATProcess.h"
#include <iostream>
#include <sstream>

using namespace std;

CProcess::CProcess(DWORD processID) :
	m_hThread(INVALID_HANDLE_VALUE),
	m_hProcess(INVALID_HANDLE_VALUE),
	m_dwProcessID(processID)
{
	m_hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, processID);

	if (m_hProcess == NULL)
		m_dwErrorcode = GetLastError();
}

CProcess::CProcess(const CProcess& instance)
{
	this->m_hProcess = this->m_hThread = INVALID_HANDLE_VALUE;

	if (!DuplicateHandle(instance.m_hProcess, &this->m_hProcess) || !DuplicateHandle(instance.m_hThread, &this->m_hThread))
		m_dwErrorcode = ERROR_INVALID_HANDLE;

	this->m_dwProcessID = instance.m_dwProcessID;
}

CProcess& CProcess::operator=(const CProcess& instance)
{
	if (!DuplicateHandle(instance.m_hProcess, &this->m_hProcess) || !DuplicateHandle(instance.m_hThread, &this->m_hThread))
		m_dwErrorcode = ERROR_INVALID_HANDLE;

	this->m_dwProcessID = instance.m_dwProcessID;
	return *this;
}

CProcess::~CProcess()
{
	if (m_hProcess != INVALID_HANDLE_VALUE) 
		CloseHandle(m_hProcess);

	if (m_hThread != INVALID_HANDLE_VALUE) 
		CloseHandle(m_hThread);
}

bool CProcess::DuplicateHandle(HANDLE hSource, HANDLE* hTarget)
{
	if (hSource == INVALID_HANDLE_VALUE) 
		return true;

	return (::DuplicateHandle(GetCurrentProcess(), hSource, GetCurrentProcess(), hTarget, 0, FALSE, DUPLICATE_SAME_ACCESS) == TRUE ? true : false);
}

bool CProcess::WriteMemory(LPVOID pAddress, LPCVOID data, DWORD size) const
{
	bool bIsOK = true;
	SIZE_T szWritesize = 0;

	if (FALSE == WriteProcessMemory(m_hProcess, pAddress, data, size, &szWritesize) || szWritesize != size)
		bIsOK = false;

	return bIsOK;
}

bool CProcess::ReadMemory(LPVOID pAddress, LPVOID buffer, DWORD size) const
{
	bool bIsOK = true;
	SIZE_T szReadsize = 0;

	
	if (FALSE == ReadProcessMemory(m_hProcess, pAddress, buffer, size, &szReadsize) || szReadsize != size)
		bIsOK = false;

	return bIsOK;
}

bool CProcess::QueryMemory(LPVOID pAddress, MEMORY_BASIC_INFORMATION * pmbiMemoryinfo) const
{
	bool bIsOK = true;

	if (0 == VirtualQueryEx(m_hProcess, pAddress, pmbiMemoryinfo, sizeof(MEMORY_BASIC_INFORMATION)))
		bIsOK = false;

	return bIsOK;
}

DWORD CProcess::ProtectMemory(LPVOID address, SIZE_T size, DWORD protect) const
{
	DWORD dwOldProtect;

	if (FALSE == VirtualProtectEx(m_hProcess, address, size, protect, &dwOldProtect))
		return 0;

	return dwOldProtect;
}

bool CProcess::StartThread(LPVOID address, LPVOID param)
{
	m_hThread = CreateRemoteThread(m_hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, param, 0, NULL);

	if (m_hThread != NULL)
		SetThreadPriority(m_hThread, THREAD_PRIORITY_TIME_CRITICAL);

	return (m_hThread != NULL);
}

// wait for remote thread to exit and close its handle
void CProcess::WaitForThread()
{
	if (m_hThread == NULL)
		return;

	WaitForSingleObject(m_hThread, INFINITE);

	CloseHandle(m_hThread);
	m_hThread = NULL;
}

LPVOID CProcess::AllocMemory(DWORD size) const
{
	return AllocMemory(size, MEM_RESERVE | MEM_COMMIT);
}

LPVOID CProcess::AllocMemory(DWORD size, DWORD allocationType) const
{
	return AllocMemory(size, NULL, allocationType);
}

LPVOID CProcess::AllocMemory(DWORD size, LPVOID desiredAddress, DWORD allocationType) const
{
	return VirtualAllocEx(m_hProcess, desiredAddress, size, allocationType, PAGE_EXECUTE_READWRITE);
}

bool CProcess::FreeMemory(LPVOID pAddress) const
{
	return (VirtualFreeEx(m_hProcess, pAddress, 0, MEM_RELEASE) != 0);
}

// note: does not work in process start event
std::vector<MODULEENTRY32> CProcess::GetModules()
{
	std::vector<MODULEENTRY32> result;
	HANDLE hModulesSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_dwProcessID);

	do 
	{
		if (hModulesSnap == INVALID_HANDLE_VALUE)
			break;

		MODULEENTRY32 me32;
		me32.dwSize = sizeof(MODULEENTRY32);

		if (!Module32First(hModulesSnap, &me32))
			break;

		do
		{
			result.push_back(me32);
		} while (Module32Next(hModulesSnap, &me32));

		CloseHandle(hModulesSnap);

	} while (false);


	if (hModulesSnap == INVALID_HANDLE_VALUE)
		CloseHandle(hModulesSnap);

	return result;
}

void CProcess::ThrowSysError(const char* msg, DWORD lastError)
{
	m_dwErrorcode = lastError;
}

// also works if process is suspended and not fully initialized yet
uintptr_t CProcess::GetImageBase(HANDLE hThread)
{
	CONTEXT context;
	uintptr_t iImagebase = 0;
	context.ContextFlags = CONTEXT_SEGMENTS;

	do 
	{
		if (!GetThreadContext(hThread, &context))
			break;

		// translate FS selector to virtual address
		LDT_ENTRY ldtEntry;
		if (!GetThreadSelectorEntry(hThread, context.SegFs, &ldtEntry))
			break;

		uintptr_t fsVA = (ldtEntry.HighWord.Bytes.BaseHi) << 24 | (ldtEntry.HighWord.Bytes.BaseMid) << 16 | (ldtEntry.BaseLow);

		SIZE_T read;
		// finally read image based address from PEB:[8]
		if (!(ReadProcessMemory(m_hProcess, (LPCVOID)(fsVA + 0x30), &iImagebase, sizeof(uintptr_t), &read)
			&& ReadProcessMemory(m_hProcess, (LPCVOID)(iImagebase + 8), &iImagebase, sizeof(uintptr_t), &read)))
			break;

	} while (false);

	if (0 == iImagebase)
		m_dwErrorcode = GetLastError();

	return iImagebase;
}

// retrieve image base address for current process
// ONLY works if process has bee initialized, otherwise thread enumeration will fail
// use overloaded function instead
uintptr_t CProcess::GetImageBase()
{
	uintptr_t iImagebase = 0;
	// first get handle to one of the threads in the process
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, m_dwProcessID);

	if (hSnapshot == INVALID_HANDLE_VALUE)
		m_dwErrorcode = GetLastError();

	THREADENTRY32 threadEntry;
	threadEntry.dwSize = sizeof(THREADENTRY32);

	do 
	{
		if (!Thread32First(hSnapshot, &threadEntry))
			break;

		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry.th32ThreadID);

		if (NULL == hThread)
			break;

		try
		{
			uintptr_t iImagebase = GetImageBase(hThread);
			CloseHandle(hThread);
		}
		catch (const std::exception&)
		{
			CloseHandle(hThread);
		}

	} while (false);

	if (0 == iImagebase)
		m_dwErrorcode = GetLastError();

	if (hSnapshot)
		CloseHandle(hSnapshot);

	return iImagebase;
}