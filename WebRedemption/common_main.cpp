#include "common_main.h"
#include "common_helper.h"

#include <ws2tcpip.h>

#include "CommonControl\Commonfun.h"
#include "export_function.h"

#pragma comment(lib,"ws2_32.lib")
#pragma  comment(lib,DIRECTORY_LIB_INTERNAL "HookControl.lib")
#pragma  comment(lib,DIRECTORY_LIB_INTERNAL "CommonControl.lib")


namespace Global {
	CDebug Log("HTTP_Redirect.log");
	sockaddr_in addrEncodeSocket = { 0 };
	PBUSINESS_DATA pBusinessData = NULL;
}

BOOL APIENTRY DllMain(_In_ HINSTANCE hDllHandle, _In_ DWORD dwReason, _In_opt_ void * _Reserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) {
		char szModuleName[MAX_PATH + 1] = { 0 };
		GetModuleFileNameA(Common::GetModuleHandleByAddr(DllMain), szModuleName, MAX_PATH);

		if (LockModule(szModuleName, &hDllHandle)) {
			StartBusiness(NULL);
		}
	}

	return TRUE;
}