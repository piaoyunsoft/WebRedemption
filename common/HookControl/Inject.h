#pragma once


namespace HookControl {

	bool InjectDll(DWORD dwProcessID, LPCTSTR szInjectFileFullPath);
	bool InjectDll(DWORD dwProcessID, LPCTSTR pcszInjectFileFullPath, HINSTANCE * phRemoteModule = NULL);
};

namespace HookControl {

	typedef struct  _INJECT_SHELLCODEINFO
	{
		SIZE_T  sizeShellCodeSize;
		const void *   pShellCodePointer;

		SIZE_T  sizeShellCodeDataPos;
		void *   pShellCodeDataPointer;
	}INJECT_SHELLCODEINFO, * PINJECT_SHELLCODEINFO;

	bool InjectShellCode(HANDLE hTargetProcess, PINJECT_SHELLCODEINFO pInjectShellcode, const void * pRoutineContext = NULL, SIZE_T sizeContextSize = 0);
	bool InjectShellCode(DWORD dwTargetProcessID, PINJECT_SHELLCODEINFO pInjectShellcode, const void * pRoutineContext = NULL, SIZE_T sizeContextSize = 0);
};
