#pragma once
#include "HookHelp.h"
#include "InlineHook.h"


namespace HookControl{

	typedef struct _HOOKCONTROL_POSTMESSAGE
	{
		void * CallAddress;
		BOOL RetValue;
	}HOOKCONTROL_POSTMESSAGE, *PHOOKCONTROL_POSTMESSAGE;

	typedef BOOL(WINAPI * __pfnPostMessage)(HWND, UINT, WPARAM, LPARAM);
	
	extern __pfnPostMessage pfnPostMessage;

	bool OnBeforePostMessage(HOOKCONTROL_POSTMESSAGE * retStatuse, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnAfterPostMessage(HOOKCONTROL_POSTMESSAGE * retStatuse, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void StopPostMessageHook();
	bool StartPostMessageHook();
}
