//
//    Copyright (C) Microsoft.  All rights reserved.
//

#ifndef _GPIOHPD_H_
#define _GPIOHPD_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum { GPIOHPD_FILE_DEVICE = FILE_DEVICE_UNKNOWN };

//
// IOCTL codes enumeration
//
enum {
    // General IOCTLs
    GPIOHPD_IOCTL_ID_REGISTER_NOTIFICATION = 0,
    GPIOHPD_IOCTL_ID_LAST
};

#define GPIOHPD_DEVICE_OBJECT_NAME_WSZ L"\\Device\\GpioHpd"

//
// IOCTL_GPIOHPD_REGISTER_NOTIFICATION
//
// Purpose: Register a callback to receive notifications of HDMI plug
//          and unplug events.
//

enum {
    IOCTL_GPIOHPD_REGISTER_NOTIFICATION = ULONG(
            CTL_CODE(
                GPIOHPD_FILE_DEVICE,
                GPIOHPD_IOCTL_ID_REGISTER_NOTIFICATION,
                METHOD_BUFFERED,
                FILE_READ_DATA))
}; // IOCTL_GPIOHPD_REGISTER_NOTIFICATION

//
// This is called by the HPD driver to notify the client of an HDMI hotplug event.
//
typedef
_IRQL_requires_(DISPATCH_LEVEL)
VOID
EVT_GPIOHPD_HOTPLUG_NOTIFICATION (
    _In_ PVOID ContextPtr,
    _In_ BOOLEAN Connected
    );

typedef EVT_GPIOHPD_HOTPLUG_NOTIFICATION *PFN_GPIOHPD_HOTPLUG_NOTIFICATION;

typedef struct _GPIOHPD_REGISTER_NOTIFICATION_INPUT {
    //
    // Pointer to function the HPD driver should call when an HDMI hotplug
    // event occurs.
    //
    PFN_GPIOHPD_HOTPLUG_NOTIFICATION EvtHotplugNotificationFunc;
    
    //
    // Context pointer which the HPD driver supplies in the ContextPtr 
    // parameter of the EVT_GPIOHPD_HOTPLUG_NOTIFICATION callback.
    //
    PVOID ContextPtr;
} GPIOHPD_REGISTER_NOTIFICATION_INPUT, *PGPIOHPD_REGISTER_NOTIFICATION_INPUT;

//
// Provides initial connection state
//
typedef struct _GPIOHPD_REGISTER_NOTIFICATION_OUTPUT {
    BOOLEAN Connected;
} GPIOHPD_REGISTER_NOTIFICATION_OUTPUT, *PGPIOHPD_REGISTER_NOTIFICATION_OUTPUT;


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif //_GPIOHPD_H_
