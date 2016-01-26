/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    device.cpp

Abstract:

    Contains device code for hotplug detection device. This must be its own
    device node so that it can register for the ActiveBoth interrupt resource.

Environment:

    kernel-mode only

Revision History:

--*/

#include "precomp.hpp"

#include "GpioHpdLogging.h"
#include "GpioHpdDevice.hpp"

#include "GpioHpdDevice.tmh"

GPIOHPD_NONPAGED_SEGMENT_BEGIN; //=================================================

_Use_decl_annotations_
BOOLEAN GPIOHPD_DEVICE::HotPlugDetectIsr (
    PKINTERRUPT /*InterruptPtr*/,
    PVOID ServiceContextPtr
    )
{
    auto thisPtr = static_cast<GPIOHPD_DEVICE*>(ServiceContextPtr);
    InterlockedIncrement(&thisPtr->gpioPinPolarity);
    KeInsertQueueDpc(&thisPtr->dpcObject, nullptr, nullptr);

    return TRUE;
}

_Use_decl_annotations_
VOID GPIOHPD_DEVICE::HotPlugDetectDpc (
    PKDPC /*DpcPtr*/,
    PVOID ContextPtr,
    PVOID /*SystemArgument1*/,
    PVOID /*SystemArgument2*/
    )
{
    NT_ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    auto thisPtr = static_cast<GPIOHPD_DEVICE*>(ContextPtr);

    GPIOHPD_REGISTER_NOTIFICATION_INPUT registrationInfo =
        thisPtr->registrationInfo;

    if (!thisPtr->NotificationEnabled()) return;

    NT_ASSERT(registrationInfo.EvtHotplugNotificationFunc);

    BOOLEAN connected = thisPtr->Connected();
    if (connected == thisPtr->lastNotificationValue) {
        GPIOHPD_LOG_TRACE(
            "Skipping notification because connection state did not change. (connected=%d)",
            connected);
        return;
    }
    thisPtr->lastNotificationValue = connected;

    GPIOHPD_LOG_TRACE(
        "Firing notification! (EvtHotplugNotificationFunc=%p, connected=%d)",
        registrationInfo.EvtHotplugNotificationFunc,
        connected);

    registrationInfo.EvtHotplugNotificationFunc(
        registrationInfo.ContextPtr,
        connected);
}

GPIOHPD_NONPAGED_SEGMENT_END; //===================================================
GPIOHPD_PAGED_SEGMENT_BEGIN; //====================================================

_Use_decl_annotations_
GPIOHPD_DEVICE::GPIOHPD_DEVICE () :
    gpioPinPolarity(1),     // initial polarity is always 1
    notificationEnabled(0),
    lastNotificationValue(),
    dpcObject(),
    kinterruptPtr(),
    registrationInfo()
{
    PAGED_CODE();
    GPIOHPD_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    KeInitializeDpc(&this->dpcObject, HotPlugDetectDpc, this);
}

_Use_decl_annotations_
void GPIOHPD_DEVICE::EnableNotification (
    const GPIOHPD_REGISTER_NOTIFICATION_INPUT& RegistrationInfo
    )
{
    PAGED_CODE();
    GPIOHPD_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(RegistrationInfo.EvtHotplugNotificationFunc);
    NT_ASSERT(!this->registrationInfo.EvtHotplugNotificationFunc);

    this->registrationInfo = RegistrationInfo;
    LONG alreadyEnabled = InterlockedOr(&this->notificationEnabled, 1);
    UNREFERENCED_PARAMETER(alreadyEnabled);
    NT_ASSERT(!alreadyEnabled);
}

_Use_decl_annotations_
void GPIOHPD_DEVICE::DisableNotification ()
{
    PAGED_CODE();
    GPIOHPD_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(this->registrationInfo.EvtHotplugNotificationFunc);

    LONG enabled = InterlockedAnd(&this->notificationEnabled, 0);
    UNREFERENCED_PARAMETER(enabled);
    NT_ASSERT(enabled);

    this->registrationInfo = GPIOHPD_REGISTER_NOTIFICATION_INPUT();
}

_Use_decl_annotations_
VOID GPIOHPD_DEVICE::EvtIoInternalDeviceControl (
    WDFQUEUE WdfQueue,
    WDFREQUEST WdfRequest,
    size_t /*OutputBufferLength*/,
    size_t /*InputBufferLength*/,
    ULONG IoControlCode
    )
{
    PAGED_CODE();
    GPIOHPD_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NTSTATUS status;
    GPIOHPD_DEVICE* thisPtr = getGpioHpdDeviceFromWdfObject(
            WdfIoQueueGetDevice(WdfQueue));

    switch (IoControlCode) {
    case IOCTL_GPIOHPD_REGISTER_NOTIFICATION:
    {
        GPIOHPD_REGISTER_NOTIFICATION_INPUT* inputBufferPtr;
        status = WdfRequestRetrieveInputBuffer(
                WdfRequest,
                sizeof(*inputBufferPtr),
                reinterpret_cast<PVOID*>(&inputBufferPtr),
                nullptr);
        if (!NT_SUCCESS(status)) {
            GPIOHPD_LOG_ERROR(
                "Failed to retrieve input buffer. (status=%!STATUS!, WdfRequest=%p)",
                status,
                WdfRequest);
            WdfRequestComplete(WdfRequest, status);
            return;
        }

        if (!inputBufferPtr->EvtHotplugNotificationFunc) {
            GPIOHPD_LOG_ERROR(
                "EvtHotplugNotificationFunc cannot be null. (WdfRequest=%p)",
                WdfRequest);
            WdfRequestComplete(WdfRequest, STATUS_INVALID_PARAMETER);
            return;
        }

        GPIOHPD_REGISTER_NOTIFICATION_OUTPUT* outputBufferPtr;
        status = WdfRequestRetrieveOutputBuffer(
                WdfRequest,
                sizeof(*outputBufferPtr),
                reinterpret_cast<PVOID*>(&outputBufferPtr),
                nullptr);
        if (!NT_SUCCESS(status)) {
            GPIOHPD_LOG_ERROR(
                "Failed to retrieve output buffer. (status=%!STATUS!, WdfRequest=%p)",
                status,
                WdfRequest);
            WdfRequestComplete(WdfRequest, status);
            return;
        }

        if (thisPtr->NotificationEnabled()) {
            GPIOHPD_LOG_ERROR(
                "Notification already registered! Only one client at a time is allowed. (thisPtr->registrationInfo.EvtHotplugNotificationFunc=%p)",
                thisPtr->registrationInfo.EvtHotplugNotificationFunc);
            WdfRequestComplete(WdfRequest, STATUS_SHARING_VIOLATION);
            return;
        }

        GPIOHPD_LOG_INFORMATION(
            "Enabling notification. (EvtHotplugNotificationFunc=%p)",
            inputBufferPtr->EvtHotplugNotificationFunc);

        // Read current connected state. Cannot write into output buffer
        // until we are done with input buffer
        BOOLEAN connected = thisPtr->Connected();
        thisPtr->lastNotificationValue = connected;

        // Enable notification delivery
        thisPtr->EnableNotification(*inputBufferPtr);

        // Fire an initial notification to account for an interrupt that may
        // have occurred between when we read the value and when the ISR
        // was enabled
        KeInsertQueueDpc(&thisPtr->dpcObject, nullptr, nullptr);

        *outputBufferPtr = GPIOHPD_REGISTER_NOTIFICATION_OUTPUT();
        outputBufferPtr->Connected = connected;
        WdfRequestCompleteWithInformation(
            WdfRequest,
            STATUS_SUCCESS,
            sizeof(*outputBufferPtr));

        return;
    } // case IOCTL_GPIOHPD_REGISTER_NOTIFICATION
    } // switch (...IoControlCode)

    GPIOHPD_LOG_WARNING(
        "Unrecognized control code. (IoControlCode=0x%x, WdfRequest=%p)",
        IoControlCode,
        WdfRequest);
    WdfRequestComplete(WdfRequest, STATUS_INVALID_DEVICE_REQUEST);
}

_Use_decl_annotations_
VOID GPIOHPD_DEVICE::EvtFileClose (WDFFILEOBJECT WdfFileObject)
{
    PAGED_CODE();
    GPIOHPD_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    GPIOHPD_DEVICE* thisPtr = getGpioHpdDeviceFromWdfObject(
            WdfFileObjectGetDevice(WdfFileObject));

    GPIOHPD_LOG_TRACE(
        "Disabling notification. (WdfFileObject=%p, EvtHotplugNotificationFunc=%p)",
        WdfFileObject,
        thisPtr->registrationInfo.EvtHotplugNotificationFunc);

    thisPtr->DisableNotification();
}


_Use_decl_annotations_
NTSTATUS GPIOHPD_DEVICE::EvtDeviceD0Entry (
    WDFDEVICE /*WdfDevice*/,
    WDF_POWER_DEVICE_STATE /*PreviousState*/
    )
{
    PAGED_CODE();
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS GPIOHPD_DEVICE::EvtDeviceD0Exit (
    WDFDEVICE /*WdfDevice*/,
    WDF_POWER_DEVICE_STATE /*PreviousState*/
    )
{
    PAGED_CODE();
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS GPIOHPD_DEVICE::EvtDevicePrepareHardware (
    WDFDEVICE WdfDevice,
    WDFCMRESLIST /*ResourcesRaw*/,
    WDFCMRESLIST ResourcesTranslated
    )
{
    PAGED_CODE();

    NTSTATUS status;

    const CM_PARTIAL_RESOURCE_DESCRIPTOR* interruptResourcePtr = nullptr;
    {
        ULONG interruptResourceCount = 0;
        // Look for an interrupt resource and a GPIO IO connection resource
        const ULONG resourceCount = WdfCmResourceListGetCount(ResourcesTranslated);
        for (ULONG i = 0; i < resourceCount; ++i) {
            const CM_PARTIAL_RESOURCE_DESCRIPTOR* resourcePtr =
                WdfCmResourceListGetDescriptor(ResourcesTranslated, i);

            switch (resourcePtr->Type) {
            case CmResourceTypeInterrupt:
                switch (interruptResourceCount) {
                case 0:
                    // first interrupt resource is the one we're looking for
                    interruptResourcePtr = resourcePtr;
                    break;
                default:
                    GPIOHPD_LOG_WARNING(
                        "Received unexpected interrupt resource. (interruptResourceCount=%d, resourcePtr=%p)",
                        interruptResourceCount,
                        resourcePtr);
                }
                ++interruptResourceCount;
                break;
            }
        }

        if (!interruptResourcePtr) {
            GPIOHPD_LOG_ERROR(
                "Did not receive required GPIO IO resource and interrupt resource. (ResourcesTranslated = %p, interruptResourceCount = %d)",
                ResourcesTranslated,
                interruptResourceCount);
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }
    }

    GPIOHPD_DEVICE* thisPtr = getGpioHpdDeviceFromWdfObject(WdfDevice);

    // Connect Hot Plug Detect (HPD) GPIO Interrupt
    {
        auto params = IO_CONNECT_INTERRUPT_PARAMETERS();
        params.Version = CONNECT_FULLY_SPECIFIED;
        params.FullySpecified.PhysicalDeviceObject =
            WdfDeviceWdmGetPhysicalDevice(WdfDevice);
        params.FullySpecified.InterruptObject = &thisPtr->kinterruptPtr;
        params.FullySpecified.ServiceRoutine = HotPlugDetectIsr;
        params.FullySpecified.ServiceContext = thisPtr;
        params.FullySpecified.SpinLock = nullptr;
        params.FullySpecified.SynchronizeIrql = static_cast<KIRQL>(interruptResourcePtr->u.Interrupt.Level);
        params.FullySpecified.FloatingSave = FALSE;
        params.FullySpecified.ShareVector = interruptResourcePtr->ShareDisposition;
        params.FullySpecified.Vector = interruptResourcePtr->u.Interrupt.Vector;
        params.FullySpecified.Irql = static_cast<KIRQL>(interruptResourcePtr->u.Interrupt.Level);

        if (!(interruptResourcePtr->Flags & CM_RESOURCE_INTERRUPT_LATCHED)) {
            GPIOHPD_LOG_ERROR(
                "Active-both GPIO interrupts must be edge sensitive! (interruptResourcePtr=%p)",
                interruptResourcePtr);
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }
        params.FullySpecified.InterruptMode = Latched;

        params.FullySpecified.ProcessorEnableMask = interruptResourcePtr->u.Interrupt.Affinity;
        params.FullySpecified.Group = 0;

        status = IoConnectInterruptEx(&params);
        if (!NT_SUCCESS(status)) {
            GPIOHPD_LOG_ERROR(
                "IoConnectInterruptEx(...) failed to connect HPD interrupt. (status=%!STATUS!)",
                status);
            return status;
        }
    }

    NT_ASSERT(status == STATUS_SUCCESS);
    return status;
}

_Use_decl_annotations_
NTSTATUS GPIOHPD_DEVICE::EvtDeviceReleaseHardware (
    WDFDEVICE WdfDevice,
    WDFCMRESLIST /*ResourcesTranslated*/
    )
{
    PAGED_CODE();

    GPIOHPD_DEVICE* thisPtr = getGpioHpdDeviceFromWdfObject(WdfDevice);

    // disconnect interrupt
    if (thisPtr->kinterruptPtr) {
        auto params = IO_DISCONNECT_INTERRUPT_PARAMETERS();
        params.Version = CONNECT_FULLY_SPECIFIED;
        params.ConnectionContext.InterruptObject = thisPtr->kinterruptPtr;
        IoDisconnectInterruptEx(&params);
        thisPtr->kinterruptPtr = nullptr;
    }

    // Close remote IO target

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS GPIOHPD_DEVICE::EvtDriverDeviceAdd (
    WDFDRIVER /*WdfDriver*/,
    WDFDEVICE_INIT* DeviceInitPtr
    )
{
    PAGED_CODE();

    NTSTATUS status;

    //
    // Setup PNP/Power callbacks.
    //
    {
        WDF_PNPPOWER_EVENT_CALLBACKS pnpCallbacks;
        WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpCallbacks);

        pnpCallbacks.EvtDevicePrepareHardware = EvtDevicePrepareHardware;
        pnpCallbacks.EvtDeviceReleaseHardware = EvtDeviceReleaseHardware;
        pnpCallbacks.EvtDeviceD0Entry = EvtDeviceD0Entry;
        pnpCallbacks.EvtDeviceD0Exit = EvtDeviceD0Exit;

        WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInitPtr, &pnpCallbacks);
    }

    //
    // File object configuration
    //
    {
        WDF_OBJECT_ATTRIBUTES wdfObjectAttributes;
        WDF_OBJECT_ATTRIBUTES_INIT(&wdfObjectAttributes);
        wdfObjectAttributes.SynchronizationScope = WdfSynchronizationScopeNone;
        wdfObjectAttributes.ExecutionLevel = WdfExecutionLevelPassive;

        WDF_FILEOBJECT_CONFIG config;
        WDF_FILEOBJECT_CONFIG_INIT(
            &config,
            nullptr,                // EvtDeviceFileCreate
            EvtFileClose,           // EvtDeviceFileClose
            nullptr);               // EvtDeviceFileCleanup

        WdfDeviceInitSetFileObjectConfig(
            DeviceInitPtr,
            &config,
            &wdfObjectAttributes);
    }

    //
    // Assign a device name
    //
    {
        DECLARE_CONST_UNICODE_STRING(deviceName, GPIOHPD_DEVICE_OBJECT_NAME_WSZ);
        status = WdfDeviceInitAssignName(DeviceInitPtr, &deviceName);
        if (!NT_SUCCESS(status)) {
            GPIOHPD_LOG_ERROR(
                "WdfDeviceInitAssignName(...) failed. (status=%!STATUS!)",
                status);
            return status;
        }
    }

    // Only allow a single handle open at a time
    WdfDeviceInitSetExclusive(DeviceInitPtr, TRUE);

    //
    // Create the device.
    //
    WDFDEVICE wdfDevice;
    GPIOHPD_DEVICE* thisPtr;
    {
        WDF_OBJECT_ATTRIBUTES wdfObjectAttributes;
        WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(
            &wdfObjectAttributes,
            GPIOHPD_DEVICE);

        status = WdfDeviceCreate(
                &DeviceInitPtr,
                &wdfObjectAttributes,
                &wdfDevice);
        if (!NT_SUCCESS(status)) {
            GPIOHPD_LOG_ERROR(
                "Failed to create WDFDEVICE. (DeviceInitPtr = %p, status = %!STATUS!)",
                DeviceInitPtr,
                status);
            return status;
        }

        void *contextMemory = getGpioHpdDeviceFromWdfObject(wdfDevice);
        thisPtr = new (contextMemory) GPIOHPD_DEVICE();
        NT_ASSERT(thisPtr);
    }

    //
    // Create the default queue
    //
    {
        WDF_OBJECT_ATTRIBUTES wdfObjectAttributes;
        WDF_OBJECT_ATTRIBUTES_INIT(&wdfObjectAttributes);
        wdfObjectAttributes.SynchronizationScope = WdfSynchronizationScopeNone;
        wdfObjectAttributes.ExecutionLevel = WdfExecutionLevelPassive;

        WDF_IO_QUEUE_CONFIG config;
        WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
            &config,
            WdfIoQueueDispatchSequential);

        config.EvtIoInternalDeviceControl = EvtIoInternalDeviceControl;

        WDFQUEUE wdfQueue;
        status = WdfIoQueueCreate(
                wdfDevice,
                &config,
                &wdfObjectAttributes,
                &wdfQueue);
        if (!NT_SUCCESS(status)) {
            GPIOHPD_LOG_ERROR(
                "Failed to create default queue. (status=%!STATUS!, wdfDevice=%p)",
                status,
                wdfDevice);
            return status;
        }
    }

    NT_ASSERT(status == STATUS_SUCCESS);
    return status;
}

_Use_decl_annotations_
VOID GPIOHPD_DEVICE::EvtDriverUnload ( WDFDRIVER WdfDriver )
{
    PAGED_CODE();

    DRIVER_OBJECT* driverObjectPtr = WdfDriverWdmGetDriverObject(WdfDriver);
    WPP_CLEANUP(driverObjectPtr);
}

GPIOHPD_PAGED_SEGMENT_END; //======================================================
GPIOHPD_INIT_SEGMENT_BEGIN; //=====================================================

_Use_decl_annotations_
NTSTATUS DriverEntry (
    DRIVER_OBJECT* DriverObjectPtr,
    UNICODE_STRING* RegistryPathPtr
    )
{
    PAGED_CODE();

    //
    // Initialize logging
    //
    {
        WPP_INIT_TRACING(DriverObjectPtr, RegistryPathPtr);
        RECORDER_CONFIGURE_PARAMS recorderConfigureParams;
        RECORDER_CONFIGURE_PARAMS_INIT(&recorderConfigureParams);
        WppRecorderConfigure(&recorderConfigureParams);
#if DBG
        WPP_RECORDER_LEVEL_FILTER(GPIOHPD_TRACING_DEFAULT) = TRUE;
#endif // DBG
    }

    GPIOHPD_LOG_INFORMATION(
        "(DriverObjectPtr = 0x%p, RegistryPathPtr = 0x%p)",
        DriverObjectPtr,
        RegistryPathPtr);

    WDFDRIVER wdfDriver;
    {
        WDF_DRIVER_CONFIG wdfDriverConfig;
        WDF_DRIVER_CONFIG_INIT(&wdfDriverConfig, GPIOHPD_DEVICE::EvtDriverDeviceAdd);
        wdfDriverConfig.EvtDriverUnload = GPIOHPD_DEVICE::EvtDriverUnload;
        wdfDriverConfig.DriverPoolTag = GPIOHPD_POOL_TAG;

        NTSTATUS status = WdfDriverCreate(
                DriverObjectPtr,
                RegistryPathPtr,
                WDF_NO_OBJECT_ATTRIBUTES,
                &wdfDriverConfig,
                &wdfDriver);
        if (!NT_SUCCESS(status)) {
            GPIOHPD_LOG_ASSERTION(
                "WdfDriverCreate failed. (DriverObjectPtr = 0x%p, RegistryPathPtr = 0x%p, status = %!STATUS!)",
                DriverObjectPtr,
                RegistryPathPtr,
                status);
            return status;
        } // if
    } // wdfDriver

    return STATUS_SUCCESS;
} // DriverEntry (...)

GPIOHPD_INIT_SEGMENT_END; //=======================================================