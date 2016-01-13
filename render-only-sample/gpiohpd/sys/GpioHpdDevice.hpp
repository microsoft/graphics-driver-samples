/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    GpioHpd.h

Abstract:

    This module contains driver definitions for GPIOHPD

Environment:

    kernel-mode only

Revision History:

--*/

#ifndef _GPIOHPDDEVICE_HPP_
#define _GPIOHPDDEVICE_HPP_

#define GPIOHPD_NONPAGED_SEGMENT_BEGIN \
    __pragma(code_seg(push)) \
    //__pragma(code_seg(.text))

#define GPIOHPD_NONPAGED_SEGMENT_END \
    __pragma(code_seg(pop))

#define GPIOHPD_PAGED_SEGMENT_BEGIN \
    __pragma(code_seg(push)) \
    __pragma(code_seg("PAGE"))

#define GPIOHPD_PAGED_SEGMENT_END \
    __pragma(code_seg(pop))

#define GPIOHPD_INIT_SEGMENT_BEGIN \
    __pragma(code_seg(push)) \
    __pragma(code_seg("INIT"))

#define GPIOHPD_INIT_SEGMENT_END \
    __pragma(code_seg(pop))

#define GPIOHPD_ASSERT_MAX_IRQL(Irql) NT_ASSERT(KeGetCurrentIrql() <= (Irql))
#define GPIOHPD_ASSERT_LOW_IRQL() GPIOHPD_ASSERT_MAX_IRQL(APC_LEVEL)

enum : ULONG { GPIOHPD_POOL_TAG = '_DPH' };

//
// Default memory allocation and object construction for C++ modules
//

__forceinline void* __cdecl operator new (
    size_t Size,
    POOL_TYPE PoolType,
    ULONG Tag
    ) throw ()
{
    if (!Size) Size = 1;
    return ExAllocatePoolWithTag(PoolType, Size, Tag);
} // operator new ( size_t, POOL_TYPE, ULONG )

__forceinline void __cdecl operator delete ( void* Ptr ) throw ()
{
    if (Ptr) ExFreePool(Ptr);
} // operator delete ( void* )

__forceinline void* __cdecl operator new[] (
    size_t Size,
    POOL_TYPE PoolType,
    ULONG Tag
    ) throw ()
{
    if (!Size) Size = 1;
    return ExAllocatePoolWithTag(PoolType, Size, ULONG(Tag));
} // operator new[] ( size_t, POOL_TYPE, ULONG )

__forceinline void __cdecl operator delete[] ( void* Ptr ) throw ()
{
    if (Ptr) ExFreePool(Ptr);
} // operator delete[] ( void* )

__forceinline void* __cdecl operator new ( size_t, void* Ptr ) throw ()
{
    return Ptr;
} // operator new ( size_t, void* )

__forceinline void __cdecl operator delete ( void*, void* ) throw ()
{} // void operator delete ( void*, void* )

__forceinline void* __cdecl operator new[] ( size_t, void* Ptr ) throw ()
{
    return Ptr;
} // operator new[] ( size_t, void* )

__forceinline void __cdecl operator delete[] ( void*, void* ) throw ()
{} // void operator delete[] ( void*, void* )


class GPIOHPD_DEVICE {
public: // NONPAGED

    static KSERVICE_ROUTINE HotPlugDetectIsr;
    static KDEFERRED_ROUTINE HotPlugDetectDpc;

private: // NONPAGED

    GPIOHPD_DEVICE (const GPIOHPD_DEVICE&) = delete;
    GPIOHPD_DEVICE& operator= (const GPIOHPD_DEVICE&) = delete;

    __forceinline bool Connected ( )
    {
        // LOW = connected, HIGH = disconnected
        return !(InterlockedOr(&this->gpioPinPolarity, 0) & 1);
    }

    __forceinline bool NotificationEnabled ( )
    {
        return !!InterlockedOr(&this->notificationEnabled, 0);
    }

    volatile LONG gpioPinPolarity;
    volatile LONG notificationEnabled;
    BOOLEAN lastNotificationValue;
    KDPC dpcObject;
    PKINTERRUPT kinterruptPtr;
    GPIOHPD_REGISTER_NOTIFICATION_INPUT registrationInfo;

public: // PAGED

    _IRQL_requires_max_(PASSIVE_LEVEL)
    GPIOHPD_DEVICE ();

    _IRQL_requires_max_(PASSIVE_LEVEL)
    void EnableNotification (
        const GPIOHPD_REGISTER_NOTIFICATION_INPUT& RegistrationInfo
        );

    _IRQL_requires_max_(PASSIVE_LEVEL)
    void DisableNotification ();

    static EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL EvtIoInternalDeviceControl;
    static EVT_WDF_FILE_CLOSE EvtFileClose;

    static EVT_WDF_DEVICE_D0_ENTRY EvtDeviceD0Entry;
    static EVT_WDF_DEVICE_D0_EXIT EvtDeviceD0Exit;
    static EVT_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
    static EVT_WDF_DEVICE_RELEASE_HARDWARE EvtDeviceReleaseHardware;

    static EVT_WDF_DRIVER_DEVICE_ADD EvtDriverDeviceAdd;
    static EVT_WDF_DRIVER_UNLOAD EvtDriverUnload;

private: // PAGED

};

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(
    GPIOHPD_DEVICE,
    getGpioHpdDeviceFromWdfObject);

extern "C" DRIVER_INITIALIZE DriverEntry;

#endif // _GPIOHPDDEVICE_HPP_
