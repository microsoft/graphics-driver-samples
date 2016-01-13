//
// Copyright (C) Microsoft. All rights reserved.
//

#include "precomp.h"

#include "RosKmdLogging.h"
#include "Vc4Common.tmh"

#include "Vc4Common.h"

VC4_NONPAGED_SEGMENT_BEGIN; //================================================
VC4_NONPAGED_SEGMENT_END; //==================================================
VC4_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
NTSTATUS Vc4OpenDevice (
    UNICODE_STRING* FileNamePtr,
    ACCESS_MASK DesiredAccess,
    ULONG ShareAccess,
    FILE_OBJECT** FileObjectPPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    HANDLE fileHandle;
    OBJECT_ATTRIBUTES attributes;
    InitializeObjectAttributes(
        &attributes,
        FileNamePtr,
        OBJ_KERNEL_HANDLE,
        NULL,       // RootDirectory
        NULL);      // SecurityDescriptor

    IO_STATUS_BLOCK iosb;
    NTSTATUS status = ZwCreateFile(
            &fileHandle,
            DesiredAccess,
            &attributes,
            &iosb,
            nullptr,                    // AllocationSize
            FILE_ATTRIBUTE_NORMAL,
            ShareAccess,
            FILE_OPEN,
            FILE_NON_DIRECTORY_FILE,    // CreateOptions
            nullptr,                    // EaBuffer
            0);                         // EaLength
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "ZwCreateFile failed. (status=%!STATUS!, FileNamePtr=%wZ, DesiredAccess=0x%x, ShareAccess=0x%x)",
            status,
            FileNamePtr,
            DesiredAccess,
            ShareAccess);
        return status;
    } // if
    auto closeHandle = VC4_FINALLY::Do([&] {
        PAGED_CODE();
        NTSTATUS closeStatus = ZwClose(fileHandle);
        UNREFERENCED_PARAMETER(closeStatus);
        NT_ASSERT(NT_SUCCESS(closeStatus));
    });

    status = ObReferenceObjectByHandleWithTag(
            fileHandle,
            DesiredAccess,
            *IoFileObjectType,
            KernelMode,
            VC4_ALLOC_TAG::DEVICE,
            reinterpret_cast<PVOID*>(FileObjectPPtr),
            nullptr);   // HandleInformation
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "ObReferenceObjectByHandleWithTag(...) failed. (status=%!STATUS!, fileHandle=%p)",
            status,
            fileHandle);
        return status;
    } // if
    
    NT_ASSERT(*FileObjectPPtr);
    NT_ASSERT(status == STATUS_SUCCESS);
    return status;
}

_Use_decl_annotations_
NTSTATUS Vc4SendWriteSynchronously (
    FILE_OBJECT* FileObjectPtr,
    void* InputBufferPtr,
    ULONG InputBufferLength,
    ULONG* InformationPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);
    
    KEVENT event;
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    DEVICE_OBJECT* deviceObjectPtr = IoGetRelatedDeviceObject(FileObjectPtr);
    auto iosb = IO_STATUS_BLOCK();
    IRP* irpPtr = IoBuildSynchronousFsdRequest(
            IRP_MJ_WRITE,
            deviceObjectPtr,
            InputBufferPtr,
            InputBufferLength,
            nullptr,            // StartingOffset
            &event,
            &iosb);
    if (!irpPtr) {
        ROS_LOG_LOW_MEMORY(
            "IoBuildSynchronousFsdRequest(...) failed. (deviceObjectPtr=%p, FileObjectPtr=%p, InputBufferPtr=%p, InputBufferLength=%d)",
            deviceObjectPtr,
            FileObjectPtr,
            InputBufferPtr,
            InputBufferLength);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IO_STACK_LOCATION* irpSp = IoGetNextIrpStackLocation(irpPtr);
    irpSp->FileObject = FileObjectPtr;

    iosb.Status = STATUS_NOT_SUPPORTED;
    NTSTATUS status = IoCallDriver(deviceObjectPtr, irpPtr);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,          // Alertable
            nullptr);       // Timeout

        status = iosb.Status;
    }
    
    *InformationPtr = iosb.Information;
    return status;
}

_Use_decl_annotations_
NTSTATUS Vc4SendIoctlSynchronously (
    FILE_OBJECT* FileObjectPtr,
    ULONG IoControlCode,
    void* InputBufferPtr,
    ULONG InputBufferLength,
    void* OutputBufferPtr,
    ULONG OutputBufferLength,
    BOOLEAN InternalDeviceIoControl,
    ULONG* InformationPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);
    
    KEVENT event;
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    DEVICE_OBJECT* deviceObjectPtr = IoGetRelatedDeviceObject(FileObjectPtr);
    auto iosb = IO_STATUS_BLOCK();
    IRP* irpPtr = IoBuildDeviceIoControlRequest(
            IoControlCode,
            deviceObjectPtr,
            InputBufferPtr,
            InputBufferLength,
            OutputBufferPtr,                    // OutputBuffer
            OutputBufferLength,                 // OutputBufferLength
            InternalDeviceIoControl,
            &event,
            &iosb);
    if (!irpPtr) {
        ROS_LOG_LOW_MEMORY(
            "IoBuildDeviceIoControlRequest(...) failed. (IoControlCode=0x%x, deviceObjectPtr=%p, FileObjectPtr=%p)",
            IoControlCode,
            deviceObjectPtr,
            FileObjectPtr);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IO_STACK_LOCATION* irpSp = IoGetNextIrpStackLocation(irpPtr);
    irpSp->FileObject = FileObjectPtr;

    iosb.Status = STATUS_NOT_SUPPORTED;
    NTSTATUS status = IoCallDriver(deviceObjectPtr, irpPtr);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,          // Alertable
            nullptr);       // Timeout

        status = iosb.Status;
    }
    
    *InformationPtr = iosb.Information;
    return status;
}

VC4_PAGED_SEGMENT_END; //=====================================================
