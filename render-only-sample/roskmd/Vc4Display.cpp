//
// Copyright (C) Microsoft. All rights reserved.
//

#include "precomp.h"

#include "RosKmdLogging.h"
#include "Vc4Display.tmh"

#include "Vc4Common.h"
#include "Vc4Debug.h"
#include "Vc4Display.h"

VC4_NONPAGED_SEGMENT_BEGIN; //================================================

void BltBits (
    const void *SourceBitsPtr,
    void *DestBitsPtr,
    ULONG Pitch,
    _In_reads_(RectCount) const RECT* RectsPtr,
    ULONG RectCount
    )
{
    for (UINT i = 0; i < RectCount; ++i) {
        const RECT* rectPtr = &RectsPtr[i];

        NT_ASSERT(rectPtr->right >= rectPtr->left);
        NT_ASSERT(rectPtr->bottom >= rectPtr->top);

        const UINT numPixels = rectPtr->right - rectPtr->left;
        const UINT numRows = rectPtr->bottom - rectPtr->top;
        const UINT bytesToCopy = numPixels * 4;
        BYTE* dstStartPtr = ((BYTE*)DestBitsPtr +
                          rectPtr->top * Pitch +
                          rectPtr->left * 4);
        const BYTE* srcStartPtr = ((BYTE*)SourceBitsPtr +
                                rectPtr->top * Pitch +
                                rectPtr->left * 4);

        for (UINT row = 0; row < numRows; ++row) {
            RtlCopyMemory(dstStartPtr, srcStartPtr, bytesToCopy);
            dstStartPtr += Pitch;
            srcStartPtr += Pitch;
        }
    }
}

_Use_decl_annotations_
void VC4_DISPLAY::ResetDevice ()
{
    // TODO: Set the display adapter to VGA character mode (80 x 50)
    ROS_LOG_ERROR("Vc4DdiResetDevice() was called but is not implemented.");
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::SystemDisplayEnable (
    D3DDDI_VIDEO_PRESENT_TARGET_ID /*TargetId*/,
    DXGKARG_SYSTEM_DISPLAY_ENABLE_FLAGS* /*FlagsPtr*/,
    UINT* /*WidthPtr*/,
    UINT* /*HeightPtr*/,
    D3DDDIFORMAT* /*ColorFormatPtr*/
    )
{
    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
void VC4_DISPLAY::SystemDisplayWrite (
    VOID* /*SourcePtr*/,
    UINT /*SourceWidth*/,
    UINT /*SourceHeight*/,
    UINT /*SourceStride*/,
    UINT /*PositionX*/,
    UINT /*PositionY*/
    )
{
    ROS_LOG_ASSERTION("Not implemented");
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::SetVidPnSourceAddress (
    const DXGKARG_SETVIDPNSOURCEADDRESS* /*SetVidPnSourceAddressPtr*/
    )
{
    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
BOOLEAN VC4_DISPLAY::InterruptRoutine (
    ULONG /*MessageNumber*/
    )
{
    // Get PixelValve status register
    VC4PIXELVALVE_INTERRUPT intStat =
        {READ_REGISTER_NOFENCE_ULONG(&this->pvRegistersPtr->IntStat)};

    enum : ULONG { VC4PIXELVALVE_INTERRUPT_MASK = 0x3ff };
    if (!(intStat.AsUlong & VC4PIXELVALVE_INTERRUPT_MASK)) {
        return FALSE;
    }

    // VSync interrupt
    bool notifyPresentComplete = false;
    if (intStat.VfpStart) {
        FRAME_BUFFER_ID pendingFrameBufferId = InterlockedAnd(
                &this->pendingActiveFrameBufferId,
                0);
        if (pendingFrameBufferId) {
            FRAME_BUFFER_ID oldActiveFrameBufferId =
                this->activeFrameBufferId;
            this->activeFrameBufferId = pendingFrameBufferId;

            // Return the old active frame buffer to the free list
            FRAME_BUFFER_ID localBackBufferId = InterlockedCompareExchange(
                    &this->backBufferId,
                    oldActiveFrameBufferId,
                    0);

            if (localBackBufferId) {
                // The back buffer slot should be available!
                ROS_LOG_ASSERTION(
                    "The free buffer list is already full! (localBackBufferId=%d, oldActiveFrameBufferId=%d, pendingFrameBufferId=%d",
                    localBackBufferId,
                    oldActiveFrameBufferId,
                    pendingFrameBufferId);
            }

            ROS_LOG_TRACE(
                "Reporting display progress as complete. (oldActiveFrameBufferId=%d, pendingFrameBufferId=%d)",
                oldActiveFrameBufferId,
                pendingFrameBufferId);

            // Disable the VfpStart interrupt. This ensures that the vsync
            // interrupt only gets fired when a new frame is displayed
            this->pixelValveIntEn.VfpStart = FALSE;

            notifyPresentComplete = true;
        } else {
            ROS_LOG_ASSERTION(
                "Received VfpStart interrupt without a new frame buffer! (this->activeFrameBufferId=%d, this->pendingFrameBufferId=%d, this->backBufferId=%d)",
                this->activeFrameBufferId,
                this->pendingActiveFrameBufferId,
                this->backBufferId);
        }
    }

    // Write new interrupt enable/disable mask
    WRITE_REGISTER_NOFENCE_ULONG(
        &this->pvRegistersPtr->IntEn,
        this->pixelValveIntEn.AsUlong);

    // Acknowledge interrupts
    WRITE_REGISTER_NOFENCE_ULONG(
        &this->pvRegistersPtr->IntStat,
        intStat.AsUlong);

    if (notifyPresentComplete) {
        // Notify framework that previous active buffer is now safe
        // to use again
        DXGKARGCB_NOTIFY_INTERRUPT_DATA args = {};
        args.InterruptType = DXGK_INTERRUPT_DISPLAYONLY_PRESENT_PROGRESS;
        args.DisplayOnlyPresentProgress.VidPnSourceId = 0;
        args.DisplayOnlyPresentProgress.ProgressId =
            DXGK_PRESENT_DISPLAYONLY_PROGRESS_ID_COMPLETE;

        this->dxgkInterface.DxgkCbNotifyInterrupt(
            this->dxgkInterface.DeviceHandle,
            &args);
    }

    this->dxgkInterface.DxgkCbQueueDpc(this->dxgkInterface.DeviceHandle);

    return TRUE;
}

_Use_decl_annotations_
VOID VC4_DISPLAY::EvtHotplugNotification (PVOID ContextPtr, BOOLEAN Connected)
{
    NT_ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    ROS_LOG_INFORMATION(
        "Received hotplug notification. (Connected=%d)",
        Connected);

    auto thisPtr = static_cast<VC4_DISPLAY*>(ContextPtr);

    thisPtr->hdmiConnected = Connected;

    auto childStatus = DXGK_CHILD_STATUS();
    childStatus.Type = StatusConnection;
    childStatus.ChildUid = 0;
    childStatus.HotPlug.Connected = Connected;

    NTSTATUS status = thisPtr->dxgkInterface.DxgkCbIndicateChildStatus(
            thisPtr->dxgkInterface.DeviceHandle,
            &childStatus);

    if (!NT_SUCCESS(status)) {
        ROS_LOG_ASSERTION(
            "DxgkCbIndicateChildStatus(...) failed! (status=%!STATUS!, Connected=%d)",
            status,
            Connected);
    }
}

VC4_NONPAGED_SEGMENT_END; //==================================================
VC4_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
VC4_DISPLAY::VC4_DISPLAY (
    const DEVICE_OBJECT* PhysicalDeviceObjectPtr,
    const DXGKRNL_INTERFACE& DxgkInterface,
    const DXGK_START_INFO& DxgkStartInfo,
    const DXGK_DEVICE_INFO& DxgkDeviceInfo
    ) :
    physicalDeviceObjectPtr(PhysicalDeviceObjectPtr),
    dxgkInterface(DxgkInterface),
    dxgkStartInfo(DxgkStartInfo),
    dxgkDeviceInfo(DxgkDeviceInfo),

    dbgHelper(this->dxgkInterface),
    dxgkDisplayInfo(),
    dxgkVideoSignalInfo(),
    dxgkCurrentSourceMode(),

    hpdFileObjectPtr(),
    hdmiConnected(),
    i2cFileObjectPtrs(),

    hvsRegistersPtr(),
    pvRegistersPtr(),
    pixelValveIntEn(),

    frameBufferLength(0),
    biosFrameBufferPtr(),
    systemFrameBuffers(),

    activeFrameBufferId(),
    pendingActiveFrameBufferId(),
    backBufferId(),

    displayListPtr(),
    displayListControlWord0()
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_TRACE(
        L"Successfully constructed display subsystem. (this=0x%p, PhysicalDeviceObjectPtr=0x%p)",
        this,
        PhysicalDeviceObjectPtr);
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::StartDevice (
    ULONG FirstResourceIndex,
    ULONG* NumberOfVideoPresentSourcesPtr,
    ULONG* NumberOfChildrenPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NTSTATUS status;

    // Precondition: the caller is responsible to set up common device information
    NT_ASSERT(this->dxgkInterface.DeviceHandle);
    NT_ASSERT(this->dxgkDeviceInfo.PhysicalDeviceObject == this->physicalDeviceObjectPtr);

    // Open handle to HPD driver and register notification
    FILE_OBJECT* localHpdFileObjectPtr;
    {
        DECLARE_CONST_UNICODE_STRING(hpdDeviceName, GPIOHPD_DEVICE_OBJECT_NAME_WSZ);
        status = Vc4OpenDevice(
                const_cast<UNICODE_STRING*>(&hpdDeviceName),
                GENERIC_READ | GENERIC_WRITE,
                0,
                &localHpdFileObjectPtr);

        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "Failed to open handle to hotplug detection device.. (status=%!STATUS!, hpdDeviceName=%wZ)",
                status,
                &hpdDeviceName);
            return status;
        }
    }
    auto dereferenceHpdFileObject = VC4_FINALLY::DoUnless([&] {
        PAGED_CODE();
        ObDereferenceObjectWithTag(localHpdFileObjectPtr, VC4_ALLOC_TAG::DEVICE);
    });

    // Register for hotplug notification
    {
        auto inputBuffer = GPIOHPD_REGISTER_NOTIFICATION_INPUT();
        inputBuffer.EvtHotplugNotificationFunc = EvtHotplugNotification;
        inputBuffer.ContextPtr = this;
        GPIOHPD_REGISTER_NOTIFICATION_OUTPUT outputBuffer;
        ULONG information;
        status = Vc4SendIoctlSynchronously(
                localHpdFileObjectPtr,
                IOCTL_GPIOHPD_REGISTER_NOTIFICATION,
                &inputBuffer,
                sizeof(inputBuffer),
                &outputBuffer,
                sizeof(outputBuffer),
                TRUE,                       // InternalDeviceIoControl
                &information);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "Vc4SendIoctlSynchronously(...IOCTL_GPIOHPD_REGISTER_NOTIFICATION...) failed. (status=%!STATUS!, localHpdFileObjectPtr=%p)",
                status,
                localHpdFileObjectPtr);
            return status;
        }
        this->hdmiConnected = outputBuffer.Connected;
    }

    // Find and validate hardware resources
    const CM_PARTIAL_RESOURCE_DESCRIPTOR* hvsMemoryResourcePtr = nullptr;
    const CM_PARTIAL_RESOURCE_DESCRIPTOR* pvMemoryResourcePtr = nullptr;
    const CM_PARTIAL_RESOURCE_DESCRIPTOR* pvInterruptResourcePtr = nullptr;
    const CM_PARTIAL_RESOURCE_DESCRIPTOR*
        i2cResourcePtrs[ARRAYSIZE(this->i2cFileObjectPtrs)] = {0};
    {
        const CM_RESOURCE_LIST* resourceListPtr =
            this->dxgkDeviceInfo.TranslatedResourceList;
        NT_ASSERT(resourceListPtr->Count == 1);
        const CM_PARTIAL_RESOURCE_LIST* partialResourceListPtr =
            &resourceListPtr->List[0].PartialResourceList;

        ULONG memResourceCount = 0;
        ULONG interruptResourceCount = 0;
        ULONG i2cResourceCount = 0;
        for (ULONG i = FirstResourceIndex; i < partialResourceListPtr->Count; ++i) {
            const CM_PARTIAL_RESOURCE_DESCRIPTOR* resourcePtr =
                &partialResourceListPtr->PartialDescriptors[i];
            switch (resourcePtr->Type) {
            case CmResourceTypeMemory:
                switch (memResourceCount) {
                case 0:
                    // first memory resource is HVS block
                    hvsMemoryResourcePtr = resourcePtr;
                    break;
                case 1:
                    // second memory resource is PixelValve block
                    pvMemoryResourcePtr = resourcePtr;
                    break;
                default:
                    ROS_LOG_WARNING(
                        "Received unexpected memory resource. (memResourceCount=%d, resourcePtr=%p, FirstResourceIndex=%d)",
                        memResourceCount,
                        resourcePtr,
                        FirstResourceIndex);
                }
                ++memResourceCount;
                break;
            case CmResourceTypeInterrupt:
                switch (interruptResourceCount) {
                case 0:
                    // first interrupt resource is PixelValve interrupt
                    pvInterruptResourcePtr = resourcePtr;
                    break;
                default:
                    ROS_LOG_WARNING(
                        "Received unexpected interrupt resource. (interruptResourceCount=%d, resourcePtr=%p, FirstResourceIndex=%d)",
                        interruptResourceCount,
                        resourcePtr,
                        FirstResourceIndex);
                }
                ++interruptResourceCount;
                break;
            case CmResourceTypeConnection:
                switch (resourcePtr->u.Connection.Class) {
                case CM_RESOURCE_CONNECTION_CLASS_SERIAL:
                    switch (resourcePtr->u.Connection.Type) {
                    case CM_RESOURCE_CONNECTION_TYPE_SERIAL_I2C:
                        if (i2cResourceCount < ARRAYSIZE(i2cResourcePtrs)) {
                            i2cResourcePtrs[i2cResourceCount] = resourcePtr;
                        } else {
                            ROS_LOG_WARNING(
                                "Received unexpected I2C resource. (i2cResourceCount=%d, resourcePtr=%p, FirstResourceIndex=%d)",
                                i2cResourceCount,
                                resourcePtr,
                                FirstResourceIndex);
                        }
                        ++i2cResourceCount;
                    }
                }
                break;
            } // switch
        }

        if (!hvsMemoryResourcePtr ||
            !pvMemoryResourcePtr ||
            !pvInterruptResourcePtr ||
            (i2cResourceCount < ARRAYSIZE(i2cResourcePtrs))) {

            ROS_LOG_ERROR(
                "Did not find required resources. (TranslatedResourceList=%p, FirstResourceIndex=%d, hvsMemoryResourcePtr=%p, pvMemoryResourcePtr=%p, pvInterruptResourcePtr=%p, i2cResourceCount=%d)",
                this->dxgkDeviceInfo.TranslatedResourceList,
                FirstResourceIndex,
                hvsMemoryResourcePtr,
                pvMemoryResourcePtr,
                pvInterruptResourcePtr,
                i2cResourceCount);
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }

        if (hvsMemoryResourcePtr->u.Memory.Length < sizeof(VC4HVS_REGISTERS)) {
            ROS_LOG_ERROR(
                "Memory region is too small to fit VC4HVS_REGISTERS. (hvsMemoryResourcePtr=%p)",
                hvsMemoryResourcePtr);
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }

        if (pvMemoryResourcePtr->u.Memory.Length < sizeof(VC4PIXELVALVE_REGISTERS)) {
            ROS_LOG_ERROR(
                "Memory region is too small to fit VC4PIXELVALVE_REGISTERS. (pvMemoryResourcePtr=%p)",
                pvMemoryResourcePtr);
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }
    }

    // Map the HVS registers
    NT_ASSERT(
        (hvsMemoryResourcePtr->Type == CmResourceTypeMemory) &&
        (hvsMemoryResourcePtr->u.Memory.Length >= sizeof(VC4HVS_REGISTERS)));

    VC4HVS_REGISTERS* _hvsRegistersPtr;
    status = this->dxgkInterface.DxgkCbMapMemory(
            this->dxgkInterface.DeviceHandle,
            hvsMemoryResourcePtr->u.Memory.Start,
            sizeof(*_hvsRegistersPtr),
            FALSE,
            FALSE,
            MmNonCached,
            reinterpret_cast<PVOID*>(&_hvsRegistersPtr));
    if (!NT_SUCCESS(status)) {
        ROS_LOG_LOW_MEMORY(
            "Failed to map VC4 HVS registers into system address space. (status=%!STATUS!, hvsMemoryResourcePtr->u.Memory.Start=0x%I64x, length=%d)",
            status,
            hvsMemoryResourcePtr->u.Memory.Start.QuadPart,
            sizeof(*_hvsRegistersPtr));
        return status;
    }
    auto unmapHvsRegisters = VC4_FINALLY::DoUnless([&] {
        PAGED_CODE();
        NTSTATUS unmapStatus = this->dxgkInterface.DxgkCbUnmapMemory(
                this->dxgkInterface.DeviceHandle,
                _hvsRegistersPtr);
        UNREFERENCED_PARAMETER(unmapStatus);
        NT_ASSERT(NT_SUCCESS(unmapStatus));
    });

    // Map the PixelValve registers
    NT_ASSERT(
        (pvMemoryResourcePtr->Type == CmResourceTypeMemory) &&
        (pvMemoryResourcePtr->u.Memory.Length >= sizeof(VC4PIXELVALVE_REGISTERS)));

    VC4PIXELVALVE_REGISTERS* _pvRegistersPtr;
    status = this->dxgkInterface.DxgkCbMapMemory(
            this->dxgkInterface.DeviceHandle,
            pvMemoryResourcePtr->u.Memory.Start,
            sizeof(*_pvRegistersPtr),
            FALSE,
            FALSE,
            MmNonCached,
            reinterpret_cast<PVOID*>(&_pvRegistersPtr));
    if (!NT_SUCCESS(status)) {
        ROS_LOG_LOW_MEMORY(
            "Failed to map PixelValve registers into system address space. (status=%!STATUS!, pvMemoryResourcePtr->u.Memory.Start=0x%I64x, length=%d)",
            status,
            pvMemoryResourcePtr->u.Memory.Start.QuadPart,
            sizeof(*_pvRegistersPtr));
        return status;
    }
    auto unmapPvRegisters = VC4_FINALLY::DoUnless([&] {
        PAGED_CODE();
        NTSTATUS unmapStatus = this->dxgkInterface.DxgkCbUnmapMemory(
                this->dxgkInterface.DeviceHandle,
                _pvRegistersPtr);
        UNREFERENCED_PARAMETER(unmapStatus);
        NT_ASSERT(NT_SUCCESS(unmapStatus));
    });

    // Open I2C connections
    auto dereferenceI2cFileObjects = VC4_FINALLY::DoUnless([&] {
        PAGED_CODE();
        for (auto& fileObjectPtr : this->i2cFileObjectPtrs) {
            if (!fileObjectPtr) break;
            ObDereferenceObjectWithTag(fileObjectPtr, VC4_ALLOC_TAG::DEVICE);
            fileObjectPtr = nullptr;
        }
    });
    for (ULONG i = 0; i < ARRAYSIZE(i2cResourcePtrs); ++i) {
        static_assert(
            ARRAYSIZE(i2cResourcePtrs) == ARRAYSIZE(this->i2cFileObjectPtrs),
            "Sanity check on size of I2C channel array");

        DECLARE_UNICODE_STRING_SIZE(deviceName, RESOURCE_HUB_PATH_CHARS);

        status = RESOURCE_HUB_CREATE_PATH_FROM_ID(
                &deviceName,
                i2cResourcePtrs[i]->u.Connection.IdLowPart,
                i2cResourcePtrs[i]->u.Connection.IdHighPart);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "RESOURCE_HUB_CREATE_PATH_FROM_ID() failed. (status = %!STATUS!)",
                status);
            return Vc4SanitizeNtstatus(status);
        } // if

        NT_ASSERT(!this->i2cFileObjectPtrs[i]);
        status = Vc4OpenDevice(
                &deviceName,
                GENERIC_READ | GENERIC_WRITE,
                0,                                  // ShareAccess
                &(this->i2cFileObjectPtrs[i]));
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "Failed to open I2C connection. (status=%!STATUS!, deviceName=%wZ)",
                status,
                &deviceName);
            return status;
        }
    }

    // Ensure HVS register block is enabled
    {
        VC4HVS_DISPCTRL dispCtrl =
            {READ_REGISTER_NOFENCE_ULONG(&_hvsRegistersPtr->DISPCTRL)};

        if (!dispCtrl.ENABLE) {
            ROS_LOG_ERROR("The HVS is not enabled");
            return STATUS_DEVICE_HARDWARE_ERROR;
        }

#ifdef DBG
        // Dump the HVS registers for debugging purposes before making any
        // modifications
        this->dbgHelper.DumpHvsRegisters(_hvsRegistersPtr);
#endif
    }

    // Validate the DISPID register
    {
        const ULONG dispId = READ_REGISTER_NOFENCE_ULONG(
                &_hvsRegistersPtr->DISPID);
        if (dispId != VC4HVS_DISPID) {
            ROS_LOG_ASSERTION(
                "DISPID register did not match expected value. (dispId=0x%x, VC4HVS_DISPID=0x%x)",
                dispId,
                VC4HVS_DISPID);
            return STATUS_DEVICE_HARDWARE_ERROR;
        }
    }

    // Validate the HVS version
    {
        VC4HVS_DISPLSTAT displstat =
            {READ_REGISTER_NOFENCE_ULONG(&_hvsRegistersPtr->DISPLSTAT)};

        if (displstat.VERSION != VC4HVS_VERSION_VC4) {
            ROS_LOG_ERROR(
                "The HVS version is not VC4. (displstat.VERSION=0x%x, VC4HVS_VERSION_VC4=0x%x, displstat.AsUlong=0x%x)",
                displstat.VERSION,
                VC4HVS_VERSION_VC4,
                displstat.AsUlong);
            return STATUS_NOT_SUPPORTED;
        }
    }

    // Ensure PixelValve register block is enabled
    {
        VC4PIXELVALVE_CONTROL pvControlRegister =
            {READ_REGISTER_NOFENCE_ULONG(&_pvRegistersPtr->Control)};

        if (!pvControlRegister.Enable) {
            ROS_LOG_ERROR(
                "The PixelValve peripheral is not enabled. (pvControlRegister=0x%x, _pvRegistersPtr=%p)",
                pvControlRegister.AsUlong,
                _pvRegistersPtr);
            return STATUS_DEVICE_HARDWARE_ERROR;
        }

#ifdef DBG
        this->dbgHelper.DumpPixelValveRegisters(_pvRegistersPtr);
#endif

        // Ensure interrupts are disabled
        this->pixelValveIntEn.AsUlong = 0;
        WRITE_REGISTER_NOFENCE_ULONG(&_pvRegistersPtr->IntEn, 0);
    }

    status = this->dxgkInterface.DxgkCbAcquirePostDisplayOwnership(
            this->dxgkInterface.DeviceHandle,
            &this->dxgkDisplayInfo);
    if (!NT_SUCCESS(status) || (this->dxgkDisplayInfo.Width == 0)) {
        ROS_LOG_ERROR(
            "DxgkCbAcquirePostDisplayOwnership() failed. (status = %!STATUS!, this->dxgkDisplayInfo.Width = %d, this->dxgkInterface.DeviceHandle = %p)",
            status,
            this->dxgkDisplayInfo.Width,
            this->dxgkInterface.DeviceHandle);
        return STATUS_UNSUCCESSFUL;
    }

    // set up the video signal info which we'll report in RecommendMonitorModes
    // and EnumCofuncModality
    this->dxgkVideoSignalInfo.VideoStandard = D3DKMDT_VSS_OTHER;
    this->dxgkVideoSignalInfo.TotalSize.cx = this->dxgkDisplayInfo.Width;
    this->dxgkVideoSignalInfo.TotalSize.cy = this->dxgkDisplayInfo.Height;
    this->dxgkVideoSignalInfo.ActiveSize = this->dxgkVideoSignalInfo.TotalSize;
    this->dxgkVideoSignalInfo.VSyncFreq.Numerator = D3DKMDT_FREQUENCY_NOTSPECIFIED;
    this->dxgkVideoSignalInfo.VSyncFreq.Denominator = D3DKMDT_FREQUENCY_NOTSPECIFIED;
    this->dxgkVideoSignalInfo.HSyncFreq.Numerator = D3DKMDT_FREQUENCY_NOTSPECIFIED;
    this->dxgkVideoSignalInfo.HSyncFreq.Denominator = D3DKMDT_FREQUENCY_NOTSPECIFIED;
    this->dxgkVideoSignalInfo.PixelRate = D3DKMDT_FREQUENCY_NOTSPECIFIED;
    this->dxgkVideoSignalInfo.ScanLineOrdering = D3DDDI_VSSLO_PROGRESSIVE;

    ROS_LOG_TRACE(
        "Successfully acquired post display ownership. (Width = %d, Height = %d, Pitch = %d, ColorFormat = %d, TargetId = %d, AcpiId = %d)",
        this->dxgkDisplayInfo.Width,
        this->dxgkDisplayInfo.Height,
        this->dxgkDisplayInfo.Pitch,
        this->dxgkDisplayInfo.ColorFormat,
        this->dxgkDisplayInfo.TargetId,
        this->dxgkDisplayInfo.AcpiId);

    // map frame buffer from physical address
    ULONG _frameBufferLength = this->dxgkDisplayInfo.Pitch *
        this->dxgkDisplayInfo.Height;
    void* _biosFrameBufferPtr = MmMapIoSpaceEx(
            this->dxgkDisplayInfo.PhysicAddress,
            _frameBufferLength,
            PAGE_READWRITE | PAGE_NOCACHE);
    if (!_biosFrameBufferPtr) {
        ROS_LOG_LOW_MEMORY(
            "Failed to map frame buffer into system address space. (this->dxgkDisplayInfo.PhysicAddress = 0x%I64x, _frameBufferLength = %d)",
            this->dxgkDisplayInfo.PhysicAddress.QuadPart,
            _frameBufferLength);
        return STATUS_NO_MEMORY;
    }
    auto unmapBiosFrameBuffer = VC4_FINALLY::DoUnless([&] {
        PAGED_CODE();
        MmUnmapIoSpace(_biosFrameBufferPtr, _frameBufferLength);
    });

    // allocate two more buffers so that we can do triple buffering
    void* systemFrameBuffer1 = MmAllocateContiguousMemory(
            _frameBufferLength,
            PHYSICAL_ADDRESS{ULONG(-1)});
    if (!systemFrameBuffer1) {
        ROS_LOG_LOW_MEMORY("Failed to allocate contiguous memory for frame buffer");
        return STATUS_NO_MEMORY;
    }
    auto freeSystemFrameBuffer1 = VC4_FINALLY::DoUnless([&] {
        PAGED_CODE();
        MmFreeContiguousMemory(systemFrameBuffer1);
    });

    // XXX: make frame buffer all red for debugging purposes
    for (ULONG i = 0; i < (_frameBufferLength / sizeof(ULONG)); ++i) {
        reinterpret_cast<ULONG*>(systemFrameBuffer1)[i] = 0xffff0000; // red ?
    }

    // Validate and store a pointer to the active display list
    {
        VC4HVS_DISPFIFOCTRL dispCtrl1 = {
            READ_REGISTER_NOFENCE_ULONG(&_hvsRegistersPtr->DISPCTRL1)};

        // expect DISPCTRL1.ENB to be TRUE
        // expect FIFOREG, FIFO32, ONECTX, ONESHOT, and RESET to be FALSE
        if (!dispCtrl1.ENB || dispCtrl1.FIFOREG || dispCtrl1.FIFO32 ||
            dispCtrl1.ONECTX || dispCtrl1.ONESHOT || dispCtrl1.RESET) {

            ROS_LOG_ASSERTION(
                "DISPCTRL1 (display FIFO 1 control) is not in the expected state. (dispCtrl1=0x%x)",
                dispCtrl1.AsUlong);
            return STATUS_INVALID_DEVICE_STATE;
        }

        // expect DISPCTRL1.LINES to be equal to reported height
        // and DISPCTRL1.WIDTH to be equal to reported width
        if ((dispCtrl1.LINES != this->dxgkDisplayInfo.Height) ||
            (dispCtrl1.WIDTH != this->dxgkDisplayInfo.Width)) {

            ROS_LOG_ASSERTION(
                "Display FIFO 1 width and height do not match values reported by BIOS. (dispCtrl1=0x%x)",
                dispCtrl1.AsUlong);
            return STATUS_INVALID_DEVICE_STATE;
        }

        // Read and validate the display list address
        VC4HVS_DISPLIST dispList1 = {
            READ_REGISTER_NOFENCE_ULONG(&_hvsRegistersPtr->DISPLIST1)};
        VC4HVS_DLIST_ENTRY_UNITY displayListCopy;
        if ((dispList1.HEADE + ARRAYSIZE(displayListCopy.AsUlong)) >
             ARRAYSIZE(_hvsRegistersPtr->DLISTMEM)) {

            ROS_LOG_ASSERTION("Display list address is out of range");
            return STATUS_INVALID_DEVICE_STATE;
        }

        // Copy the display list from context memory to local memory
        READ_REGISTER_NOFENCE_BUFFER_ULONG(
            &_hvsRegistersPtr->DLISTMEM[dispList1.HEADE],
            displayListCopy.AsUlong,
            ARRAYSIZE(displayListCopy.AsUlong));

        if (displayListCopy.PointerWord0 !=
            Vc4PhysicalAddressFromVirtual(_biosFrameBufferPtr)) {

            ROS_LOG_ASSERTION("The display list does not point to the BIOS buffer");
            return STATUS_INVALID_DEVICE_STATE;
        }

        this->displayListPtr = reinterpret_cast<VC4HVS_DLIST_ENTRY_UNITY*>(
            &_hvsRegistersPtr->DLISTMEM[dispList1.HEADE]);
        this->displayListControlWord0 = displayListCopy.ControlWord0;
    }

    // All resources have been acquired. Save them in the device context

    dereferenceHpdFileObject.DoNot();
    this->hpdFileObjectPtr = localHpdFileObjectPtr;

    dereferenceI2cFileObjects.DoNot();

    unmapHvsRegisters.DoNot();
    this->hvsRegistersPtr = _hvsRegistersPtr;

    unmapPvRegisters.DoNot();
    this->pvRegistersPtr = _pvRegistersPtr;

    this->frameBufferLength = _frameBufferLength;

    unmapBiosFrameBuffer.DoNot();
    this->biosFrameBufferPtr = _biosFrameBufferPtr;

    freeSystemFrameBuffer1.DoNot();
    this->systemFrameBuffers[0] = systemFrameBuffer1;

    // initialize the frameBuffers structure
    this->frameBuffers[0] = _FRAME_BUFFER_DESCRIPTOR {
        _biosFrameBufferPtr,
        Vc4PhysicalAddressFromVirtual(_biosFrameBufferPtr)
    };

    this->frameBuffers[1] = _FRAME_BUFFER_DESCRIPTOR {
        this->systemFrameBuffers[0],
        Vc4PhysicalAddressFromVirtual(this->systemFrameBuffers[0])
    };

    static_assert(
        ARRAYSIZE(this->systemFrameBuffers) == 1,
        "Sanity check on size of systemFrameBuffers");
    static_assert(
        ARRAYSIZE(this->frameBuffers) == 2,
        "Sanity check on size of frameBuffers");

    this->activeFrameBufferId = 1;             // BIOS frame buffer
    this->pendingActiveFrameBufferId = 0;      // no swap pending
    this->backBufferId = 2;

    *NumberOfVideoPresentSourcesPtr = 1;
    *NumberOfChildrenPtr = CHILD_COUNT;     // represents the HDMI connector

    ROS_LOG_TRACE("Successfully started device.");
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
void VC4_DISPLAY::StopDevice ()
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // Close HPD notification handle
    NT_ASSERT(this->hpdFileObjectPtr);
    ObDereferenceObjectWithTag(this->hpdFileObjectPtr, VC4_ALLOC_TAG::DEVICE);
    this->hpdFileObjectPtr = nullptr;

    // Close I2C handles
    for (auto& fileObjectPtr : this->i2cFileObjectPtrs) {
        NT_ASSERT(fileObjectPtr);
        ObDereferenceObjectWithTag(fileObjectPtr, VC4_ALLOC_TAG::DEVICE);
        fileObjectPtr = nullptr;
    }

    // Disable interrupts
    WRITE_REGISTER_NOFENCE_ULONG(&this->pvRegistersPtr->IntEn, 0);

    // Make the BIOS frame buffer active before freeing system buffers
    WRITE_REGISTER_NOFENCE_ULONG(
        &this->displayListPtr->PointerWord0,
        this->frameBuffers[0].PhysicalAddress);

    // Free system frame buffers
    for (auto& frameBufferPtr : this->systemFrameBuffers) {
        NT_ASSERT(frameBufferPtr);
        MmFreeContiguousMemory(frameBufferPtr);
        frameBufferPtr = nullptr;
    }

    // Unmap BIOS frame buffer
    NT_ASSERT(this->biosFrameBufferPtr);
    MmUnmapIoSpace(
        this->biosFrameBufferPtr,
        this->frameBufferLength);
    this->biosFrameBufferPtr = nullptr;
    this->frameBufferLength = 0;

    // Unmap PixelValve register block
    NT_ASSERT(this->pvRegistersPtr);
    NTSTATUS unmapStatus = this->dxgkInterface.DxgkCbUnmapMemory(
            this->dxgkInterface.DeviceHandle,
            this->pvRegistersPtr);
    UNREFERENCED_PARAMETER(unmapStatus);
    NT_ASSERT(NT_SUCCESS(unmapStatus));
    this->pvRegistersPtr = nullptr;

    // Unmap HVS register block
    NT_ASSERT(this->hvsRegistersPtr);
    unmapStatus = this->dxgkInterface.DxgkCbUnmapMemory(
            this->dxgkInterface.DeviceHandle,
            this->hvsRegistersPtr);
    NT_ASSERT(NT_SUCCESS(unmapStatus));
    this->hvsRegistersPtr = nullptr;

    ROS_LOG_TRACE("Successfully stopped device.");
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::DispatchIoRequest (
    ULONG /*VidPnSourceId*/,
    VIDEO_REQUEST_PACKET* VideoRequestPacketPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_WARNING(
        "Unsupported IO Control Code. (VideoRequestPacketPtr->IoControlCode = 0x%lx)",
        VideoRequestPacketPtr->IoControlCode);
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::QueryChildRelations (
    DXGK_CHILD_DESCRIPTOR* ChildRelationsPtr,
    ULONG ChildRelationsSizeInBytes
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // Enumerate the child devices of the adapter. There is a single child
    // to enumerate which is the HDMI connector. The number of children
    // is specified above in NumberOfChildrenPtr

    const ULONG childRelationsCount =
        ChildRelationsSizeInBytes / sizeof(*ChildRelationsPtr);

    // The caller allocates and zeros one more entry that we specifed in
    // the NumberOfChildrenPtr output parameter of StartDevice
    NT_ASSERT(childRelationsCount == (CHILD_COUNT + 1));

    // The following code assumes a single child descriptor
    NT_ASSERT(CHILD_COUNT == 1);

    ChildRelationsPtr[0].ChildDeviceType = TypeVideoOutput;
    ChildRelationsPtr[0].ChildCapabilities.HpdAwareness = HpdAwarenessInterruptible;
    ChildRelationsPtr[0].ChildCapabilities.Type.VideoOutput.InterfaceTechnology = D3DKMDT_VOT_HDMI;
    ChildRelationsPtr[0].ChildCapabilities.Type.VideoOutput.MonitorOrientationAwareness = D3DKMDT_MOA_NONE;
    ChildRelationsPtr[0].ChildCapabilities.Type.VideoOutput.SupportsSdtvModes = FALSE;

    // child device is not an ACPI device
    NT_ASSERT(ChildRelationsPtr[0].AcpiUid == 0);

    ChildRelationsPtr[0].ChildUid = 0;

    ROS_LOG_TRACE("Child relations reported successfully.");
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::QueryChildStatus (
    DXGK_CHILD_STATUS* ChildStatusPtr,
    BOOLEAN NonDestructiveOnly
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(ChildStatusPtr->ChildUid == 0);

    switch (ChildStatusPtr->Type) {
    case StatusConnection:
        ChildStatusPtr->HotPlug.Connected = this->hdmiConnected;
        break;
    case StatusRotation:
        ROS_LOG_ERROR("Received StatusRotation query even though D3DKMDT_MOA_NONE was reported.");
        return STATUS_INVALID_PARAMETER;
    case StatusMiracastConnection:
        ROS_LOG_ERROR("Miracast is not supported.");
        return STATUS_NOT_SUPPORTED;
    case StatusUninitialized:
    default:
        ROS_LOG_ERROR(
            "Received invalid Type value in DdiQueryChildStatus. (ChildStatusPtr->Type = %d)",
            ChildStatusPtr->Type);
        return STATUS_INVALID_PARAMETER;
    }

    ROS_LOG_TRACE(
        "Child status reported successfully. (NonDestructiveOnly = %d)",
        NonDestructiveOnly);
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::QueryDeviceDescriptor (
    ULONG ChildUid,
    DXGK_DEVICE_DESCRIPTOR* DeviceDescriptorPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    UNREFERENCED_PARAMETER(ChildUid);
    NT_ASSERT(ChildUid == 0);

    ROS_LOG_TRACE(
        "Querying EDID from monitor over DDC I2C channel. (DescriptorOffset=%d, DescriptorLength=%d)",
        DeviceDescriptorPtr->DescriptorOffset,
        DeviceDescriptorPtr->DescriptorLength);

    if (!this->hdmiConnected) {
        ROS_LOG_WARNING("The HDMI connector is disconnected.");
    }

    NTSTATUS status;

    // Write segment register
    // XXX only do this if segmentNumber is nonzero
    {
        ULONG segmentNumber = DeviceDescriptorPtr->DescriptorOffset / 256;
        if (segmentNumber > BYTE_MAX) {
            ROS_LOG_ERROR(
                "Invalid EDID offset! (DescriptorOffset=%d, segmentNumber=%d)",
                DeviceDescriptorPtr->DescriptorOffset,
                segmentNumber);
            return STATUS_INVALID_PARAMETER;
        }

        BYTE writeBuf[] = { static_cast<BYTE>(segmentNumber) };
        ULONG information;
        status = Vc4SendWriteSynchronously (
                this->i2cFileObjectPtrs[I2C_CHANNEL_INDEX_EDDC],
                writeBuf,
                sizeof(writeBuf),
                &information);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "Failed to write segment address to I2C channel. (status=%!STATUS!, this->i2cFileObjectPtr2=%p, segmentNumber=%d)",
                status,
                this->i2cFileObjectPtrs[I2C_CHANNEL_INDEX_EDDC],
                segmentNumber);
        }
    }

    SPB_TRANSFER_LIST_AND_ENTRIES(2) inputBuffer;
    SPB_TRANSFER_LIST_INIT(&inputBuffer.List, 2);

    ULONG offsetWithinSegment = DeviceDescriptorPtr->DescriptorOffset % 256;
    BYTE writeBuf[] = { static_cast<BYTE>(offsetWithinSegment) };
    inputBuffer.List.Transfers[0] = SPB_TRANSFER_LIST_ENTRY_INIT_SIMPLE(
        SpbTransferDirectionToDevice,
        0,
        writeBuf,
        sizeof(writeBuf));

#pragma warning(suppress: 26000) // Supress OACR buffer overflow warning
#pragma warning(suppress: 6201)  // Suppress OACR index out of range warning
    inputBuffer.List.Transfers[1] = SPB_TRANSFER_LIST_ENTRY_INIT_SIMPLE(
        SpbTransferDirectionFromDevice,
        0,
        DeviceDescriptorPtr->DescriptorBuffer,
        DeviceDescriptorPtr->DescriptorLength);

    ULONG bytesTransferred;
    status = Vc4SendIoctlSynchronously(
            this->i2cFileObjectPtrs[I2C_CHANNEL_INDEX_DDC],
            IOCTL_SPB_EXECUTE_SEQUENCE,
            &inputBuffer,
            sizeof(inputBuffer),
            nullptr,
            0,
            FALSE,                      // InternalDeviceIoControl
            &bytesTransferred);

    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "Failed to query EDID from monitor. (status=%!STATUS!, DescriptorOffset=%d, DescriptorLength=%d)",
            status,
            DeviceDescriptorPtr->DescriptorOffset,
            DeviceDescriptorPtr->DescriptorLength);
        return Vc4SanitizeNtstatus(status);
    }

    if (bytesTransferred != (1 + DeviceDescriptorPtr->DescriptorLength)) {
        ROS_LOG_ERROR(
            "An unexpected number of bytes was transferred. (bytesTransferred=%d, expected=%d)",
            bytesTransferred,
            1 + DeviceDescriptorPtr->DescriptorLength);
        return STATUS_UNSUCCESSFUL;
    }

    NT_ASSERT(status == STATUS_SUCCESS);
    return status;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::SetPowerState (
    ULONG DeviceUid,
    DEVICE_POWER_STATE DevicePowerState,
    POWER_ACTION ActionType
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    if (DeviceUid == DISPLAY_ADAPTER_HW_ID) {
        // TODO put the adapter into a low power state
    } else {
        NT_ASSERT(DeviceUid == 0);
        // TODO put the attached monitor into the specified power state
    }

    ROS_LOG_TRACE(
        L"Successfully set power state. (DeviceUid = %d, DevicePowerState = %d, ActionType = %d)",
        DeviceUid,
        DevicePowerState,
        ActionType);

    return STATUS_SUCCESS;
}

//
// Even though this driver does not support hardware cursors,
// and reports such in QueryAdapterInfo. This function can still be called to
// set the pointer to not visible
//
_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::SetPointerPosition (
    const DXGKARG_SETPOINTERPOSITION* SetPointerPositionPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(SetPointerPositionPtr->VidPnSourceId == 0);
    if (!SetPointerPositionPtr->Flags.Visible) {
        ROS_LOG_TRACE("Received request to set pointer visibility to OFF.");
        return STATUS_SUCCESS;
    } else {
        ROS_LOG_ASSERTION("SetPointerPosition should never be called to set the pointer to visible since VC4 doesn't support hardware cursors.");
        return STATUS_UNSUCCESSFUL;
    }
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::SetPointerShape (
    const DXGKARG_SETPOINTERSHAPE* SetPointerShapePtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    UNREFERENCED_PARAMETER(SetPointerShapePtr);
    NT_ASSERT(SetPointerShapePtr->VidPnSourceId == 0);

    ROS_LOG_ASSERTION("SetPointerShape should never be called since VC4 doesn't support hardware cursors.");
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::IsSupportedVidPn (
    DXGKARG_ISSUPPORTEDVIDPN* IsSupportedVidPnPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    IsSupportedVidPnPtr->IsVidPnSupported = FALSE;

    if (IsSupportedVidPnPtr->hDesiredVidPn == 0) {
        IsSupportedVidPnPtr->IsVidPnSupported = TRUE;
        ROS_LOG_TRACE("Returning IsVidPnSupported=TRUE for hDesiredVidPn=0.");
        return STATUS_SUCCESS;
    }

    // Verify that there is exactly one path and source and target IDs are both 0
    const DXGK_VIDPN_INTERFACE* vidPnInterfacePtr;
    NTSTATUS status = this->dxgkInterface.DxgkCbQueryVidPnInterface(
            IsSupportedVidPnPtr->hDesiredVidPn,
            DXGK_VIDPN_INTERFACE_VERSION_V1,
            &vidPnInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DxgkCbQueryVidPnInterface() failed. (status = %!STATUS!, IsSupportedVidPnPtr->hDesiredVidPn = %p, this = %p)",
            status,
            IsSupportedVidPnPtr->hDesiredVidPn,
            this);
        return status;
    }

    D3DKMDT_HVIDPNTOPOLOGY vidPnTopologyHandle;
    const DXGK_VIDPNTOPOLOGY_INTERFACE* topologyInterfacePtr;
    status = vidPnInterfacePtr->pfnGetTopology(
        IsSupportedVidPnPtr->hDesiredVidPn,
        &vidPnTopologyHandle,
        &topologyInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "pfnGetTopology() failed. (status = %!STATUS!, IsSupportedVidPnPtr->hDesiredVidPn = %p, vidPnInterfacePtr = %p)",
            status,
            IsSupportedVidPnPtr->hDesiredVidPn,
            vidPnInterfacePtr);
        return status;
    }

    // verify this topology contains exactly one path
    SIZE_T numPaths;
    status = topologyInterfacePtr->pfnGetNumPaths(
            vidPnTopologyHandle,
            &numPaths);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "Failed to get number of paths in topology. (status = %!STATUS!, vidPnTopologyHandle = %p)",
            status,
            vidPnTopologyHandle);
        return status;
    }

    if (numPaths != 1) {
        ROS_LOG_ERROR(
            "Returning 'not supported' for a topology that does not contain exactly 1 path. (numPaths = %d, vidPnTopologyHandle = %p)",
            numPaths,
            vidPnTopologyHandle);
        NT_ASSERT(!IsSupportedVidPnPtr->IsVidPnSupported);
        return STATUS_SUCCESS;
    }

    // get the only path in the topology
    const D3DKMDT_VIDPN_PRESENT_PATH* presentPathPtr;
    status = topologyInterfacePtr->pfnAcquireFirstPathInfo(
            vidPnTopologyHandle,
            &presentPathPtr);
    if (status != STATUS_SUCCESS) {
        ROS_LOG_ERROR(
            "Failed to get first path info from vidpn topology. (status = %!STATUS!, IsSupportedVidPnPtr->hDesiredVidPn = %p, vidPnTopologyHandle = %p)",
            status,
            IsSupportedVidPnPtr->hDesiredVidPn,
            vidPnTopologyHandle);
        return status;
    }
    NT_ASSERT(presentPathPtr);
    auto releasePathInfo = VC4_FINALLY::Do([&, presentPathPtr] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = topologyInterfacePtr->pfnReleasePathInfo(
                vidPnTopologyHandle,
                presentPathPtr);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    // We support exactly one source and target
    if (!((presentPathPtr->VidPnSourceId == 0) &&
          (presentPathPtr->VidPnTargetId == 0)))
    {
        ROS_LOG_ERROR(
            "Returning 'not supported' for out of range source or target id. (presentPathPtr->VidPnSourceId = %d, presentPathPtr->VidPnTargetId = %d)",
            presentPathPtr->VidPnSourceId,
            presentPathPtr->VidPnTargetId);
        NT_ASSERT(!IsSupportedVidPnPtr->IsVidPnSupported);
        return STATUS_SUCCESS;
    }

    ROS_LOG_TRACE(
        "Returning 'supported' for vidpn. (IsSupportedVidPnPtr->hDesiredVidPn = %p)",
        IsSupportedVidPnPtr->hDesiredVidPn);

    IsSupportedVidPnPtr->IsVidPnSupported = TRUE;
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::RecommendFunctionalVidPn (
    const DXGKARG_RECOMMENDFUNCTIONALVIDPN* const /*RecommendFunctionalVidPnPtr*/
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::EnumVidPnCofuncModality (
    const DXGKARG_ENUMVIDPNCOFUNCMODALITY* const EnumCofuncModalityPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // get the vidPn interface
    const DXGK_VIDPN_INTERFACE* vidPnInterfacePtr;
    NTSTATUS status = this->dxgkInterface.DxgkCbQueryVidPnInterface(
            EnumCofuncModalityPtr->hConstrainingVidPn,
            DXGK_VIDPN_INTERFACE_VERSION_V1,
            &vidPnInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DxgkCbQueryVidPnInterface() failed. (status = %!STATUS!, EnumCofuncModalityPtr->hConstrainingVidPn, = %p, this = %p)",
            status,
            EnumCofuncModalityPtr->hConstrainingVidPn,
            this);
        return status;
    }

    // get the topology
    D3DKMDT_HVIDPNTOPOLOGY vidPnTopologyHandle;
    const DXGK_VIDPNTOPOLOGY_INTERFACE* topologyInterfacePtr;
    status = vidPnInterfacePtr->pfnGetTopology(
        EnumCofuncModalityPtr->hConstrainingVidPn,
        &vidPnTopologyHandle,
        &topologyInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "pfnGetTopology() failed. (status = %!STATUS!, EnumCofuncModalityPtr->hConstrainingVidPn, = %p, vidPnInterfacePtr = %p)",
            status,
            EnumCofuncModalityPtr->hConstrainingVidPn,
            vidPnInterfacePtr);
        return status;
    }

    // iterate through each path in the topology
    const D3DKMDT_VIDPN_PRESENT_PATH* presentPathPtr;
    status = topologyInterfacePtr->pfnAcquireFirstPathInfo(
            vidPnTopologyHandle,
            &presentPathPtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "Failed to get first path info from vidpn topology. (status = %!STATUS!, EnumCofuncModalityPtr->hConstrainingVidPn, = %p, vidPnTopologyHandle = %p)",
            status,
            EnumCofuncModalityPtr->hConstrainingVidPn,
            vidPnTopologyHandle);
        return status;
    }

    while (status == STATUS_SUCCESS) {
        NT_ASSERT(presentPathPtr);
        auto releasePathInfo = VC4_FINALLY::Do([&, presentPathPtr] () {
            PAGED_CODE();
            NTSTATUS releaseStatus = topologyInterfacePtr->pfnReleasePathInfo(
                    vidPnTopologyHandle,
                    presentPathPtr);
            UNREFERENCED_PARAMETER(releaseStatus);
            NT_ASSERT(NT_SUCCESS(releaseStatus));
        });

        // If this source mode set isn't the pivot point, inspect the source
        // mode set and potentially add cofunctional modes
        if (!((EnumCofuncModalityPtr->EnumPivotType == D3DKMDT_EPT_VIDPNSOURCE) &&
              (EnumCofuncModalityPtr->EnumPivot.VidPnSourceId == presentPathPtr->VidPnSourceId)))
        {
            status = SourceHasPinnedMode(
                    EnumCofuncModalityPtr->hConstrainingVidPn,
                    vidPnInterfacePtr,
                    presentPathPtr->VidPnSourceId);
            switch (status) {
            case STATUS_SUCCESS: break;
            case STATUS_NOT_FOUND:
            {
                // This source does not have a pinned mode.
                // Create a new source mode set
                ROS_LOG_TRACE(
                    "This source does not have a pinned mode. Creating a new source mode set. (presentPathPtr->VidPnSourceId = %d)",
                    presentPathPtr->VidPnSourceId);

                status = this->CreateAndAssignSourceModeSet(
                        EnumCofuncModalityPtr->hConstrainingVidPn,
                        vidPnInterfacePtr,
                        presentPathPtr->VidPnSourceId,
                        presentPathPtr->VidPnTargetId);
                if (!NT_SUCCESS(status)) {
                    return status;
                }

                break;
            }
            default:
                return status;
            }
        } // source mode set

        // If this target mode set isn't the pivot point, inspect the target
        // mode set and potentially add cofunctional modes
        if (!((EnumCofuncModalityPtr->EnumPivotType == D3DKMDT_EPT_VIDPNTARGET) &&
              (EnumCofuncModalityPtr->EnumPivot.VidPnTargetId == presentPathPtr->VidPnTargetId)))
        {
            status = TargetHasPinnedMode(
                    EnumCofuncModalityPtr->hConstrainingVidPn,
                    vidPnInterfacePtr,
                    presentPathPtr->VidPnTargetId);
            switch (status) {
            case STATUS_SUCCESS: break;
            case STATUS_NOT_FOUND:
            {
                // This target does not have a pinned mode.
                // Create a new target mode set
                ROS_LOG_TRACE(
                    "This target does not have a pinned mode. Creating a new target mode set. (presentPathPtr->VidPnTargetId = %d)",
                    presentPathPtr->VidPnTargetId);

                status = this->CreateAndAssignTargetModeSet(
                        EnumCofuncModalityPtr->hConstrainingVidPn,
                        vidPnInterfacePtr,
                        presentPathPtr->VidPnSourceId,
                        presentPathPtr->VidPnTargetId);
                if (!NT_SUCCESS(status)) {
                    return status;
                }

                break;
            }
            default:
                return status;
            }
        } // target mode set

        D3DKMDT_VIDPN_PRESENT_PATH modifiedPresentPath = *presentPathPtr;
        bool presentPathModified = false;

        // SCALING: If this path's scaling isn't the pivot point, do work on the scaling support
        if (!((EnumCofuncModalityPtr->EnumPivotType == D3DKMDT_EPT_SCALING) &&
              (EnumCofuncModalityPtr->EnumPivot.VidPnSourceId == presentPathPtr->VidPnSourceId) &&
              (EnumCofuncModalityPtr->EnumPivot.VidPnTargetId == presentPathPtr->VidPnTargetId)))
        {
            // If the scaling is unpinned, then modify the scaling support field
            if (presentPathPtr->ContentTransformation.Scaling == D3DKMDT_VPPS_UNPINNED)
            {
                // Identity and centered scaling are supported, but not any stretch modes
                ROS_LOG_TRACE("Setting scaling support to Identity.");

                modifiedPresentPath.ContentTransformation.ScalingSupport =
                    D3DKMDT_VIDPN_PRESENT_PATH_SCALING_SUPPORT();

                // We do not support scaling
                modifiedPresentPath.ContentTransformation.ScalingSupport.Identity = TRUE;

                presentPathModified = true;
            }
        } // scaling

        // ROTATION: If this path's rotation isn't the pivot point, do work on the rotation support
        if (!((EnumCofuncModalityPtr->EnumPivotType != D3DKMDT_EPT_ROTATION) &&
              (EnumCofuncModalityPtr->EnumPivot.VidPnSourceId == presentPathPtr->VidPnSourceId) &&
              (EnumCofuncModalityPtr->EnumPivot.VidPnTargetId == presentPathPtr->VidPnTargetId)))
        {
            // If the rotation is unpinned, then modify the rotation support field
            if (presentPathPtr->ContentTransformation.Rotation == D3DKMDT_VPPR_UNPINNED)
            {
                ROS_LOG_TRACE("Setting rotation support to Identity.");

                modifiedPresentPath.ContentTransformation.RotationSupport =
                    D3DKMDT_VIDPN_PRESENT_PATH_ROTATION_SUPPORT();

                // We do not support rotation
                modifiedPresentPath.ContentTransformation.RotationSupport.Identity = TRUE;
                modifiedPresentPath.ContentTransformation.RotationSupport.Offset0 = TRUE;

                presentPathModified = true;
            }
        } // rotation

        // Update the content transformation
        if (presentPathModified) {
            // The correct path will be found by this function and the appropriate fields updated
            status = topologyInterfacePtr->pfnUpdatePathSupportInfo(
                    vidPnTopologyHandle,
                    &modifiedPresentPath);
            if (!NT_SUCCESS(status)) {
                ROS_LOG_ERROR(
                    "DXGK_VIDPNTOPOLOGY_INTERFACE::pfnUpdatePathSupportInfo() failed. (status = %!STATUS!)",
                    status);
                return status;
            }
        }

        // get next path
        const D3DKMDT_VIDPN_PRESENT_PATH* nextPresentPathPtr;
        status = topologyInterfacePtr->pfnAcquireNextPathInfo(
            vidPnTopologyHandle,
            presentPathPtr,
            &nextPresentPathPtr);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "pfnAcquireNextPathInfo() failed. (status = %!STATUS!, EnumCofuncModalityPtr->hConstrainingVidPn, = %p, vidPnTopologyHandle = %p)",
                status,
                EnumCofuncModalityPtr->hConstrainingVidPn,
                vidPnTopologyHandle);
            return status;
        }

        presentPathPtr = nextPresentPathPtr;
    }

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::SetVidPnSourceVisibility (
    const DXGKARG_SETVIDPNSOURCEVISIBILITY* SetVidPnSourceVisibilityPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_TRACE(
        "Received request to set visibility. (VidPnSourceId = %d, Visible = %d)",
        SetVidPnSourceVisibilityPtr->VidPnSourceId,
        SetVidPnSourceVisibilityPtr->Visible);

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::CommitVidPn (
    const DXGKARG_COMMITVIDPN* const CommitVidPnPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_TRACE(
        "DdiCommitVidPn() was called. (hFunctionalVidPn = %p, AffectedVidPnSourceId = %d, MonitorConnectivityChecks = %d, hPrimaryAllocation = %p, Flags.PathPowerTransition = %d, Flags.PathPoweredOff = %d)",
        CommitVidPnPtr->hFunctionalVidPn,
        CommitVidPnPtr->AffectedVidPnSourceId,
        CommitVidPnPtr->MonitorConnectivityChecks,
        CommitVidPnPtr->hPrimaryAllocation,
        CommitVidPnPtr->Flags.PathPowerTransition,
        CommitVidPnPtr->Flags.PathPoweredOff);

#ifdef DBG
    this->dbgHelper.DumpVidPn(CommitVidPnPtr->hFunctionalVidPn);
#endif // DBG

    // get the vidPn interface
    const DXGK_VIDPN_INTERFACE* vidPnInterfacePtr;
    NTSTATUS status = this->dxgkInterface.DxgkCbQueryVidPnInterface(
            CommitVidPnPtr->hFunctionalVidPn,
            DXGK_VIDPN_INTERFACE_VERSION_V1,
            &vidPnInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DxgkCbQueryVidPnInterface() failed. (status = %!STATUS!, CommitVidPnPtr->hFunctionalVidPn, = %p, this = %p)",
            status,
            CommitVidPnPtr->hFunctionalVidPn,
            this);
        return status;
    }

    // get the topology
    D3DKMDT_HVIDPNTOPOLOGY vidPnTopologyHandle;
    const DXGK_VIDPNTOPOLOGY_INTERFACE* topologyInterfacePtr;
    status = vidPnInterfacePtr->pfnGetTopology(
        CommitVidPnPtr->hFunctionalVidPn,
        &vidPnTopologyHandle,
        &topologyInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "pfnGetTopology() failed. (status = %!STATUS!, CommitVidPnPtr->hFunctionalVidPn, = %p, vidPnInterfacePtr = %p)",
            status,
            CommitVidPnPtr->hFunctionalVidPn,
            vidPnInterfacePtr);
        return status;
    }

    // Find out the number of paths now, if it's 0 don't bother with source
    // mode set and pinned mode, just clear current and then quit
    SIZE_T numPaths;
    status = topologyInterfacePtr->pfnGetNumPaths(
            vidPnTopologyHandle,
            &numPaths);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPNTOPOLOGY_INTERFACE::pfnGetNumPaths() failed. (status = %!STATUS!)",
            status);
        return status;
    }

    if (numPaths == 0) {
        ROS_LOG_INFORMATION(L"There are no paths in this topology.");
        return STATUS_SUCCESS;
    }

    // Get the source mode set for this SourceId
    D3DKMDT_HVIDPNSOURCEMODESET sourceModeSetHandle;
    const DXGK_VIDPNSOURCEMODESET_INTERFACE* smsInterfacePtr;
    status = vidPnInterfacePtr->pfnAcquireSourceModeSet(
            CommitVidPnPtr->hFunctionalVidPn,
            CommitVidPnPtr->AffectedVidPnSourceId,
            &sourceModeSetHandle,
            &smsInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPN_INTERFACE::pfnAcquireSourceModeSet() failed. (status = %!STATUS!)",
            status);
        return status;
    }
    NT_ASSERT(sourceModeSetHandle);
    auto releaseSms = VC4_FINALLY::Do([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = vidPnInterfacePtr->pfnReleaseSourceModeSet(
                CommitVidPnPtr->hFunctionalVidPn,
                sourceModeSetHandle);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    // Get the source mode to pin
    const D3DKMDT_VIDPN_SOURCE_MODE* pinnedSourceModeInfoPtr;
    status = smsInterfacePtr->pfnAcquirePinnedModeInfo(
            sourceModeSetHandle,
            &pinnedSourceModeInfoPtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPNSOURCEMODESET_INTERFACE::pfnAcquirePinnedModeInfo() failed. (status = %!STATUS!, sourceModeSetHandle = %p)",
            status,
            sourceModeSetHandle);
        return status;
    }

    if (status != STATUS_SUCCESS) {
        NT_ASSERT(!pinnedSourceModeInfoPtr);

        // There is no mode to pin on this source
        return STATUS_SUCCESS;
    }

    auto releaseModeInfo = VC4_FINALLY::Do([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = smsInterfacePtr->pfnReleaseModeInfo(
                sourceModeSetHandle,
                pinnedSourceModeInfoPtr);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    // TODO: shouldn't this vidpn be valid by design of IsVidPnSupported
    // and EnumCofuncModality? Should these be changed to assertions?
    status = this->IsVidPnSourceModeFieldsValid(pinnedSourceModeInfoPtr);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if ((pinnedSourceModeInfoPtr->Format.Graphics.PrimSurfSize.cx !=
         this->dxgkVideoSignalInfo.TotalSize.cx) ||
        (pinnedSourceModeInfoPtr->Format.Graphics.PrimSurfSize.cy !=
         this->dxgkVideoSignalInfo.TotalSize.cy))
    {
        ROS_LOG_ERROR(
            "VidPn source has different size than monitor. (pinnedSourceModeInfoPtr->Format.Graphics.PrimSurfSize = %d,%d, this->dxgkVideoSignalInfo.TotalSize = %d, %d)",
            pinnedSourceModeInfoPtr->Format.Graphics.PrimSurfSize.cx,
            pinnedSourceModeInfoPtr->Format.Graphics.PrimSurfSize.cy,
            this->dxgkVideoSignalInfo.TotalSize.cx,
            this->dxgkVideoSignalInfo.TotalSize.cy);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }

    // Get the number of paths from this source so we can loop through all paths
    SIZE_T numPathsFromSource = 0;
    status = topologyInterfacePtr->pfnGetNumPathsFromSource(
            vidPnTopologyHandle,
            CommitVidPnPtr->AffectedVidPnSourceId,
            &numPathsFromSource);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPNTOPOLOGY_INTERFACE::pfnGetNumPathsFromSource() failed. (status = %!STATUS!)",
            status);
        return status;
    }

    // Loop through all paths to set this mode
    for (SIZE_T pathIndex = 0; pathIndex < numPathsFromSource; ++pathIndex) {
        // Get the target id for this path
        D3DDDI_VIDEO_PRESENT_TARGET_ID targetId;
        status = topologyInterfacePtr->pfnEnumPathTargetsFromSource(
                vidPnTopologyHandle,
                CommitVidPnPtr->AffectedVidPnSourceId,
                pathIndex,
                &targetId);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "DXGK_VIDPNTOPOLOGY_INTERFACE::pfnEnumPathTargetsFromSource() failed. (status = %!STATUS!)",
                status);
            return status;
        }

        // Get the actual path info
        const D3DKMDT_VIDPN_PRESENT_PATH* presentPathPtr;
        status = topologyInterfacePtr->pfnAcquirePathInfo(
                vidPnTopologyHandle,
                CommitVidPnPtr->AffectedVidPnSourceId,
                targetId,
                &presentPathPtr);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "DXGK_VIDPNTOPOLOGY_INTERFACE::pfnAcquirePathInfo() failed. (status = %!STATUS!, AffectedVidPnSourceId = %d, targetId = %d)",
                status,
                CommitVidPnPtr->AffectedVidPnSourceId,
                targetId);
            return status;
        }
        auto releasePathInfo = VC4_FINALLY::Do([&] {
            PAGED_CODE();
            NTSTATUS releaseStatus = topologyInterfacePtr->pfnReleasePathInfo(
                    vidPnTopologyHandle,
                    presentPathPtr);
            UNREFERENCED_PARAMETER(releaseStatus);
            NT_ASSERT(NT_SUCCESS(releaseStatus));
        });

        // TODO shouldn't this path be valid by design of IsVidPnSupported
        // and EnumCofuncModality?
        status = this->IsVidPnPathFieldsValid(presentPathPtr);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    this->dxgkCurrentSourceMode = *pinnedSourceModeInfoPtr;

    ROS_LOG_TRACE(
        "Successfully committed VidPn. (hFunctionalVidPn = %p, PrimSurfSize = %d, %d, VisibleRegionSize = %d,%d, Stride = %d, PixelFormat = %d, ColorBasis = %d, PixelValueAccessMode = %d)",
        CommitVidPnPtr->hFunctionalVidPn,
        pinnedSourceModeInfoPtr->Format.Graphics.PrimSurfSize.cx,
        pinnedSourceModeInfoPtr->Format.Graphics.PrimSurfSize.cy,
        pinnedSourceModeInfoPtr->Format.Graphics.VisibleRegionSize.cx,
        pinnedSourceModeInfoPtr->Format.Graphics.VisibleRegionSize.cy,
        pinnedSourceModeInfoPtr->Format.Graphics.Stride,
        pinnedSourceModeInfoPtr->Format.Graphics.PixelFormat,
        pinnedSourceModeInfoPtr->Format.Graphics.ColorBasis,
        pinnedSourceModeInfoPtr->Format.Graphics.PixelValueAccessMode);

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::UpdateActiveVidPnPresentPath (
    const DXGKARG_UPDATEACTIVEVIDPNPRESENTPATH* const /*UpdateActiveVidPnPresentPathPtr*/
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

//
// TODO This callback is optional - if we return an EDID in
// DdiQueryDeviceDescriptor, do we have to implement it?
//
_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::RecommendMonitorModes (
    const DXGKARG_RECOMMENDMONITORMODES* const RecommendMonitorModesPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(RecommendMonitorModesPtr->VideoPresentTargetId == 0);

#ifdef DBG
    this->dbgHelper.DumpMonitorModes(RecommendMonitorModesPtr);
#endif // DBG

    const auto& tbl = *RecommendMonitorModesPtr->pMonitorSourceModeSetInterface;

    D3DKMDT_MONITOR_SOURCE_MODE* monitorModePtr;
    NTSTATUS status = tbl.pfnCreateNewModeInfo(
            RecommendMonitorModesPtr->hMonitorSourceModeSet,
            &monitorModePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "pfnCreateNewModeInfo() failed. (status = %!STATUS!)",
            status);
        return status;
    }
    auto releaseMonitorMode = VC4_FINALLY::DoUnless([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = tbl.pfnReleaseModeInfo(
                RecommendMonitorModesPtr->hMonitorSourceModeSet,
                monitorModePtr);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    *monitorModePtr = D3DKMDT_MONITOR_SOURCE_MODE();
    monitorModePtr->VideoSignalInfo = this->dxgkVideoSignalInfo;

    // We set the preference to PREFERRED since this is the only supported mode
    monitorModePtr->Origin = D3DKMDT_MCO_DRIVER;
    monitorModePtr->Preference = D3DKMDT_MP_PREFERRED;
    monitorModePtr->ColorBasis = D3DKMDT_CB_SRGB;
    monitorModePtr->ColorCoeffDynamicRanges.FirstChannel = 8;
    monitorModePtr->ColorCoeffDynamicRanges.SecondChannel = 8;
    monitorModePtr->ColorCoeffDynamicRanges.ThirdChannel = 8;
    monitorModePtr->ColorCoeffDynamicRanges.FourthChannel = 8;

    status = tbl.pfnAddMode(
            RecommendMonitorModesPtr->hMonitorSourceModeSet,
            monitorModePtr);
    if (!NT_SUCCESS(status)) {
        if (status == STATUS_GRAPHICS_MODE_ALREADY_IN_MODESET) {
            status = STATUS_SUCCESS;
        } else {
            ROS_LOG_ERROR(
                "pfnAddMode failed. (status = %!STATUS!, RecommendMonitorModesPtr->hMonitorSourceModeSet = %p, monitorModePtr = %p)",
                status,
                RecommendMonitorModesPtr->hMonitorSourceModeSet,
                monitorModePtr);
        }
        return status;
    }
    releaseMonitorMode.DoNot();

    ROS_LOG_TRACE(
        "Added single monitor mode. (...TotalSize = %d,%d)",
        monitorModePtr->VideoSignalInfo.TotalSize.cx,
        monitorModePtr->VideoSignalInfo.TotalSize.cy);
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::QueryVidPnHWCapability (
    DXGKARG_QUERYVIDPNHWCAPABILITY* VidPnHWCapsPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_TRACE(
        "DdiQueryVidPnHWCapability() was called. (hFunctionalVidPn = %p, SourceId = %d, TargetId = %d)",
        VidPnHWCapsPtr->hFunctionalVidPn,
        VidPnHWCapsPtr->SourceId,
        VidPnHWCapsPtr->TargetId);

    NT_ASSERT(VidPnHWCapsPtr->SourceId == 0);
    NT_ASSERT(VidPnHWCapsPtr->TargetId == 0);

    // we do not have hardware capability for any transformations
    VidPnHWCapsPtr->VidPnHWCaps.DriverRotation = FALSE;             // driver does not support rotation
    VidPnHWCapsPtr->VidPnHWCaps.DriverScaling = FALSE;              // driver does not support scaling
    VidPnHWCapsPtr->VidPnHWCaps.DriverCloning = FALSE;              // driver does not support cloning
    VidPnHWCapsPtr->VidPnHWCaps.DriverColorConvert = TRUE;          // driver does color conversions in software
    VidPnHWCapsPtr->VidPnHWCaps.DriverLinkedAdapaterOutput = FALSE; // driver does not support linked adapters
    VidPnHWCapsPtr->VidPnHWCaps.DriverRemoteDisplay = FALSE;        // driver does not support remote displays

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::PresentDisplayOnly (
    const DXGKARG_PRESENT_DISPLAYONLY* PresentDisplayOnlyPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_TRACE(
        "Received present call. (VidPnSourceId = %d, BytesPerPixel = %d, Pitch = %d, Flags.Rotate = %d, NumMoves = %d, NumDirtyRects = %d)",
        PresentDisplayOnlyPtr->VidPnSourceId,
        PresentDisplayOnlyPtr->BytesPerPixel,
        PresentDisplayOnlyPtr->Pitch,
        PresentDisplayOnlyPtr->Flags.Rotate,
        PresentDisplayOnlyPtr->NumMoves,
        PresentDisplayOnlyPtr->NumDirtyRects);

    NT_ASSERT(PresentDisplayOnlyPtr->BytesPerPixel == 4);
    NT_ASSERT(ULONG(PresentDisplayOnlyPtr->Pitch) == this->dxgkDisplayInfo.Pitch);
    NT_ASSERT(!PresentDisplayOnlyPtr->Flags.Rotate);

    // Get a buffer from the free buffer list
    FRAME_BUFFER_ID localBackBufferId = InterlockedAnd(&this->backBufferId, 0);
    NT_ASSERT(localBackBufferId);

    _FRAME_BUFFER_DESCRIPTOR* backBufferDescriptorPtr =
        this->getFrameBufferDescriptorFromId(localBackBufferId);

    // Copy source pixels to back buffer
    __try {
        // must copy entire frame each time
        memcpy(
            backBufferDescriptorPtr->BufferPtr,
            PresentDisplayOnlyPtr->pSource,
            this->frameBufferLength);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ROS_LOG_ERROR("An exception occurred while accessing the user buffer.");
        return STATUS_UNSUCCESSFUL;
    }

    // Update the source address in the display list
    WRITE_REGISTER_NOFENCE_ULONG(
        &this->displayListPtr->PointerWord0,
        backBufferDescriptorPtr->PhysicalAddress);

    // Save the back buffer as the new pending active buffer
    FRAME_BUFFER_ID oldPendingActive = InterlockedExchange(
            &this->pendingActiveFrameBufferId,
            localBackBufferId);
    if (oldPendingActive) {
        ROS_LOG_ASSERTION(
            "PresentDisplayOnly was called before the previous frame was reported as complete! (localBackBufferId=%d, oldPendingActive=%d, activeFrameBufferId=%d)",
            localBackBufferId,
            oldPendingActive,
            this->activeFrameBufferId);
        return STATUS_UNSUCCESSFUL;
    }

    // Enable the VFP interrupt
    this->pixelValveIntEn.VfpStart = TRUE;
    WRITE_REGISTER_NOFENCE_ULONG(
        &this->pvRegistersPtr->IntEn,
        this->pixelValveIntEn.AsUlong);

    return STATUS_PENDING;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::StopDeviceAndReleasePostDisplayOwnership (
    D3DDDI_VIDEO_PRESENT_TARGET_ID /*TargetId*/,
    DXGK_DISPLAY_INFORMATION* /*DisplayInfoPtr*/
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

//
// Returns STATUS_SUCCESS if the source has a pinned mode, or STATUS_NOT_FOUND
// if the source does not have a pinned mode.
//
_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::SourceHasPinnedMode (
    D3DKMDT_HVIDPN VidPnHandle,
    const DXGK_VIDPN_INTERFACE* VidPnInterfacePtr,
    D3DKMDT_VIDEO_PRESENT_SOURCE_MODE_ID SourceId
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // Get the source mode set for this SourceId
    D3DKMDT_HVIDPNSOURCEMODESET sourceModeSetHandle;
    const DXGK_VIDPNSOURCEMODESET_INTERFACE* smsInterfacePtr;
    NTSTATUS status = VidPnInterfacePtr->pfnAcquireSourceModeSet(
            VidPnHandle,
            SourceId,
            &sourceModeSetHandle,
            &smsInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPN_INTERFACE::pfnAcquireSourceModeSet() failed. (status = %!STATUS!)",
            status);
        return Vc4SanitizeNtstatus(status);
    }
    NT_ASSERT(sourceModeSetHandle);
    auto releaseSms = VC4_FINALLY::Do([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = VidPnInterfacePtr->pfnReleaseSourceModeSet(
                VidPnHandle,
                sourceModeSetHandle);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    // Get the pinned mode info for this source info set
    const D3DKMDT_VIDPN_SOURCE_MODE* pinnedSourceModeInfoPtr;
    status = smsInterfacePtr->pfnAcquirePinnedModeInfo(
            sourceModeSetHandle,
            &pinnedSourceModeInfoPtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPNSOURCEMODESET_INTERFACE::pfnAcquirePinnedModeInfo() failed. (status = %!STATUS!, sourceModeSetHandle = %p)",
            status,
            sourceModeSetHandle);
        return Vc4SanitizeNtstatus(status);
    }

    if (status != STATUS_SUCCESS) {
        NT_ASSERT(!pinnedSourceModeInfoPtr);
        return STATUS_NOT_FOUND;
    }

    auto releaseModeInfo = VC4_FINALLY::Do([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = smsInterfacePtr->pfnReleaseModeInfo(
                sourceModeSetHandle,
                pinnedSourceModeInfoPtr);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    return STATUS_SUCCESS;
}

//
// Creates a new source mode set, adds a mode that is compatible with the
// connected monitor
//
_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::CreateAndAssignSourceModeSet (
    D3DKMDT_HVIDPN VidPnHandle,
    const DXGK_VIDPN_INTERFACE* VidPnInterfacePtr,
    D3DKMDT_VIDEO_PRESENT_SOURCE_MODE_ID SourceId,
    D3DKMDT_VIDEO_PRESENT_SOURCE_MODE_ID /*TargetId*/
    ) const
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // Create a new source mode set which will be added to the constraining VidPn with all the possible modes
    D3DKMDT_HVIDPNSOURCEMODESET sourceModeSetHandle;
    const DXGK_VIDPNSOURCEMODESET_INTERFACE* smsInterfacePtr;
    NTSTATUS status = VidPnInterfacePtr->pfnCreateNewSourceModeSet(
            VidPnHandle,
            SourceId,
            &sourceModeSetHandle,
            &smsInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPN_INTERFACE::pfnCreateNewSourceModeSet failed. (status = %!STATUS!)",
            status);
        return status;
    }
    NT_ASSERT(sourceModeSetHandle);
    auto releaseSms = VC4_FINALLY::DoUnless([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = VidPnInterfacePtr->pfnReleaseSourceModeSet(
                VidPnHandle,
                sourceModeSetHandle);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    // Create new mode info and add it to the source mode set
    {
        D3DKMDT_VIDPN_SOURCE_MODE* sourceModeInfoPtr;
        status = smsInterfacePtr->pfnCreateNewModeInfo(
                sourceModeSetHandle,
                &sourceModeInfoPtr);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "DXGK_VIDPNSOURCEMODESET_INTERFACE::pfnCreateNewModeInfo() failed. (status = %!STATUS!)",
                status);
            return status;
        }
        NT_ASSERT(sourceModeInfoPtr);
        auto releaseModeInfo = VC4_FINALLY::DoUnless([&] () {
            PAGED_CODE();
            NTSTATUS releaseStatus = smsInterfacePtr->pfnReleaseModeInfo(
                    sourceModeSetHandle,
                    sourceModeInfoPtr);
            UNREFERENCED_PARAMETER(releaseStatus);
            NT_ASSERT(NT_SUCCESS(releaseStatus));
        });

        // Populate mode info with values from current mode and hard-coded values
        // Always report 32 bpp format, this will be color converted during the present if the mode is < 32bpp
        *sourceModeInfoPtr = D3DKMDT_VIDPN_SOURCE_MODE();
        sourceModeInfoPtr->Type = D3DKMDT_RMT_GRAPHICS;
        sourceModeInfoPtr->Format.Graphics.PrimSurfSize.cx = this->dxgkDisplayInfo.Width;
        sourceModeInfoPtr->Format.Graphics.PrimSurfSize.cy = this->dxgkDisplayInfo.Height;
        sourceModeInfoPtr->Format.Graphics.VisibleRegionSize = sourceModeInfoPtr->Format.Graphics.PrimSurfSize;
        sourceModeInfoPtr->Format.Graphics.Stride = this->dxgkDisplayInfo.Pitch;
        sourceModeInfoPtr->Format.Graphics.PixelFormat = D3DDDIFMT_A8R8G8B8;
        sourceModeInfoPtr->Format.Graphics.ColorBasis = D3DKMDT_CB_SCRGB;
        sourceModeInfoPtr->Format.Graphics.PixelValueAccessMode = D3DKMDT_PVAM_DIRECT;

        ROS_LOG_TRACE(
            "Adding source mode. (PrimSurfSize = %d,%d, Stride = %d, PixelFormat = %d, ColorBasis = %d, PixelValueAccessMode = %d)",
            sourceModeInfoPtr->Format.Graphics.PrimSurfSize.cx,
            sourceModeInfoPtr->Format.Graphics.PrimSurfSize.cy,
            sourceModeInfoPtr->Format.Graphics.Stride,
            sourceModeInfoPtr->Format.Graphics.PixelFormat,
            sourceModeInfoPtr->Format.Graphics.ColorBasis,
            sourceModeInfoPtr->Format.Graphics.PixelValueAccessMode);

        // Add mode to the source mode set
        status = smsInterfacePtr->pfnAddMode(
                sourceModeSetHandle,
                sourceModeInfoPtr);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "DXGK_VIDPNSOURCEMODESET_INTERFACE::pfnAddMode() failed. (status = %!STATUS!)",
                status);
            return status;
        }
        releaseModeInfo.DoNot();
    } // source mode info

    // Assign source mode set to source
    status = VidPnInterfacePtr->pfnAssignSourceModeSet(
            VidPnHandle,
            SourceId,
            sourceModeSetHandle);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPN_INTERFACE::pfnAssignSourceModeSet() failed. (status = %!STATUS!)",
            status);
        return status;
    }
    releaseSms.DoNot();

    return STATUS_SUCCESS;
}

//
// Returns STATUS_SUCCESS if the source has a pinned mode, or STATUS_NOT_FOUND
// if the target does not have a pinned mode.
//
_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::TargetHasPinnedMode (
    D3DKMDT_HVIDPN VidPnHandle,
    const DXGK_VIDPN_INTERFACE* VidPnInterfacePtr,
    D3DKMDT_VIDEO_PRESENT_TARGET_MODE_ID TargetId
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // Get the source mode set for this SourceId
    D3DKMDT_HVIDPNTARGETMODESET targetModeSetHandle;
    const DXGK_VIDPNTARGETMODESET_INTERFACE* tmsInterfacePtr;
    NTSTATUS status = VidPnInterfacePtr->pfnAcquireTargetModeSet(
            VidPnHandle,
            TargetId,
            &targetModeSetHandle,
            &tmsInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPN_INTERFACE::pfnAcquireTargetModeSet() failed. (status = %!STATUS!)",
            status);
        return Vc4SanitizeNtstatus(status);
    }
    NT_ASSERT(targetModeSetHandle);
    auto releaseTargetModeSet = VC4_FINALLY::Do([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = VidPnInterfacePtr->pfnReleaseTargetModeSet(
                VidPnHandle,
                targetModeSetHandle);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    // Get the pinned mode info for this source info set
    const D3DKMDT_VIDPN_TARGET_MODE* pinnedTargetModeInfoPtr;
    status = tmsInterfacePtr->pfnAcquirePinnedModeInfo(
            targetModeSetHandle,
            &pinnedTargetModeInfoPtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPNTARGETMODESET_INTERFACE::pfnAcquirePinnedModeInfo() failed. (status = %!STATUS!, targetModeSetHandle = %p)",
            status,
            targetModeSetHandle);
        return Vc4SanitizeNtstatus(status);
    }

    if (status != STATUS_SUCCESS) {
        NT_ASSERT(!pinnedTargetModeInfoPtr);
        return STATUS_NOT_FOUND;
    }

    auto releaseModeInfo = VC4_FINALLY::Do([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = tmsInterfacePtr->pfnReleaseModeInfo(
                targetModeSetHandle,
                pinnedTargetModeInfoPtr);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    return STATUS_SUCCESS;
}

//
// Creates a new target mode set, adds a mode that is compatible with the
// connected monitor.
//
_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::CreateAndAssignTargetModeSet (
    D3DKMDT_HVIDPN VidPnHandle,
    const DXGK_VIDPN_INTERFACE* VidPnInterfacePtr,
    D3DKMDT_VIDEO_PRESENT_SOURCE_MODE_ID /*SourceId*/,
    D3DKMDT_VIDEO_PRESENT_TARGET_MODE_ID TargetId
    ) const
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    D3DKMDT_HVIDPNTARGETMODESET targetModeSetHandle;
    const DXGK_VIDPNTARGETMODESET_INTERFACE* tmsInterfacePtr;
    NTSTATUS status = VidPnInterfacePtr->pfnCreateNewTargetModeSet(
            VidPnHandle,
            TargetId,
            &targetModeSetHandle,
            &tmsInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPN_INTERFACE::pfnCreateNewTargetModeSet failed. (status = %!STATUS!)",
            status);
        return status;
    }
    NT_ASSERT(targetModeSetHandle);
    auto releaseSms = VC4_FINALLY::DoUnless([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus = VidPnInterfacePtr->pfnReleaseTargetModeSet(
                VidPnHandle,
                targetModeSetHandle);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    // Create new mode info and add it to the target mode set
    {
        D3DKMDT_VIDPN_TARGET_MODE* targetModeInfoPtr;
        status = tmsInterfacePtr->pfnCreateNewModeInfo(
                targetModeSetHandle,
                &targetModeInfoPtr);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "DXGK_VIDPNTARGETMODESET_INTERFACE::pfnCreateNewModeInfo() failed. (status = %!STATUS!)",
                status);
            return status;
        }
        NT_ASSERT(targetModeInfoPtr);
        auto releaseModeInfo = VC4_FINALLY::DoUnless([&] () {
            PAGED_CODE();
            NTSTATUS releaseStatus = tmsInterfacePtr->pfnReleaseModeInfo(
                    targetModeSetHandle,
                    targetModeInfoPtr);
            UNREFERENCED_PARAMETER(releaseStatus);
            NT_ASSERT(NT_SUCCESS(releaseStatus));
        });

        // Report the same video signal info that we reported in RecommendMonitorModes
        targetModeInfoPtr->Id = TargetId;
        targetModeInfoPtr->VideoSignalInfo = this->dxgkVideoSignalInfo;
        targetModeInfoPtr->Preference = D3DKMDT_MP_PREFERRED;

        // Add mode to the target mode set
        status = tmsInterfacePtr->pfnAddMode(
                targetModeSetHandle,
                targetModeInfoPtr);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "DXGK_VIDPNTARGETMODESET_INTERFACE::pfnAddMode() failed. (status = %!STATUS!)",
                status);
            return status;
        }
        releaseModeInfo.DoNot();
    } // target mode info

    // Assign target mode set to target
    status = VidPnInterfacePtr->pfnAssignTargetModeSet(
            VidPnHandle,
            TargetId,
            targetModeSetHandle);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "DXGK_VIDPN_INTERFACE::pfnAssignTargetModeSet() failed. (status = %!STATUS!)",
            status);
        return status;
    }
    releaseSms.DoNot();

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::IsVidPnSourceModeFieldsValid (
    const D3DKMDT_VIDPN_SOURCE_MODE* SourceModePtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    if (SourceModePtr->Type != D3DKMDT_RMT_GRAPHICS) {
        ROS_LOG_ERROR(
            "Pinned source mode is not a graphics mode. (SourceModePtr->Type = %d)",
            SourceModePtr->Type);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }

    if ((SourceModePtr->Format.Graphics.ColorBasis != D3DKMDT_CB_SCRGB) &&
        (SourceModePtr->Format.Graphics.ColorBasis != D3DKMDT_CB_UNINITIALIZED))
    {
        ROS_LOG_ERROR(
            "Pinned source mode has a non-linear RGB color basis (ColorBasis = %d)",
            SourceModePtr->Format.Graphics.ColorBasis);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }

    if (SourceModePtr->Format.Graphics.PixelValueAccessMode != D3DKMDT_PVAM_DIRECT) {
        ROS_LOG_ERROR(
            "Pinned source mode has a palettized access mode. (PixelValueAccessMode = %d)",
            SourceModePtr->Format.Graphics.PixelValueAccessMode);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }

    if (SourceModePtr->Format.Graphics.PixelFormat != D3DDDIFMT_A8R8G8B8) {
        ROS_LOG_ERROR(
            "Pinned source mode has invalid pixel format. (SourceModePtr->Format.Graphics.PixelFormat = %d, D3DDDIFMT_A8R8G8B8 = %d)",
            SourceModePtr->Format.Graphics.PixelFormat,
            D3DDDIFMT_A8R8G8B8);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS VC4_DISPLAY::IsVidPnPathFieldsValid (
    const D3DKMDT_VIDPN_PRESENT_PATH* PathPtr
    )
{
    PAGED_CODE();
    VC4_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    if (PathPtr->VidPnSourceId != 0) {
        ROS_LOG_ERROR(
            "Path contains invalid source id. (VidPnSourceId = %d)",
            PathPtr->VidPnSourceId);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE;
    }

    if (PathPtr->VidPnTargetId != 0) {
        ROS_LOG_ERROR(
            "Path contains invalid target id. (VidPnTargetId = %d)",
            PathPtr->VidPnTargetId);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_TARGET;
    }

    if (PathPtr->GammaRamp.Type != D3DDDI_GAMMARAMP_DEFAULT)
    {
        ROS_LOG_ERROR(
            "Path contains a gamma ramp. (GammaRamp.Type = %d)",
            PathPtr->GammaRamp.Type);
        return STATUS_GRAPHICS_GAMMA_RAMP_NOT_SUPPORTED;
    }

    if ((PathPtr->ContentTransformation.Scaling != D3DKMDT_VPPS_IDENTITY) &&
        (PathPtr->ContentTransformation.Scaling != D3DKMDT_VPPS_NOTSPECIFIED) &&
        (PathPtr->ContentTransformation.Scaling != D3DKMDT_VPPS_UNINITIALIZED))
    {
        ROS_LOG_ERROR(
            "Path contains a non-identity scaling. (ContentTransformation.Scaling = %d)",
            PathPtr->ContentTransformation.Scaling);
        return STATUS_GRAPHICS_VIDPN_MODALITY_NOT_SUPPORTED;
    }

    if ((PathPtr->ContentTransformation.Rotation != D3DKMDT_VPPR_IDENTITY) &&
        (PathPtr->ContentTransformation.Rotation != D3DKMDT_VPPR_NOTSPECIFIED) &&
        (PathPtr->ContentTransformation.Rotation != D3DKMDT_VPPR_UNINITIALIZED))
    {
        ROS_LOG_ERROR(
            "Path contains a not-identity rotation (ContentTransformation.Rotation = %d)",
            PathPtr->ContentTransformation.Rotation);
        return STATUS_GRAPHICS_VIDPN_MODALITY_NOT_SUPPORTED;
    }

    if ((PathPtr->VidPnTargetColorBasis != D3DKMDT_CB_SCRGB) &&
        (PathPtr->VidPnTargetColorBasis != D3DKMDT_CB_UNINITIALIZED))
    {
        ROS_LOG_ERROR(
            "Path has a non-linear RGB color basis. (VidPnTargetColorBasis = %d)",
            PathPtr->VidPnTargetColorBasis);
        return STATUS_GRAPHICS_INVALID_VIDEO_PRESENT_SOURCE_MODE;
    }

    return STATUS_SUCCESS;
}

VC4_PAGED_SEGMENT_END; //=====================================================
