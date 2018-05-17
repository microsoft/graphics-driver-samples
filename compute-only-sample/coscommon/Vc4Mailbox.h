#pragma once

#pragma warning(disable : 4201)

#include <wingdi.h>

#define FILE_DEVICE_RPIQ        2836

//
// RPIQ IOCTL definition
//
enum RPIQFunction
{
    RPIQ_FUNC_MIN = 2000,
    RPIQ_FUNC_MAILBOX_POWER_MANAGEMENT = 2000,
    RPIQ_FUNC_MAILBOX_FRAME_BUFFER = 2001,
    RPIQ_FUNC_MAILBOX_VIRT_UART = 2002,
    RPIQ_FUNC_MAILBOX_VCHIQ = 2003,
    RPIQ_FUNC_MAILBOX_LED = 2004,
    RPIQ_FUNC_MAILBOX_BUTTONS = 2005,
    RPIQ_FUNC_MAILBOX_TOUCH_SCREEN = 2006,
    RPIQ_FUNC_MAILBOX_UNKNOWN = 2007,
    RPIQ_FUNC_MAILBOX_PROPERTY = 2008,
    RPIQ_FUNC_MAILBOX_PROPERTY_VC = 2009,

    RPIQ_FUNC_MAX = 4000
};

#define IOCTL_MAILBOX_PROPERTY \
    CTL_CODE(FILE_DEVICE_RPIQ, RPIQ_FUNC_MAILBOX_PROPERTY, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define TAG_REQUEST 0

//
// Response
//
#define RESPONSE_SUCCESS    0x80000000
#define RESPONSE_ERROR      0x80000001

#pragma pack(push, 1)

typedef struct _MAILBOX_HEADER {
    ULONG TotalBuffer;
    ULONG RequestResponse;
    ULONG TagID;
    ULONG ResponseLength;
    ULONG Request;
} MAILBOX_HEADER, *PMAILBOX_HEADER;

#define TAG_ID_SET_POWER_VC4  0x00030012

typedef struct _MAILBOX_LOCK_MEM {
    MAILBOX_HEADER Header;
    ULONG PowerOn;
    ULONG EndTag;
} MAILBOX_SET_POWER_VC4, *PMAILBOX_SET_POWER_VC4;

__inline VOID INIT_MAILBOX_SET_POWER_VC4(
    _Out_ MAILBOX_SET_POWER_VC4* c,
    _In_ BOOL PowerOn
    )
{
    c->Header.TotalBuffer = sizeof(MAILBOX_SET_POWER_VC4);
    c->Header.RequestResponse = TAG_REQUEST;
    c->Header.TagID = TAG_ID_SET_POWER_VC4;
    c->Header.ResponseLength = 4;
    c->Header.Request = TAG_REQUEST;
    c->PowerOn = PowerOn;
    c->EndTag = 0;
}

#pragma pack(pop)

