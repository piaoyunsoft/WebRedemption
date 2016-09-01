#include <windows.h>

#include "IATHook.h"

namespace HookControl {

	typedef struct _IMAGE_DELAYLOAD_DESCRIPTOR {
		union {
			DWORD AllAttributes;
			struct {
				DWORD RvaBased : 1;             // Delay load version 2
				DWORD ReservedAttributes : 31;
			};
		} Attributes;

		DWORD DllNameRVA;                       // RVA to the name of the target library (NULL-terminate ASCII string)
		DWORD ModuleHandleRVA;                  // RVA to the HMODULE caching location (PHMODULE)
		DWORD ImportAddressTableRVA;            // RVA to the start of the IAT (PIMAGE_THUNK_DATA)
		DWORD ImportNameTableRVA;               // RVA to the start of the name table (PIMAGE_THUNK_DATA::AddressOfData)
		DWORD BoundImportAddressTableRVA;       // RVA to an optional bound IAT
		DWORD UnloadInformationTableRVA;        // RVA to an optional unload info table
		DWORD TimeDateStamp;                    // 0 if not bound,
												// Otherwise, date/time of the target DLL

	} IMAGE_DELAYLOAD_DESCRIPTOR, *PIMAGE_DELAYLOAD_DESCRIPTOR;

	typedef const IMAGE_DELAYLOAD_DESCRIPTOR *PCIMAGE_DELAYLOAD_DESCRIPTOR;

	struct  PARAMETERS_NOTIFY_IATHOOK_FUNCNAME
	{
		LPCSTR pszTargetFuncName;
		LPCVOID pReplaceFuncAddr;
	};

	bool Notify_IATHook_ByFuncName(IN PIMAGE_THUNK_DATA pCurrentFuncAddrThunk, IN LPCSTR pszCurrentFuncName, LPCVOID pConnext)
	{
		PARAMETERS_NOTIFY_IATHOOK_FUNCNAME * pParameters = (PARAMETERS_NOTIFY_IATHOOK_FUNCNAME *)pConnext;

		if (NULL == pszCurrentFuncName)
			return false;

		if ((IMAGE_SNAP_BY_ORDINAL(ULONG_PTR(pszCurrentFuncName)) && IMAGE_ORDINAL(ULONG_PTR(pParameters->pszTargetFuncName)) != IMAGE_ORDINAL(ULONG_PTR(pszCurrentFuncName))))
			return false;

		if (IMAGE_SNAP_BY_ORDINAL(ULONG_PTR(pszCurrentFuncName)) || 0 == _stricmp(pParameters->pszTargetFuncName, pszCurrentFuncName))
		{
			pCurrentFuncAddrThunk->u1.Function = ULONG_PTR(pParameters->pReplaceFuncAddr);

			return true;
		}

		return false;
	}

	bool IATHook(IN HMODULE hHookModule, IN LPCTSTR pszTargetDllName, IN LPCSTR pszTargetFuncName, IN LPCVOID pReplaceFuncAddr)
	{
		PARAMETERS_NOTIFY_IATHOOK_FUNCNAME tpiParameters = { 0 };

		tpiParameters.pReplaceFuncAddr = pReplaceFuncAddr;
		tpiParameters.pszTargetFuncName = pszTargetFuncName;

		return 0 != SetIATHookNotifyRoutine(hHookModule, pszTargetDllName, Notify_IATHook_ByFuncName, &tpiParameters);
	}

	struct  PARAMETERS_NOTIFY_IATHOOK_FUNCADDR
	{
		LPCVOID pTargetFuncAddr;
		LPCVOID pReplaceFuncAddr;
	};

	bool Notify_IATHook_ByFuncAddr(IN PIMAGE_THUNK_DATA pCurrentFuncAddrThunk, IN LPCSTR pszTargetFuncName, LPCVOID pConnext)
	{
		PARAMETERS_NOTIFY_IATHOOK_FUNCADDR * pParameters = (PARAMETERS_NOTIFY_IATHOOK_FUNCADDR *)pConnext;
		if (CONST ULONG_PTR(pParameters->pTargetFuncAddr) == ULONG_PTR(pCurrentFuncAddrThunk->u1.Function))
		{
			pCurrentFuncAddrThunk->u1.Function = ULONG_PTR(pParameters->pReplaceFuncAddr);

			return true;
		}

		return false;
	}

	bool IATHook(IN HMODULE hHookModule, IN LPCTSTR pszTargetDllName, IN LPCVOID pTargetFuncAddr, IN LPCVOID pReplaceFuncAddr)
	{
		PARAMETERS_NOTIFY_IATHOOK_FUNCADDR tpiParameters = { 0 };

		tpiParameters.pTargetFuncAddr = pTargetFuncAddr;
		tpiParameters.pReplaceFuncAddr = pReplaceFuncAddr;

		return 0 != SetIATHookNotifyRoutine(hHookModule, pszTargetDllName, Notify_IATHook_ByFuncAddr, &tpiParameters);
	}

	int SetIATHookNotifyRoutine(IN HMODULE hHookModule, IN LPCTSTR pszTargetDllName, __pfnIATHookNotifyRoutine pfnIATHookNotifyRoutine, IN LPCVOID pConnext /*= NULL*/)
	{
		WORD wMagic = 0;
		int nHookCount = 0;
		PVOID pCurrentFunction = NULL;
		LPCSTR pszCurrentLibraryName = NULL;

		IMAGE_NT_HEADERS * pImageNtHeader = NULL;
		IMAGE_DOS_HEADER* pImageDosHearder = NULL;

		// 导入表
		IMAGE_THUNK_DATA * pImageThunkDataAddr = NULL;
		IMAGE_THUNK_DATA * pImageNameThunkAddr = NULL;
		IMAGE_IMPORT_DESCRIPTOR* pImageImportDescriptor = NULL;

		// 延迟导入表	
		IMAGE_THUNK_DATA *pImageDelayLoadThunkData = NULL;
		IMAGE_DELAYLOAD_DESCRIPTOR * pImageDelayLoadDescriptor = NULL;

		// 通知参数
		IMAGE_THUNK_DATA iatNotifyFuncAddrThunk = { 0 };

		LPCSTR pszCurrentFuncName = NULL;
		IMAGE_THUNK_DATA * pNotifyFuncAddrThunk = NULL;
		IMAGE_THUNK_DATA * pNotifyFuncNameThunk = NULL;

		DWORD Value = 0, OldProtect = 0, dwNewProtect = 0;
		MEMORY_BASIC_INFORMATION InforMation = { 0 };

		if (NULL == pfnIATHookNotifyRoutine)
			return false;

		pImageDosHearder = (IMAGE_DOS_HEADER*)hHookModule;

		pImageNtHeader = (IMAGE_NT_HEADERS*)((ULONG_PTR)hHookModule + pImageDosHearder->e_lfanew);

		wMagic = pImageNtHeader->OptionalHeader.Magic;

#ifdef _WIN64
		if (IMAGE_NT_OPTIONAL_HDR64_MAGIC != wMagic)
			return nHookCount;
#else
		if (IMAGE_NT_OPTIONAL_HDR32_MAGIC != wMagic)
			return nHookCount;
#endif

		if (pImageNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)
			pImageImportDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)((ULONG_PTR)hHookModule + pImageNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		if (pImageNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress)
			pImageDelayLoadDescriptor = (IMAGE_DELAYLOAD_DESCRIPTOR*)((ULONG_PTR)hHookModule + pImageNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress);

		if (NULL == pImageImportDescriptor && NULL == pImageDelayLoadDescriptor)
			return nHookCount;

		while (pImageImportDescriptor && pImageImportDescriptor->Characteristics != 0)
		{
			pszCurrentLibraryName = (LPCSTR)((ULONG_PTR)hHookModule + pImageImportDescriptor->Name);

			if (0 == _stricmp(pszCurrentLibraryName, pszTargetDllName))
			{
				pImageThunkDataAddr = (IMAGE_THUNK_DATA*)((ULONG_PTR)hHookModule + pImageImportDescriptor->FirstThunk);
				pImageNameThunkAddr = (IMAGE_THUNK_DATA*)((ULONG_PTR)hHookModule + pImageImportDescriptor->OriginalFirstThunk);
				break;
			}

			pImageImportDescriptor++;
		}

		if (pImageThunkDataAddr == NULL && pImageDelayLoadDescriptor)
		{
			while (pImageDelayLoadDescriptor->Attributes.AllAttributes != 0)
			{
				pszCurrentLibraryName = (LPCSTR)((ULONG_PTR)hHookModule + pImageDelayLoadDescriptor->DllNameRVA);

				if (0 == _stricmp(pszCurrentLibraryName, pszTargetDllName))
				{
					pImageThunkDataAddr = (IMAGE_THUNK_DATA*)((ULONG_PTR)hHookModule + pImageDelayLoadDescriptor->ImportAddressTableRVA);
					pImageDelayLoadThunkData = (IMAGE_THUNK_DATA *)((ULONG_PTR)hHookModule + pImageDelayLoadDescriptor->ImportNameTableRVA);
					break;
				}

				pImageDelayLoadDescriptor++;
			}
		}

		if (NULL == pImageThunkDataAddr)
			return nHookCount;

		do
		{
			pCurrentFunction = NULL;
			pszCurrentFuncName = NULL;

#ifdef _WIN64
			CONST BYTE ASM_CODE[] = { 0x48, 0x8d, 0x05 };		//LEA_RAX
#else
			CONST BYTE ASM_CODE[] = { 0xb8 };		//MOV_EAX
#endif

			pNotifyFuncNameThunk = pImageNameThunkAddr;
			pCurrentFunction = (void *)(PIMAGE_THUNK_DATA(pImageThunkDataAddr)->u1.Function);

			if (pImageDelayLoadThunkData)
			{
				pNotifyFuncNameThunk = pImageDelayLoadThunkData;

				if (pCurrentFunction && 0 == _memicmp(pCurrentFunction, ASM_CODE, sizeof(ASM_CODE)))
				{
#ifdef _WIN64
					pCurrentFunction = (void *)(ULONG_PTR(pCurrentFunction) + (sizeof(ASM_CODE) + sizeof(ULONG)) + *PULONG(PUCHAR(pCurrentFunction) + sizeof(ASM_CODE)));
#else
					pCurrentFunction = *(void **)(PUCHAR(pCurrentFunction) + sizeof(ASM_CODE));
#endif
				}

				if (pCurrentFunction && ULONG_PTR(pCurrentFunction) == ULONG_PTR(pImageThunkDataAddr))
				{
					if (IMAGE_SNAP_BY_ORDINAL(PIMAGE_THUNK_DATA(pImageDelayLoadThunkData)->u1.AddressOfData))
					{
						pCurrentFunction = GetProcAddress(GetModuleHandle(pszTargetDllName), (LPCSTR)IMAGE_ORDINAL64(PIMAGE_THUNK_DATA(pImageDelayLoadThunkData)->u1.AddressOfData));
					}
					else
					{
						pCurrentFunction = GetProcAddress(GetModuleHandle(pszTargetDllName), LPCSTR(PIMAGE_IMPORT_BY_NAME((ULONG_PTR)hHookModule + PIMAGE_THUNK_DATA(pImageDelayLoadThunkData)->u1.AddressOfData)->Name));
					}
				}
			}

			if (NULL == pCurrentFunction)
				break;

			pNotifyFuncAddrThunk = &iatNotifyFuncAddrThunk;
			iatNotifyFuncAddrThunk.u1.Function = ULONG_PTR(pCurrentFunction);

			if (pNotifyFuncNameThunk)
			{
				pszCurrentFuncName = (IMAGE_SNAP_BY_ORDINAL(pNotifyFuncNameThunk->u1.AddressOfData)) ?
					LPCSTR(PIMAGE_THUNK_DATA(pNotifyFuncNameThunk)->u1.AddressOfData) :
					LPCSTR(PIMAGE_IMPORT_BY_NAME((ULONG_PTR)hHookModule + PIMAGE_THUNK_DATA(pNotifyFuncNameThunk)->u1.AddressOfData)->Name);
			}

			if (true == pfnIATHookNotifyRoutine(pNotifyFuncAddrThunk, pszCurrentFuncName, pConnext))
			{
				nHookCount++;

				////改写内存保护，以便转换大小写 
				VirtualQuery(pImageThunkDataAddr, &InforMation, sizeof(MEMORY_BASIC_INFORMATION));
				VirtualProtect(InforMation.BaseAddress, InforMation.RegionSize, PAGE_READWRITE, &InforMation.Protect);

				pImageThunkDataAddr->u1.Function = pNotifyFuncAddrThunk->u1.Function;

				VirtualProtect(InforMation.BaseAddress, InforMation.RegionSize, InforMation.Protect, &OldProtect);
			}

			pImageThunkDataAddr++;

			if (pImageNameThunkAddr)
				pImageNameThunkAddr++;

			if (pImageDelayLoadThunkData)
				pImageDelayLoadThunkData++;
		} while (true);	//循环查找目标函数地址所在的位置

		return nHookCount;
	}

	// 	bool IATHook(IN HMODULE hHookModule, IN LPCTSTR pszTargetDllName, IN LPCVOID pTargetFuncAddr, IN LPCVOID pReplaceFuncAddr)
	// 	{
	// 		bool bIsOK = false;
	// 		WORD wMagic = 0;
	// 		PVOID pCurrentFunction = NULL;
	// 		LPCSTR pszCurrentLibraryName = NULL;
	// 
	// 		IMAGE_DOS_HEADER* pImageDosHearder = NULL;
	// 		IMAGE_NT_HEADERS32* pImageNtHeader32 = NULL;
	// 		IMAGE_NT_HEADERS64* pImageNtHeader64 = NULL;
	// 
	// 		// 导入表
	// 		IMAGE_THUNK_DATA32 * pImageThunkDataAddr = NULL;
	// 		IMAGE_IMPORT_DESCRIPTOR* pImageImportDescriptor = NULL;
	// 
	// 		// 延迟导入表	
	// 		IMAGE_THUNK_DATA *pImageDelayLoadThunkData = NULL;
	// 		IMAGE_DELAYLOAD_DESCRIPTOR * pImageDelayLoadDescriptor = NULL;
	// 
	// 		DWORD Value = 0, OldProtect = 0, dwNewProtect = 0;
	// 		MEMORY_BASIC_INFORMATION InforMation = { 0 };
	// 
	// 		if (NULL == pTargetFuncAddr || NULL == pReplaceFuncAddr)
	// 			return false;
	// 
	// 		pImageDosHearder = (IMAGE_DOS_HEADER*)hHookModule;
	// 
	// 		pImageNtHeader32 = (IMAGE_NT_HEADERS32*)((ULONG_PTR)hHookModule + pImageDosHearder->e_lfanew);
	// 		pImageNtHeader64 = (IMAGE_NT_HEADERS64*)((ULONG_PTR)hHookModule + pImageDosHearder->e_lfanew);
	// 
	// 		wMagic = pImageNtHeader32->OptionalHeader.Magic;
	// 
	// 		if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == wMagic)
	// 		{
	// 			pImageImportDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)((ULONG_PTR)hHookModule + pImageNtHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	// 			pImageDelayLoadDescriptor = (IMAGE_DELAYLOAD_DESCRIPTOR*)((ULONG_PTR)hHookModule + pImageNtHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress);
	// 		}
	// 		else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == wMagic)
	// 		{
	// 			pImageImportDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)((ULONG_PTR)hHookModule + pImageNtHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	// 			pImageDelayLoadDescriptor = (IMAGE_DELAYLOAD_DESCRIPTOR*)((ULONG_PTR)hHookModule + pImageNtHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress);
	// 		}
	// 		else
	// 		{
	// 			pImageImportDescriptor = NULL;
	// 			pImageDelayLoadDescriptor = NULL;
	// 		}
	// 
	// 		if (NULL == pImageImportDescriptor)
	// 			return bIsOK;
	// 
	// 		while (pImageImportDescriptor->Characteristics != 0)
	// 		{
	// 			pszCurrentLibraryName = (LPCSTR)((ULONG_PTR)hHookModule + pImageImportDescriptor->Name);
	// 
	// 			if (0 == pImageImportDescriptor->TimeDateStamp && 0 == pImageImportDescriptor->ForwarderChain)
	// 			{
	// 				if (0 == stricmp(pszCurrentLibraryName, pszTargetDllName))
	// 				{
	// 					pImageThunkDataAddr = (IMAGE_THUNK_DATA32*)((ULONG_PTR)hHookModule + pImageImportDescriptor->FirstThunk);
	// 					break;
	// 				}
	// 			}
	// 
	// 			pImageImportDescriptor++;
	// 		}
	// 
	// 		if (pImageThunkDataAddr == NULL && pImageDelayLoadDescriptor && 0 != pImageDelayLoadDescriptor->Attributes.AllAttributes)
	// 		{
	// 			while (pImageDelayLoadDescriptor->Attributes.AllAttributes != 0)
	// 			{
	// 				pszCurrentLibraryName = (LPCSTR)((ULONG_PTR)hHookModule + pImageDelayLoadDescriptor->DllNameRVA);
	// 
	// 				if (0 == pImageDelayLoadDescriptor->TimeDateStamp)
	// 				{
	// 					if (0 == stricmp(pszCurrentLibraryName, pszTargetDllName))
	// 					{
	// 						pImageThunkDataAddr = (IMAGE_THUNK_DATA32*)((ULONG_PTR)hHookModule + pImageDelayLoadDescriptor->ImportAddressTableRVA);
	// 						pImageDelayLoadThunkData = (IMAGE_THUNK_DATA *)((ULONG_PTR)hHookModule + pImageDelayLoadDescriptor->ImportNameTableRVA);
	// 						break;
	// 					}
	// 				}
	// 
	// 				pImageDelayLoadDescriptor++;
	// 			}
	// 		}
	// 
	// 		if (NULL == pImageThunkDataAddr)
	// 			return bIsOK;
	// 
	// 		do
	// 		{
	// 			pCurrentFunction = NULL;
	// 
	// 			if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == wMagic)
	// 			{
	// 				CONST BYTE MOV_EAX[] = { 0xb8 };
	// 				pCurrentFunction = (void *)(PIMAGE_THUNK_DATA32(pImageThunkDataAddr)->u1.Function);
	// 
	// 				if (pCurrentFunction && 0 == memicmp(pCurrentFunction, MOV_EAX, sizeof(MOV_EAX)))
	// 				{
	// 					pCurrentFunction = *(void **)(PUCHAR(pCurrentFunction) + sizeof(MOV_EAX));
	// 				}
	// 			}
	// 			else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == wMagic)
	// 			{
	// 				CONST BYTE LEA_RAX[] = { 0x48, 0x8d, 0x05 };
	// 				pCurrentFunction = (void *)(PIMAGE_THUNK_DATA64(pImageThunkDataAddr)->u1.Function);
	// 
	// 				if (pCurrentFunction && 0 == memicmp(pCurrentFunction, LEA_RAX, sizeof(LEA_RAX)))
	// 				{
	// 					pCurrentFunction = (void *)(ULONG_PTR(pCurrentFunction) + (sizeof(LEA_RAX) + sizeof(ULONG)) + *PULONG(PUCHAR(pCurrentFunction) + sizeof(LEA_RAX)));
	// 				}
	// 			}
	// 
	// 			if (NULL == pCurrentFunction)
	// 				break;
	// 
	// 			if (pImageDelayLoadThunkData && ULONG_PTR(pCurrentFunction) == ULONG_PTR(pImageThunkDataAddr))
	// 			{
	// 				if (IMAGE_SNAP_BY_ORDINAL(pImageDelayLoadThunkData->u1.AddressOfData))
	// 				{
	// 					pCurrentFunction = GetProcAddress(GetModuleHandle(pszTargetDllName), (LPCSTR)IMAGE_ORDINAL(pImageDelayLoadThunkData->u1.AddressOfData));
	// 				}
	// 				else
	// 				{
	// 					pCurrentFunction = GetProcAddress(GetModuleHandle(pszTargetDllName), (LPCSTR)PIMAGE_IMPORT_BY_NAME((ULONG_PTR)hHookModule + pImageDelayLoadThunkData->u1.AddressOfData)->Name);
	// 				}
	// 			}
	// 
	// 			if (CONST ULONG_PTR(pTargetFuncAddr) == ULONG_PTR(pCurrentFunction))
	// 			{
	// 				////改写内存保护，以便转换大小写 
	// 				VirtualQuery(pImageThunkDataAddr, &InforMation, sizeof(MEMORY_BASIC_INFORMATION));
	// 				VirtualProtect(InforMation.BaseAddress, InforMation.RegionSize, PAGE_READWRITE, &InforMation.Protect);
	// 
	// 				*((const void **)pImageThunkDataAddr) = pReplaceFuncAddr;
	// 
	// 				VirtualProtect(InforMation.BaseAddress, InforMation.RegionSize, InforMation.Protect, &OldProtect);
	// 
	// 				bIsOK = true;
	// 				break;
	// 			}
	// 
	// 			if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == wMagic)
	// 			{
	// 				(*(PIMAGE_THUNK_DATA32 *)&pImageThunkDataAddr)++;
	// 			}
	// 			else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == wMagic)
	// 			{
	// 				(*(PIMAGE_THUNK_DATA64 *)&pImageThunkDataAddr)++;
	// 			}
	// 
	// 			if (pImageDelayLoadThunkData)
	// 				pImageDelayLoadThunkData++;
	// 
	// 		} while (true);	//循环查找目标函数地址所在的位置
	// 
	// 		return bIsOK;
	// 	}

	bool IATHooks(IN HMODULE hHookModule, IN LPCVOID pTargetFuncAddr, IN LPCVOID pReplaceFuncAddr)
	{
		bool bIsOK = false;

		return bIsOK;
	}
}
