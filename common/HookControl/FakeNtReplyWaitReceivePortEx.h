#pragma once
#include "HookHelp.h"
#include "InlineHook.h"

typedef struct _PORT_MESSAGE
{
	USHORT DataSize;//数据长度
	USHORT MessageSize;//总长度
	USHORT MessageType;
	USHORT DataInfoOffset;
	CLIENT_ID ClientId;
	ULONG MessageId;
	ULONG SectionSize;
	//UCHAR Data[];
}PORT_MESSAGE, *PPORT_MESSAGE;

namespace HookControl{

	typedef struct _HOOKCONTROL_NTREPLYWAITRECEIVEPORTEX
	{
		void * CallAddress;
		NTSTATUS RetValue;
	}HOOKCONTROL_NTREPLYWAITRECEIVEPORTEX, *PHOOKCONTROL_NTREPLYWAITRECEIVEPORTEX;

	typedef NTSTATUS(WINAPI *__pfnNtReplyWaitReceivePortEx)(HANDLE, PVOID*, PPORT_MESSAGE, PPORT_MESSAGE, PLARGE_INTEGER);

	extern __pfnNtReplyWaitReceivePortEx pfnNtReplyWaitReceivePortEx;

	bool OnBeforeNtReplyWaitReceivePortEx(HOOKCONTROL_NTREPLYWAITRECEIVEPORTEX * retStatuse, IN HANDLE PortHandle, OUT PVOID* PortIdentifier OPTIONAL, IN PPORT_MESSAGE ReplyMessage OPTIONAL, OUT PPORT_MESSAGE Message, IN PLARGE_INTEGER Timeout);
	bool OnAfterNtReplyWaitReceivePortEx(HOOKCONTROL_NTREPLYWAITRECEIVEPORTEX * retStatuse, IN HANDLE PortHandle, OUT PVOID* PortIdentifier OPTIONAL, IN PPORT_MESSAGE ReplyMessage OPTIONAL, OUT PPORT_MESSAGE Message, IN PLARGE_INTEGER Timeout);

	void StopNtReplyWaitReceivePortExHook();
	bool StartNtReplyWaitReceivePortExHook();
}
