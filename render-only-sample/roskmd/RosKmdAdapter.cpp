
#include "precomp.h"

#include "RosKmdLogging.h"
#include "RosKmdAdapter.tmh"

#include "RosKmd.h"
#include "RosKmdAdapter.h"
#include "RosKmdRapAdapter.h"
#include "RosKmdSoftAdapter.h"
#include "RosKmdAllocation.h"
#include "RosKmdContext.h"
#include "RosKmdResource.h"
#include "RosKmdGlobal.h"
#include "RosKmdUtil.h"
#include "RosGpuCommand.h"
#include "RosKmdAcpi.h"
#include "RosKmdUtil.h"
#include "Vc4Hw.h"
#include "Vc4Ddi.h"
#include "Vc4Mailbox.h"

void * RosKmAdapter::operator new(size_t size)
{
    return ExAllocatePoolWithTag(NonPagedPoolNx, size, 'ROSD');
}

void RosKmAdapter::operator delete(void * ptr)
{
    ExFreePool(ptr);
}

RosKmAdapter::RosKmAdapter(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext) :
    m_display(PhysicalDeviceObject, m_DxgkInterface, m_DxgkStartInfo, m_deviceInfo)
{
    m_magic = kMagic;
    m_pPhysicalDevice = PhysicalDeviceObject;

    // Enable in RosKmAdapter::Start() when device is ready for interrupt
    m_bReadyToHandleInterrupt = FALSE;

    // Set initial power management state.
    m_PowerManagementStarted = FALSE;
    m_AdapterPowerDState = PowerDeviceD0; // Device is at D0 at startup
    m_NumPowerComponents = 0;
    RtlZeroMemory(&m_EnginePowerFState[0], sizeof(m_EnginePowerFState)); // Components are F0 at startup.

    RtlZeroMemory(&m_deviceId, sizeof(m_deviceId));
    m_deviceIdLength = 0;

    m_flags.m_value = 0;

#if VC4

#if GPU_CACHE_WORKAROUND

    m_rtSizeJitter = 0;

#endif

    m_busAddressOffset = 0;

#endif

    *MiniportDeviceContext = this;
}

RosKmAdapter::~RosKmAdapter()
{
    // do nothing
}

NTSTATUS
RosKmAdapter::AddAdapter(
    IN_CONST_PDEVICE_OBJECT     PhysicalDeviceObject,
    OUT_PPVOID                  MiniportDeviceContext)
{
    NTSTATUS status;
    WCHAR deviceID[512];
    ULONG dataLen;

    status = IoGetDeviceProperty(PhysicalDeviceObject, DevicePropertyHardwareID, sizeof(deviceID), deviceID, &dataLen);
    if (!NT_SUCCESS(status))
    {
        ROS_LOG_ERROR(
            "Failed to get DevicePropertyHardwareID from PDO. (status=%!STATUS!)",
            status);
        return status;
    }

    RosKmAdapter  *pRosKmAdapter = nullptr;
    if (wcscmp(deviceID, L"ACPI\\VEN_BCM&DEV_2850") == 0)
    {
        pRosKmAdapter = new RosKmdRapAdapter(PhysicalDeviceObject, MiniportDeviceContext);
        if (!pRosKmAdapter) {
            ROS_LOG_LOW_MEMORY("Failed to allocate RosKmdRapAdapter.");
            return STATUS_NO_MEMORY;
        }
    }
    else
    {
        pRosKmAdapter = new RosKmdSoftAdapter(PhysicalDeviceObject, MiniportDeviceContext);
        if (!pRosKmAdapter) {
            ROS_LOG_LOW_MEMORY("Failed to allocate RosKmdSoftAdapter.");
            return STATUS_NO_MEMORY;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::QueryEngineStatus(
    DXGKARG_QUERYENGINESTATUS  *pQueryEngineStatus)
{
    ROS_LOG_TRACE("QueryEngineStatus was called.");

    pQueryEngineStatus->EngineStatus.Responsive = 1;
    return STATUS_SUCCESS;
}

void RosKmAdapter::WorkerThread(void * inThis)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(inThis);

    pRosKmAdapter->DoWork();
}

void RosKmAdapter::DoWork(void)
{
    bool done = false;

    while (!done)
    {
        NTSTATUS status = KeWaitForSingleObject(
            &m_workerThreadEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL);

        status;
        NT_ASSERT(status == STATUS_SUCCESS);

        if (m_workerExit)
        {
            done = true;
            continue;
        }

        for (;;)
        {
            ROSDMABUFSUBMISSION *   pDmaBufSubmission = DequeueDmaBuffer(&m_dmaBufQueueLock);
            if (pDmaBufSubmission == NULL)
            {
                break;
            }

            ROSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

            if (pDmaBufInfo->m_DmaBufState.m_bPaging)
            {
                //
                // Run paging buffer in software
                //

                ProcessPagingBuffer(pDmaBufSubmission);

                NotifyDmaBufCompletion(pDmaBufSubmission);
            }
            else
            {
                //
                // Process render DMA buffer
                //

                ProcessRenderBuffer(pDmaBufSubmission);

                NotifyDmaBufCompletion(pDmaBufSubmission);
            }

            ExInterlockedInsertTailList(&m_dmaBufSubmissionFree, &pDmaBufSubmission->m_QueueEntry, &m_dmaBufQueueLock);
        }
    }
}

ROSDMABUFSUBMISSION *
RosKmAdapter::DequeueDmaBuffer(
    KSPIN_LOCK *pDmaBufQueueLock)
{
    LIST_ENTRY *pDmaEntry;

    if (pDmaBufQueueLock)
    {
        pDmaEntry = ExInterlockedRemoveHeadList(&m_dmaBufQueue, pDmaBufQueueLock);
    }
    else
    {
        if (!IsListEmpty(&m_dmaBufQueue))
        {
            pDmaEntry = RemoveHeadList(&m_dmaBufQueue);
        }
        else
        {
            pDmaEntry = NULL;
        }
    }

    return CONTAINING_RECORD(pDmaEntry, ROSDMABUFSUBMISSION, m_QueueEntry);
}

void
RosKmAdapter::ProcessPagingBuffer(
    ROSDMABUFSUBMISSION * pDmaBufSubmission)
{
    ROSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

    NT_ASSERT(0 == (pDmaBufSubmission->m_EndOffset - pDmaBufSubmission->m_StartOffset) % sizeof(DXGKARG_BUILDPAGINGBUFFER));

    DXGKARG_BUILDPAGINGBUFFER * pPagingBuffer = (DXGKARG_BUILDPAGINGBUFFER *)(pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_StartOffset);
    DXGKARG_BUILDPAGINGBUFFER * pEndofBuffer = (DXGKARG_BUILDPAGINGBUFFER *)(pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_EndOffset);

    for (; pPagingBuffer < pEndofBuffer; pPagingBuffer++)
    {
        switch (pPagingBuffer->Operation)
        {
        case DXGK_OPERATION_FILL:
        {
            NT_ASSERT(pPagingBuffer->Fill.Destination.SegmentId == ROSD_SEGMENT_VIDEO_MEMORY);
            NT_ASSERT(pPagingBuffer->Fill.FillSize % sizeof(ULONG) == 0);

            ULONG * const startAddress = reinterpret_cast<ULONG*>(
                (BYTE *)RosKmdGlobal::s_pVideoMemory +
                pPagingBuffer->Fill.Destination.SegmentAddress.QuadPart);
            for (ULONG * ptr = startAddress;
                 ptr != (startAddress + pPagingBuffer->Fill.FillSize / sizeof(ULONG));
                 ++ptr)
            {
                *ptr = pPagingBuffer->Fill.FillPattern;
            }
        }
        break;
        case DXGK_OPERATION_TRANSFER:
        {
            PBYTE   pSource, pDestination;
            MDL *   pMdlToRestore = NULL;
            CSHORT  savedMdlFlags = 0;
            PBYTE   pKmAddrToUnmap = NULL;

            if (pPagingBuffer->Transfer.Source.SegmentId == ROSD_SEGMENT_VIDEO_MEMORY)
            {
                pSource = ((BYTE *)RosKmdGlobal::s_pVideoMemory) + pPagingBuffer->Transfer.Source.SegmentAddress.QuadPart;
            }
            else
            {
                NT_ASSERT(pPagingBuffer->Transfer.Source.SegmentId == 0);

                pMdlToRestore = pPagingBuffer->Transfer.Source.pMdl;
                savedMdlFlags = pMdlToRestore->MdlFlags;

                pSource = (PBYTE)MmGetSystemAddressForMdlSafe(pPagingBuffer->Transfer.Source.pMdl, HighPagePriority);

                pKmAddrToUnmap = pSource;

                // Adjust the source address by MdlOffset
                pSource += (pPagingBuffer->Transfer.MdlOffset*PAGE_SIZE);
            }

            if (pPagingBuffer->Transfer.Destination.SegmentId == ROSD_SEGMENT_VIDEO_MEMORY)
            {
                pDestination = ((BYTE *)RosKmdGlobal::s_pVideoMemory) + pPagingBuffer->Transfer.Destination.SegmentAddress.QuadPart;
            }
            else
            {
                NT_ASSERT(pPagingBuffer->Transfer.Destination.SegmentId == 0);

                pMdlToRestore = pPagingBuffer->Transfer.Destination.pMdl;
                savedMdlFlags = pMdlToRestore->MdlFlags;

                pDestination = (PBYTE)MmGetSystemAddressForMdlSafe(pPagingBuffer->Transfer.Destination.pMdl, HighPagePriority);

                pKmAddrToUnmap = pDestination;

                // Adjust the destination address by MdlOffset
                pDestination += (pPagingBuffer->Transfer.MdlOffset*PAGE_SIZE);
            }

            if (pSource && pDestination)
            {
                RtlCopyMemory(pDestination, pSource, pPagingBuffer->Transfer.TransferSize);
            }
            else
            {
                // TODO[indyz]: Propagate the error back to runtime
                m_ErrorHit.m_PagingFailure = 1;
            }

            // Restore the state of the Mdl (for source or destionation)
            if ((0 == (savedMdlFlags & MDL_MAPPED_TO_SYSTEM_VA)) && pKmAddrToUnmap)
            {
                MmUnmapLockedPages(pKmAddrToUnmap, pMdlToRestore);
            }
        }
        break;

        default:
            NT_ASSERT(false);
        }
    }
}

void
RosKmAdapter::NotifyDmaBufCompletion(
    ROSDMABUFSUBMISSION * pDmaBufSubmission)
{
    ROSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

    if (! pDmaBufInfo->m_DmaBufState.m_bPaging)
    {
        pDmaBufInfo->m_DmaBufState.m_bCompleted = 1;
    }

    //
    // Notify the VidSch of the completion of the DMA buffer
    //
    NTSTATUS    Status;

    RtlZeroMemory(&m_interruptData, sizeof(m_interruptData));

    m_interruptData.InterruptType = DXGK_INTERRUPT_DMA_COMPLETED;
    m_interruptData.DmaCompleted.SubmissionFenceId = pDmaBufSubmission->m_SubmissionFenceId;
    m_interruptData.DmaCompleted.NodeOrdinal = 0;
    m_interruptData.DmaCompleted.EngineOrdinal = 0;

    BOOLEAN bRet;

    Status = m_DxgkInterface.DxgkCbSynchronizeExecution(
        m_DxgkInterface.DeviceHandle,
        SynchronizeNotifyInterrupt,
        this,
        0,
        &bRet);

    if (!NT_SUCCESS(Status))
    {
        m_ErrorHit.m_NotifyDmaBufCompletion = 1;
    }
}

BOOLEAN RosKmAdapter::SynchronizeNotifyInterrupt(PVOID inThis)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(inThis);

    return pRosKmAdapter->SynchronizeNotifyInterrupt();
}

BOOLEAN RosKmAdapter::SynchronizeNotifyInterrupt(void)
{
    m_DxgkInterface.DxgkCbNotifyInterrupt(m_DxgkInterface.DeviceHandle, &m_interruptData);

    return m_DxgkInterface.DxgkCbQueueDpc(m_DxgkInterface.DeviceHandle);
}

NTSTATUS
RosKmAdapter::Start(
    IN_PDXGK_START_INFO     DxgkStartInfo,
    IN_PDXGKRNL_INTERFACE   DxgkInterface,
    OUT_PULONG              NumberOfVideoPresentSources,
    OUT_PULONG              NumberOfChildren)
{
    m_DxgkStartInfo = *DxgkStartInfo;
    m_DxgkInterface = *DxgkInterface;

    //
    // Render only device has no VidPn source and target
    // Subclass should overwrite these values if it is not render-only.
    //
    *NumberOfVideoPresentSources = 0;
    *NumberOfChildren = 0;

    //
    // Sample for 1.3 model currently
    //
    m_WDDMVersion = DXGKDDI_WDDMv1_3;

    m_NumNodes = C_ROSD_GPU_ENGINE_COUNT;

    //
    // Initialize worker
    //

    KeInitializeEvent(&m_workerThreadEvent, SynchronizationEvent, FALSE);

    m_workerExit = false;

    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              hWorkerThread;

    InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

    NTSTATUS status = PsCreateSystemThread(
        &hWorkerThread,
        THREAD_ALL_ACCESS,
        &ObjectAttributes,
        NULL,
        NULL,
        (PKSTART_ROUTINE) RosKmAdapter::WorkerThread,
        this);

    if (status != STATUS_SUCCESS)
    {
        ROS_LOG_ERROR(
            "PsCreateSystemThread(...) failed for RosKmAdapter::WorkerThread. (status=%!STATUS!)",
            status);
        return status;
    }

    status = ObReferenceObjectByHandle(
        hWorkerThread,
        THREAD_ALL_ACCESS,
        *PsThreadType,
        KernelMode,
        (PVOID *)&m_pWorkerThread,
        NULL);

    ZwClose(hWorkerThread);

    if (!NT_SUCCESS(status))
    {
        ROS_LOG_ERROR(
            "ObReferenceObjectByHandle(...) failed for worker thread. (status=%!STATUS!)",
            status);
        return status;
    }

    status = m_DxgkInterface.DxgkCbGetDeviceInformation(
        m_DxgkInterface.DeviceHandle,
        &m_deviceInfo);
    if (!NT_SUCCESS(status))
    {
        ROS_LOG_ERROR(
            "DxgkCbGetDeviceInformation(...) failed. (status=%!STATUS!, m_DxgkInterface.DeviceHandle=0x%p)",
            status,
            m_DxgkInterface.DeviceHandle);
        return status;
    }

    //
    // Query APCI device ID
    //
    {
        NTSTATUS acpiStatus;

        RosKmAcpiReader acpiReader(this, DISPLAY_ADAPTER_HW_ID);
        acpiStatus = acpiReader.Read(ACPI_METHOD_HARDWARE_ID);
        if (NT_SUCCESS(acpiStatus) && (acpiReader.GetOutputArgumentCount() == 1))
        {
            RosKmAcpiArgumentParser acpiParser(&acpiReader, NULL);
            char *pDeviceId;
            ULONG DeviceIdLength;
            acpiStatus = acpiParser.GetAnsiString(&pDeviceId, &DeviceIdLength);
            if (NT_SUCCESS(acpiStatus) && DeviceIdLength)
            {
                m_deviceIdLength = min(DeviceIdLength, sizeof(m_deviceId));
                RtlCopyMemory(&m_deviceId[0], pDeviceId, m_deviceIdLength);
            }
        }
    }

    //
    // Initialize power component data.
    //
    InitializePowerComponentInfo();

    //
    // Initialize apperture state
    //

    memset(m_aperturePageTable, 0, sizeof(m_aperturePageTable));

    //
    // Intialize DMA buffer queue and lock
    //

    InitializeListHead(&m_dmaBufSubmissionFree);
    for (UINT i = 0; i < m_maxDmaBufQueueLength; i++)
    {
        InsertHeadList(&m_dmaBufSubmissionFree, &m_dmaBufSubssions[i].m_QueueEntry);
    }

    InitializeListHead(&m_dmaBufQueue);
    KeInitializeSpinLock(&m_dmaBufQueueLock);

    //
    // Initialize HW DMA buffer compeletion DPC and event
    //

    KeInitializeEvent(&m_hwDmaBufCompletionEvent, SynchronizationEvent, FALSE);
    KeInitializeDpc(&m_hwDmaBufCompletionDpc, HwDmaBufCompletionDpcRoutine, this);

    ROS_LOG_TRACE("Adapter was successfully started.");
    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::Stop()
{
    m_workerExit = true;

    KeSetEvent(&m_workerThreadEvent, 0, FALSE);

    NTSTATUS status = KeWaitForSingleObject(
        m_pWorkerThread,
        Executive,
        KernelMode,
        FALSE,
        NULL);

    status;
    NT_ASSERT(status == STATUS_SUCCESS);

    ObDereferenceObject(m_pWorkerThread);

    ROS_LOG_TRACE("Adapter was successfully stopped.");
    return STATUS_SUCCESS;
}

void RosKmAdapter::DpcRoutine(void)
{
    // dp nothing other than calling back into dxgk

    m_DxgkInterface.DxgkCbNotifyDpc(m_DxgkInterface.DeviceHandle);
}

NTSTATUS
RosKmAdapter::BuildPagingBuffer(
    IN_PDXGKARG_BUILDPAGINGBUFFER   pArgs)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PBYTE       pDmaBufStart = (PBYTE)pArgs->pDmaBuffer;
    PBYTE       pDmaBufPos = (PBYTE)pArgs->pDmaBuffer;

    //
    // hAllocation is NULL for operation on DMA buffer and pages mapped into aperture
    //

    //
    // If there is insufficient space left in DMA buffer, we should return
    // STATUS_GRAPHICS_INSUFFICIENT_DMA_BUFFER.
    //

    switch (pArgs->Operation)
    {
    case DXGK_OPERATION_MAP_APERTURE_SEGMENT:
    {
        if (pArgs->MapApertureSegment.SegmentId == kApertureSegmentId)
        {
            size_t pageIndex = pArgs->MapApertureSegment.OffsetInPages;
            size_t pageCount = pArgs->MapApertureSegment.NumberOfPages;

            NT_ASSERT(pageIndex + pageCount <= kApertureSegmentPageCount);

            size_t mdlPageOffset = pArgs->MapApertureSegment.MdlOffset;

            PMDL pMdl = pArgs->MapApertureSegment.pMdl;

            for (UINT i = 0; i < pageCount; i++)
            {
                m_aperturePageTable[pageIndex + i] = MmGetMdlPfnArray(pMdl)[mdlPageOffset + i];
            }
        }

    }
    break;

    case DXGK_OPERATION_UNMAP_APERTURE_SEGMENT:
    {
        if (pArgs->MapApertureSegment.SegmentId == kApertureSegmentId)
        {
            size_t pageIndex = pArgs->MapApertureSegment.OffsetInPages;
            size_t pageCount = pArgs->MapApertureSegment.NumberOfPages;

            NT_ASSERT(pageIndex + pageCount <= kApertureSegmentPageCount);

            while (pageCount--)
            {
                m_aperturePageTable[pageIndex++] = 0;
            }
        }
    }

    break;

    case DXGK_OPERATION_FILL:
    {
        RosKmdAllocation * pRosKmdAllocation = (RosKmdAllocation *)pArgs->Fill.hAllocation;
        pRosKmdAllocation;

        ROS_LOG_TRACE(
            "Filling DMA buffer. (Destination.SegmentAddress=0x%I64x, FillPattern=0x%lx, FillSize=%Id)",
            pArgs->Fill.Destination.SegmentAddress.QuadPart,
            pArgs->Fill.FillPattern,
            pArgs->Fill.FillSize);

        if (pArgs->DmaSize < sizeof(DXGKARG_BUILDPAGINGBUFFER))
        {
            ROS_LOG_ERROR(
                "DXGK_OPERATION_FILL: DMA buffer size is too small. (pArgs->DmaSize=%d, sizeof(DXGKARG_BUILDPAGINGBUFFER)=%d)",
                pArgs->DmaSize,
                sizeof(DXGKARG_BUILDPAGINGBUFFER));
            return STATUS_GRAPHICS_INSUFFICIENT_DMA_BUFFER;
        }
        else
        {
            *((DXGKARG_BUILDPAGINGBUFFER *)pArgs->pDmaBuffer) = *pArgs;

            pDmaBufPos += sizeof(DXGKARG_BUILDPAGINGBUFFER);
        }
    }
    break;

    case DXGK_OPERATION_DISCARD_CONTENT:
    {
        // do nothing
    }
    break;

    case DXGK_OPERATION_TRANSFER:
    {
        if (pArgs->DmaSize < sizeof(DXGKARG_BUILDPAGINGBUFFER))
        {
            ROS_LOG_ERROR(
                "DXGK_OPERATION_TRANSFER: DMA buffer is too small. (pArgs->DmaSize=%d, sizeof(DXGKARG_BUILDPAGINGBUFFER)=%d)",
                pArgs->DmaSize,
                sizeof(DXGKARG_BUILDPAGINGBUFFER));
            return STATUS_GRAPHICS_INSUFFICIENT_DMA_BUFFER;
        }
        else
        {
            *((DXGKARG_BUILDPAGINGBUFFER *)pArgs->pDmaBuffer) = *pArgs;

            pDmaBufPos += sizeof(DXGKARG_BUILDPAGINGBUFFER);
        }
    }
    break;

    default:
    {
        NT_ASSERT(false);

        m_ErrorHit.m_UnSupportedPagingOp = 1;
        Status = STATUS_SUCCESS;
    }
    break;
    }

    //
    // Update pDmaBuffer to point past the last byte used.
    pArgs->pDmaBuffer = pDmaBufPos;

    // Record DMA buffer information only when it is newly used
    ROSDMABUFINFO * pDmaBufInfo = (ROSDMABUFINFO *)pArgs->pDmaBufferPrivateData;
    if (pDmaBufInfo && (pArgs->DmaSize == ROSD_PAGING_BUFFER_SIZE))
    {
        pDmaBufInfo->m_DmaBufState.m_Value = 0;
        pDmaBufInfo->m_DmaBufState.m_bPaging = 1;

        pDmaBufInfo->m_pDmaBuffer = pDmaBufStart;
        pDmaBufInfo->m_DmaBufferSize = pArgs->DmaSize;
    }

    return Status;
}

NTSTATUS
RosKmAdapter::DispatchIoRequest(
    IN_ULONG                    VidPnSourceId,
    IN_PVIDEO_REQUEST_PACKET    VideoRequestPacket)
{
    if (RosKmdGlobal::IsRenderOnly())
    {
        ROS_LOG_WARNING(
            "Unsupported IO Control Code. (VideoRequestPacketPtr->IoControlCode = 0x%lx)",
            VideoRequestPacket->IoControlCode);
        return STATUS_NOT_SUPPORTED;
    }

    return m_display.DispatchIoRequest(VidPnSourceId, VideoRequestPacket);
}

NTSTATUS
RosKmAdapter::SubmitCommand(
    IN_CONST_PDXGKARG_SUBMITCOMMAND     pSubmitCommand)
{
    NTSTATUS        Status = STATUS_SUCCESS;

#if VC4

    if (!pSubmitCommand->Flags.Paging)
    {
        //
        // Patch DMA buffer self-reference
        //
        ROSDMABUFINFO  *pDmaBufInfo = (ROSDMABUFINFO *)pSubmitCommand->pDmaBufferPrivateData;
        BYTE           *pDmaBuf = pDmaBufInfo->m_pDmaBuffer;
        UINT            dmaBufPhysicalAddress;

        //
        // Need to record DMA buffer physical address for fully pre-patched DMA buffer
        //
        pDmaBufInfo->m_DmaBufferPhysicalAddress = pSubmitCommand->DmaBufferPhysicalAddress;

        dmaBufPhysicalAddress = GetAperturePhysicalAddress(
            pSubmitCommand->DmaBufferPhysicalAddress.LowPart);

        for (UINT i = 0; i < pDmaBufInfo->m_DmaBufState.m_NumDmaBufSelfRef; i++)
        {
            D3DDDI_PATCHLOCATIONLIST   *pPatchLoc = &pDmaBufInfo->m_DmaBufSelfRef[i];

            *((UINT *)(pDmaBuf + pPatchLoc->PatchOffset)) =
                dmaBufPhysicalAddress +
                m_busAddressOffset +
                pPatchLoc->AllocationOffset;
        }
    }

#endif

    // NOTE: pRosKmContext will be NULL for paging operations
    RosKmContext *pRosKmContext = (RosKmContext *)pSubmitCommand->hContext;
    pRosKmContext;

    QueueDmaBuffer(pSubmitCommand);

    //
    // Wake up the worker thread for the GPU node
    //
    KeSetEvent(&m_workerThreadEvent, 0, FALSE);

    return Status;
}

NTSTATUS
RosKmAdapter::Patch(
    IN_CONST_PDXGKARG_PATCH     pPatch)
{
    ROSDMABUFINFO *pDmaBufInfo = (ROSDMABUFINFO *)pPatch->pDmaBufferPrivateData;

    RosKmContext * pRosKmContext = (RosKmContext *)pPatch->hContext;
    pRosKmContext;

    pDmaBufInfo->m_DmaBufferPhysicalAddress = pPatch->DmaBufferPhysicalAddress;

    PatchDmaBuffer(
        pDmaBufInfo,
        pPatch->pAllocationList,
        pPatch->AllocationListSize,
        pPatch->pPatchLocationList + pPatch->PatchLocationListSubmissionStart,
        pPatch->PatchLocationListSubmissionLength);

    // Record DMA buffer information
    pDmaBufInfo->m_DmaBufState.m_bPatched = 1;

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::CreateAllocation(
    INOUT_PDXGKARG_CREATEALLOCATION     pCreateAllocation)
{
    NT_ASSERT(pCreateAllocation->PrivateDriverDataSize == sizeof(RosAllocationGroupExchange));
    RosAllocationGroupExchange * pRosAllocationGroupExchange = (RosAllocationGroupExchange *)pCreateAllocation->pPrivateDriverData;

    pRosAllocationGroupExchange;
    NT_ASSERT(pRosAllocationGroupExchange->m_dummy == 0);

    RosKmdResource * pRosKmdResource = NULL;

    if (pCreateAllocation->Flags.Resource)
    {
        if (pCreateAllocation->hResource == NULL)
        {
            pRosKmdResource = (RosKmdResource *)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(RosKmdResource), 'ROSD');
            if (!pRosKmdResource)
            {
                ROS_LOG_LOW_MEMORY(
                    "Failed to allocate nonpaged pool for sizeof(RosKmdResource) structure. (sizeof(RosKmdResource)=%d)",
                    sizeof(RosKmdResource));
                return STATUS_NO_MEMORY;
            }
            pRosKmdResource->m_dummy = 0;
        }
        else
        {
            pRosKmdResource = (RosKmdResource *)pCreateAllocation->hResource;
        }
    }

    NT_ASSERT(pCreateAllocation->NumAllocations == 1);

    DXGK_ALLOCATIONINFO * pAllocationInfo = pCreateAllocation->pAllocationInfo;

    NT_ASSERT(pAllocationInfo->PrivateDriverDataSize == sizeof(RosAllocationExchange));
    RosAllocationExchange * pRosAllocation = (RosAllocationExchange *)pAllocationInfo->pPrivateDriverData;

    RosKmdAllocation * pRosKmdAllocation = (RosKmdAllocation *)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(RosKmdAllocation), 'ROSD');
    if (!pRosKmdAllocation)
    {
        if (pRosKmdResource != NULL) ExFreePoolWithTag(pRosKmdResource, 'ROSD');

        ROS_LOG_ERROR(
            "Failed to allocated nonpaged pool for RosKmdAllocation. (sizeof(RosKmdAllocation)=%d)",
            sizeof(RosKmdAllocation));
        return STATUS_NO_MEMORY;
    }

    *(RosAllocationExchange *)pRosKmdAllocation = *pRosAllocation;

    pAllocationInfo->hAllocation = pRosKmdAllocation;

    pAllocationInfo->Alignment = 64;
    pAllocationInfo->AllocationPriority = D3DDDI_ALLOCATIONPRIORITY_NORMAL;
    pAllocationInfo->EvictionSegmentSet = 0; // don't use apperture for eviction

    pAllocationInfo->Flags.Value = 0;

    //
    // Allocations should be marked CPU visible unless they are shared or
    // can be flipped.
    // Shared allocations (including the primary) cannot be CPU visible unless
    // they are exclusively located in an aperture segment.
    //
    pAllocationInfo->Flags.CpuVisible =
        !((pRosAllocation->m_miscFlags & D3D10_DDI_RESOURCE_MISC_SHARED) ||
          (pRosAllocation->m_bindFlags & D3D10_DDI_BIND_PRESENT));

    // Allocations that will be flipped, such as the primary allocation,
    // cannot be cached.
    pAllocationInfo->Flags.Cached = pAllocationInfo->Flags.CpuVisible;

    pAllocationInfo->HintedBank.Value = 0;
    pAllocationInfo->MaximumRenamingListLength = 0;
    pAllocationInfo->pAllocationUsageHint = NULL;
    pAllocationInfo->PhysicalAdapterIndex = 0;
    pAllocationInfo->PitchAlignedSize = 0;
    pAllocationInfo->PreferredSegment.Value = 0;
    pAllocationInfo->PreferredSegment.SegmentId0 = ROSD_SEGMENT_VIDEO_MEMORY;
    pAllocationInfo->PreferredSegment.Direction0 = 0;

    // zero-size allocations are not allowed
    NT_ASSERT(pRosAllocation->m_hwSizeBytes != 0);
    pAllocationInfo->Size = pRosAllocation->m_hwSizeBytes;

    pAllocationInfo->SupportedReadSegmentSet = 1 << (ROSD_SEGMENT_VIDEO_MEMORY - 1);
    pAllocationInfo->SupportedWriteSegmentSet = 1 << (ROSD_SEGMENT_VIDEO_MEMORY - 1);

#if GPU_CACHE_WORKAROUND

    if (pRosAllocation->m_bindFlags & D3D10_DDI_BIND_RENDER_TARGET)
    {
        pAllocationInfo->Size += m_rtSizeJitter;

        //
        // Specific workaround for BasicTests.exe, ensures allocations use
        // new memory range by enlarging the size of the render target
        //

        m_rtSizeJitter += (5*kPageSize);
    }

#endif

    if (pCreateAllocation->Flags.Resource && pCreateAllocation->hResource == NULL && pRosKmdResource != NULL)
    {
        pCreateAllocation->hResource = pRosKmdResource;
    }

    ROS_LOG_TRACE(
        "Created allocation. (Flags.CpuVisible=%d, Flags.Cacheable=%d, Size=%Id)",
        pAllocationInfo->Flags.CpuVisible,
        pAllocationInfo->Flags.Cached,
        pAllocationInfo->Size);

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::DestroyAllocation(
    IN_CONST_PDXGKARG_DESTROYALLOCATION     pDestroyAllocation)
{
    RosKmdResource * pRosKmdResource = NULL;

    if (pDestroyAllocation->Flags.DestroyResource)
    {
        pRosKmdResource = (RosKmdResource *)pDestroyAllocation->hResource;
    }

    NT_ASSERT(pDestroyAllocation->NumAllocations == 1);
    RosKmdAllocation * pRosKmdAllocation = (RosKmdAllocation *)pDestroyAllocation->pAllocationList[0];

    ExFreePoolWithTag(pRosKmdAllocation, 'ROSD');

    if (pRosKmdResource != NULL) ExFreePoolWithTag(pRosKmdResource, 'ROSD');

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::QueryAdapterInfo(
    IN_CONST_PDXGKARG_QUERYADAPTERINFO      pQueryAdapterInfo)
{
    ROS_LOG_TRACE(
        "QueryAdapterInfo was called. (Type=%d)",
        pQueryAdapterInfo->Type);

    switch (pQueryAdapterInfo->Type)
    {
    case DXGKQAITYPE_UMDRIVERPRIVATE:
    {
        if (pQueryAdapterInfo->OutputDataSize < sizeof(ROSADAPTERINFO))
        {
            ROS_LOG_ERROR(
                "Output buffer is too small. (pQueryAdapterInfo->OutputDataSize=%d, sizeof(ROSADAPTERINFO)=%d)",
                pQueryAdapterInfo->OutputDataSize,
                sizeof(ROSADAPTERINFO));
            return STATUS_BUFFER_TOO_SMALL;
        }
        ROSADAPTERINFO* pRosAdapterInfo = (ROSADAPTERINFO*)pQueryAdapterInfo->pOutputData;

        pRosAdapterInfo->m_version = ROSD_VERSION;
        pRosAdapterInfo->m_wddmVersion = m_WDDMVersion;

        // Software APCI device only claims an interrupt resource
        pRosAdapterInfo->m_isSoftwareDevice = (m_flags.m_isVC4 != 1);

        RtlCopyMemory(
            pRosAdapterInfo->m_deviceId,
            m_deviceId,
            m_deviceIdLength);
    }
    break;

    case DXGKQAITYPE_DRIVERCAPS:
    {
        if (pQueryAdapterInfo->OutputDataSize < sizeof(DXGK_DRIVERCAPS))
        {
            ROS_LOG_ASSERTION(
                "Output buffer is too small. (pQueryAdapterInfo->OutputDataSize=%d, sizeof(DXGK_DRIVERCAPS)=%d)",
                pQueryAdapterInfo->OutputDataSize,
                sizeof(DXGK_DRIVERCAPS));
            return STATUS_BUFFER_TOO_SMALL;
        }

        DXGK_DRIVERCAPS    *pDriverCaps = (DXGK_DRIVERCAPS *)pQueryAdapterInfo->pOutputData;

        //
        // HighestAcceptableAddress
        //
        pDriverCaps->HighestAcceptableAddress.QuadPart = -1;

        //
        // TODO[bhouse] MaxAllocationListSlotId
        //

        //
        // TODO[bhouse] ApertureSegmentCommitLimit
        //

        //
        // TODO[bhouse] MaxPointerWidth
        //

        //
        // TODO[bhouse] MaxPointerHeight
        //

        //
        // TODO[bhouse] PointerCaps
        //

        //
        // TODO[bhouse] InterruptMessageNumber
        //

        //
        // TODO[bhouse] NumberOfSwizzlingRanges
        //

        //
        // TODO[bhouse] MaxOverlays
        //

        //
        // TODO[bhouse] GammarRampCaps
        //

        //
        // TODO[bhouse] PresentationCaps
        //

        pDriverCaps->PresentationCaps.SupportKernelModeCommandBuffer = FALSE;
        pDriverCaps->PresentationCaps.SupportSoftwareDeviceBitmaps = TRUE;

        //
        // Cap used for DWM off case, screen to screen blt is slow
        //
        pDriverCaps->PresentationCaps.NoScreenToScreenBlt = TRUE;
        pDriverCaps->PresentationCaps.NoOverlapScreenBlt = TRUE;

        //
        // Allow 16Kx16K (2 << (11 + 3)) texture(redirection device bitmap)
        //
        pDriverCaps->PresentationCaps.MaxTextureWidthShift = 3;
        pDriverCaps->PresentationCaps.MaxTextureHeightShift = 3;

        //
        // Use SW flip queue for flip with interval of 1 or more
        //   - we must NOT generate a DMA buffer in DxgkDdiPresent. That is,
        //     we must set the DXGKARG_PRESENT.pDmaBuffer output parameter
        //     to NULL.
        //   - DxgkDdiSetVidPnSourceAddress will be called at DIRQL
        //
        pDriverCaps->FlipCaps.FlipOnVSyncMmIo = TRUE;

        if (!RosKmdGlobal::IsRenderOnly())
        {
            //
            // The hardware can store at most one pending flip operation.
            //
            pDriverCaps->MaxQueuedFlipOnVSync = 1;

            //
            // FlipOnVSyncWithNoWait - we don't have to wait for the next VSync
            // to program in the new source address. We can program in the new
            // source address and return immediately, and it will take
            // effect at the next vsync.
            //
            pDriverCaps->FlipCaps.FlipOnVSyncWithNoWait = TRUE;

            //
            // We do not support the scheduling of a flip command to take effect
            // after two, three, or four vertical syncs.
            //
            pDriverCaps->FlipCaps.FlipInterval = FALSE;

            //
            // The address we program into hardware does not take effect until
            // the next vsync.
            //
            pDriverCaps->FlipCaps.FlipImmediateMmIo = FALSE;

            //
            // WDDM 1.3 and later drivers must set this to TRUE.
            // In an independent flip, the DWM user-mode present call is skipped
            // and DxgkDdiPresent and DxgkDdiSetVidPnSourceAddress are called.
            //
            pDriverCaps->FlipCaps.FlipIndependent = TRUE;

            //
            // TODO[jordanrh] VSyncPowerSaveAware
            // https://msdn.microsoft.com/en-us/library/windows/hardware/ff569520(v=vs.85).aspx
            //
        }

        //
        // TODO[bhouse] SchedulingCaps
        //

#if 1
        pDriverCaps->SchedulingCaps.MultiEngineAware = 1;
#endif

        //
        // Set scheduling caps to indicate support for cancelling DMA buffer
        //
#if 1
        pDriverCaps->SchedulingCaps.CancelCommandAware = 1;
#endif

        //
        // Set scheduling caps to indicate driver is preemption aware
        //
#if 1
        pDriverCaps->SchedulingCaps.PreemptionAware = 1;
#endif

        //
        // TODO[bhouse] MemoryManagementCaps
        //
#if 1
        pDriverCaps->MemoryManagementCaps.CrossAdapterResource = 1;
#endif

        //
        // TODO[bhouse] GpuEngineTopology
        //

        pDriverCaps->GpuEngineTopology.NbAsymetricProcessingNodes = m_NumNodes;

        //
        // TODO[bhouse] WDDMVersion
        //              Documentation states that we should not set this value if WDDM 1.3
        //
        pDriverCaps->WDDMVersion = m_WDDMVersion;

        //
        // TODO[bhouse] VirtualAddressCaps
        //

        //
        // TODO[bhouse] DmaBufferCaps
        //

        //
        // TODO[bhouse] PreemptionCaps
        //
#if 1
        pDriverCaps->PreemptionCaps.GraphicsPreemptionGranularity = D3DKMDT_GRAPHICS_PREEMPTION_PRIMITIVE_BOUNDARY;
        pDriverCaps->PreemptionCaps.ComputePreemptionGranularity = D3DKMDT_COMPUTE_PREEMPTION_DISPATCH_BOUNDARY;
#endif

        //
        // Must support DxgkDdiStopDeviceAndReleasePostDisplayOwnership
        //
        pDriverCaps->SupportNonVGA = TRUE;

        //
        // Must support updating path rotation in DxgkDdiUpdateActiveVidPnPresentPath
        //
        pDriverCaps->SupportSmoothRotation = TRUE;

        //
        // TODO[bhouse] SupportPerEngineTDR
        //
#if 1
        pDriverCaps->SupportPerEngineTDR = 1;
#endif

        //
        // SupportDirectFlip
        //   - must not allow video memory to be flipped to an incompatible
        //     allocation in DxgkDdiSetVidPnSourceAddress
        //   - the user mode driver must validate Direct Flip resources before
        //     the DWM uses them
        //
        pDriverCaps->SupportDirectFlip = 1;

        //
        // TODO[bhouse] SupportMultiPlaneOverlay
        //

        //
        // Support SupportRuntimePowerManagement
        // TODO[jordanrh] setting this to true causes constant power state transitions
        //
        pDriverCaps->SupportRuntimePowerManagement = FALSE;

        //
        // TODO[bhouse] SupportSurpriseRemovalInHibernation
        //

        //
        // TODO[bhouse] HybridDiscrete
        //

        //
        // TODO[bhouse] MaxOverlayPlanes
        //

    }
    break;

    case DXGKQAITYPE_QUERYSEGMENT3:
    {
        if (pQueryAdapterInfo->OutputDataSize < sizeof(DXGK_QUERYSEGMENTOUT3))
        {
            ROS_LOG_ASSERTION(
                "Output buffer is too small. (pQueryAdapterInfo->OutputDataSize=%d, sizeof(DXGK_QUERYSEGMENTOUT3)=%d)",
                pQueryAdapterInfo->OutputDataSize,
                sizeof(DXGK_QUERYSEGMENTOUT3));
            return STATUS_BUFFER_TOO_SMALL;
        }

        DXGK_QUERYSEGMENTOUT3   *pSegmentInfo = (DXGK_QUERYSEGMENTOUT3*)pQueryAdapterInfo->pOutputData;

        if (!pSegmentInfo[0].pSegmentDescriptor)
        {
            pSegmentInfo->NbSegment = 2;
        }
        else
        {
            DXGK_SEGMENTDESCRIPTOR3 *pSegmentDesc = pSegmentInfo->pSegmentDescriptor;

            //
            // Private data size should be the maximum of UMD and KMD and the same size must
            // be reported in DxgkDdiCreateContext for paging engine
            //
            pSegmentInfo->PagingBufferPrivateDataSize = sizeof(ROSUMDDMAPRIVATEDATA2);

            pSegmentInfo->PagingBufferSegmentId = ROSD_SEGMENT_APERTURE;
            pSegmentInfo->PagingBufferSize = PAGE_SIZE;

            //
            // Fill out aperture segment descriptor
            //
            memset(&pSegmentDesc[0], 0, sizeof(pSegmentDesc[0]));

            pSegmentDesc[0].Flags.Aperture = TRUE;

            //
            // TODO[bhouse] What does marking it CacheCoherent mean?  What are the side effects?
            //              What happens if we don't mark CacheCoherent?
            //
            pSegmentDesc[0].Flags.CacheCoherent = TRUE;

            //
            // TODO[bhouse] BaseAddress should never be used.  Do we need to set this still?
            //

            pSegmentDesc[0].BaseAddress.QuadPart = ROSD_SEGMENT_APERTURE_BASE_ADDRESS;

            //
            // Our fake apperture is not really visible and doesn't need to be.  We
            // still need to lie that it is visible reporting a bad physical address
            // that will never be used. This is a legacy requirement of the DX stack.
            //

            pSegmentDesc[0].CpuTranslatedAddress.QuadPart = 0xFFFFFFFE00000000;
            pSegmentDesc[0].Flags.CpuVisible = TRUE;

            pSegmentDesc[0].Size = kApertureSegmentSize;
            pSegmentDesc[0].CommitLimit = kApertureSegmentSize;

            //
            // Setup local video memory segment
            //

            memset(&pSegmentDesc[1], 0, sizeof(pSegmentDesc[1]));

            pSegmentDesc[1].BaseAddress.QuadPart = 0LL; // Gpu base physical address
            pSegmentDesc[1].Flags.CpuVisible = true;
            pSegmentDesc[1].Flags.CacheCoherent = true;
            pSegmentDesc[1].Flags.DirectFlip = true;
            pSegmentDesc[1].CpuTranslatedAddress = RosKmdGlobal::s_videoMemoryPhysicalAddress; // cpu base physical address
            pSegmentDesc[1].Size = m_localVidMemSegmentSize;

        }
    }
    break;

    case DXGKQAITYPE_NUMPOWERCOMPONENTS:
    {
        if (pQueryAdapterInfo->OutputDataSize != sizeof(UINT))
        {
            ROS_LOG_ASSERTION(
                "Output buffer is unexpected size. (pQueryAdapterInfo->OutputDataSize=%d, sizeof(UINT)=%d)",
                pQueryAdapterInfo->OutputDataSize,
                sizeof(UINT));
            return STATUS_INVALID_PARAMETER;
        }

        //
        // Support only one 3D engine(s).
        //
        *(static_cast<UINT*>(pQueryAdapterInfo->pOutputData)) = GetNumPowerComponents();
    }
    break;

    case DXGKQAITYPE_POWERCOMPONENTINFO:
    {
        if (pQueryAdapterInfo->InputDataSize != sizeof(UINT))
        {
            ROS_LOG_ASSERTION(
                "Input buffer is not of the expected size. (pQueryAdapterInfo->InputDataSize=%d, sizeof(UINT)=%d)",
                pQueryAdapterInfo->InputDataSize,
                sizeof(UINT));
            return STATUS_INVALID_PARAMETER;
        }

        if (pQueryAdapterInfo->OutputDataSize < sizeof(DXGK_POWER_RUNTIME_COMPONENT))
        {
            ROS_LOG_ASSERTION(
                "Output buffer is too small. (pQueryAdapterInfo->OutputDataSize=%d, sizeof(DXGK_POWER_RUNTIME_COMPONENT)=%d)",
                pQueryAdapterInfo->OutputDataSize,
                sizeof(DXGK_POWER_RUNTIME_COMPONENT));
            return STATUS_BUFFER_TOO_SMALL;
        }

        ULONG ComponentIndex = *(reinterpret_cast<UINT*>(pQueryAdapterInfo->pInputData));
        DXGK_POWER_RUNTIME_COMPONENT* pPowerComponent = reinterpret_cast<DXGK_POWER_RUNTIME_COMPONENT*>(pQueryAdapterInfo->pOutputData);

        NTSTATUS status = GetPowerComponentInfo(ComponentIndex, pPowerComponent);
        if (!NT_SUCCESS(status))
        {
            ROS_LOG_ERROR(
                "GetPowerComponentInfo(...) failed. (status=%!STATUS!, ComponentIndex=%d, pPowerComponent=0x%p)",
                status,
                ComponentIndex,
                pPowerComponent);
            return status;
        }
    }
    break;

    case DXGKQAITYPE_HISTORYBUFFERPRECISION:
    {
        UINT NumStructures = pQueryAdapterInfo->OutputDataSize / sizeof(DXGKARG_HISTORYBUFFERPRECISION);

        for (UINT i = 0; i < NumStructures; i++)
        {
            DXGKARG_HISTORYBUFFERPRECISION *pHistoryBufferPrecision = ((DXGKARG_HISTORYBUFFERPRECISION *)pQueryAdapterInfo->pOutputData) + i;

            pHistoryBufferPrecision->PrecisionBits = 64;
        }

    }
    break;

    default:
        ROS_LOG_WARNING(
            "Unsupported query type. (pQueryAdapterInfo->Type=%d, pQueryAdapterInfo=0x%p)",
            pQueryAdapterInfo->Type,
            pQueryAdapterInfo);
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::DescribeAllocation(
    INOUT_PDXGKARG_DESCRIBEALLOCATION       pDescribeAllocation)
{
    RosKmdAllocation *pAllocation = (RosKmdAllocation *)pDescribeAllocation->hAllocation;

    pDescribeAllocation->Width = pAllocation->m_mip0Info.TexelWidth;
    pDescribeAllocation->Height = pAllocation->m_mip0Info.TexelHeight;
    pDescribeAllocation->Format = TranslateDxgiFormat(pAllocation->m_format);

    pDescribeAllocation->MultisampleMethod.NumSamples = pAllocation->m_sampleDesc.Count;
    pDescribeAllocation->MultisampleMethod.NumQualityLevels = pAllocation->m_sampleDesc.Quality;

    pDescribeAllocation->RefreshRate.Numerator = pAllocation->m_primaryDesc.ModeDesc.RefreshRate.Numerator;
    pDescribeAllocation->RefreshRate.Denominator = pAllocation->m_primaryDesc.ModeDesc.RefreshRate.Denominator;

    return STATUS_SUCCESS;

}

NTSTATUS
RosKmAdapter::GetNodeMetadata(
    UINT                            NodeOrdinal,
    OUT_PDXGKARG_GETNODEMETADATA    pGetNodeMetadata
    )
{
    RtlZeroMemory(pGetNodeMetadata, sizeof(*pGetNodeMetadata));

    pGetNodeMetadata->EngineType = DXGK_ENGINE_TYPE_3D;

    RtlStringCbPrintfW(pGetNodeMetadata->FriendlyName,
        sizeof(pGetNodeMetadata->FriendlyName),
        L"3DNode%02X",
        NodeOrdinal);


    return STATUS_SUCCESS;
}


NTSTATUS
RosKmAdapter::SubmitCommandVirtual(
    IN_CONST_PDXGKARG_SUBMITCOMMANDVIRTUAL  /*pSubmitCommandVirtual*/)
{
    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
RosKmAdapter::PreemptCommand(
    IN_CONST_PDXGKARG_PREEMPTCOMMAND    /*pPreemptCommand*/)
{
    ROS_LOG_WARNING("Not implemented");
    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::RestartFromTimeout(void)
{
    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
RosKmAdapter::CancelCommand(
    IN_CONST_PDXGKARG_CANCELCOMMAND /*pCancelCommand*/)
{
    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
RosKmAdapter::QueryCurrentFence(
    INOUT_PDXGKARG_QUERYCURRENTFENCE pCurrentFence)
{
    ROS_LOG_WARNING("Not implemented");

    NT_ASSERT(pCurrentFence->NodeOrdinal == 0);
    NT_ASSERT(pCurrentFence->EngineOrdinal == 0);

    pCurrentFence->CurrentFence = 0;
    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::ResetEngine(
    INOUT_PDXGKARG_RESETENGINE  /*pResetEngine*/)
{
    ROS_LOG_WARNING("Not implemented");
    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::CollectDbgInfo(
    IN_CONST_PDXGKARG_COLLECTDBGINFO        /*pCollectDbgInfo*/)
{
    ROS_LOG_WARNING("Not implemented");
    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::CreateProcess(
    IN DXGKARG_CREATEPROCESS* /*pArgs*/)
{
    // pArgs->hKmdProcess = 0;
    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
RosKmAdapter::DestroyProcess(
    IN HANDLE /*KmdProcessHandle*/)
{
    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

void
RosKmAdapter::SetStablePowerState(
    IN_CONST_PDXGKARG_SETSTABLEPOWERSTATE  pArgs)
{
    UNREFERENCED_PARAMETER(pArgs);
    ROS_LOG_ASSERTION("Not implemented");
}

NTSTATUS
RosKmAdapter::CalibrateGpuClock(
    IN UINT32                                   /*NodeOrdinal*/,
    IN UINT32                                   /*EngineOrdinal*/,
    OUT_PDXGKARG_CALIBRATEGPUCLOCK              /*pClockCalibration*/
    )
{
    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
RosKmAdapter::Escape(
    IN_CONST_PDXGKARG_ESCAPE        pEscape)
{
    NTSTATUS        Status;

    if (pEscape->PrivateDriverDataSize < sizeof(UINT))
    {
        ROS_LOG_ERROR(
            "PrivateDriverDataSize is too small. (pEscape->PrivateDriverDataSize=%d, sizeof(UINT)=%d)",
            pEscape->PrivateDriverDataSize,
            sizeof(UINT));
        return STATUS_BUFFER_TOO_SMALL;
    }

    UINT    EscapeId = *((UINT *)pEscape->pPrivateDriverData);

#pragma warning( disable : 4065 )
    switch (EscapeId)
    {

    default:

        NT_ASSERT(false);
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
RosKmAdapter::ResetFromTimeout(void)
{
    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
RosKmAdapter::QueryChildRelations(
    INOUT_PDXGK_CHILD_DESCRIPTOR    ChildRelations,
    IN_ULONG                        ChildRelationsSize)
{
    if (RosKmdGlobal::IsRenderOnly())
    {
        ROS_LOG_ASSERTION("QueryChildRelations() is not supported by render-only driver.");
        return STATUS_NOT_IMPLEMENTED;
    }

    return m_display.QueryChildRelations(ChildRelations, ChildRelationsSize);
}

NTSTATUS
RosKmAdapter::QueryChildStatus(
    IN_PDXGK_CHILD_STATUS   ChildStatus,
    IN_BOOLEAN              NonDestructiveOnly)
{
    if (RosKmdGlobal::IsRenderOnly())
    {
        ROS_LOG_ASSERTION("QueryChildStatus() is not supported by render-only driver.");
        return STATUS_NOT_IMPLEMENTED;
    }

    return m_display.QueryChildStatus(ChildStatus, NonDestructiveOnly);
}

NTSTATUS
RosKmAdapter::QueryDeviceDescriptor(
    IN_ULONG                        ChildUid,
    INOUT_PDXGK_DEVICE_DESCRIPTOR   pDeviceDescriptor)
{
    if (RosKmdGlobal::IsRenderOnly())
    {
        ROS_LOG_ASSERTION("QueryChildStatus() is not supported by render-only driver.");
        return STATUS_NOT_IMPLEMENTED;
    }

    return m_display.QueryDeviceDescriptor(ChildUid, pDeviceDescriptor);
}

NTSTATUS
RosKmAdapter::NotifyAcpiEvent(
    IN_DXGK_EVENT_TYPE  EventType,
    IN_ULONG            Event,
    IN_PVOID            Argument,
    OUT_PULONG          AcpiFlags)
{
    EventType;
    Event;
    Argument;
    AcpiFlags;

    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

void
RosKmAdapter::ResetDevice(void)
{
    // Do nothing
    ROS_LOG_ASSERTION("Not implemented");
}

void
RosKmAdapter::PatchDmaBuffer(
    ROSDMABUFINFO*                  pDmaBufInfo,
    CONST DXGK_ALLOCATIONLIST*      pAllocationList,
    UINT                            allocationListSize,
    CONST D3DDDI_PATCHLOCATIONLIST* pPatchLocationList,
    UINT                            patchAllocationList)
{
    PBYTE       pDmaBuf = (PBYTE)pDmaBufInfo->m_pDmaBuffer;

    for (UINT i = 0; i < patchAllocationList; i++)
    {
        auto patch = &pPatchLocationList[i];

        allocationListSize;
        NT_ASSERT(patch->AllocationIndex < allocationListSize);

        auto allocation = &pAllocationList[patch->AllocationIndex];

        RosKmdDeviceAllocation * pRosKmdDeviceAllocation = (RosKmdDeviceAllocation *)allocation->hDeviceSpecificAllocation;

        if (allocation->SegmentId != 0)
        {
            DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "Patch RosKmdDeviceAllocation %lx at %lx\n", pRosKmdDeviceAllocation, allocation->PhysicalAddress);
            DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "Patch buffer offset %lx allocation offset %lx\n", patch->PatchOffset, patch->AllocationOffset);

            // Patch in dma buffer
            NT_ASSERT(allocation->SegmentId == ROSD_SEGMENT_VIDEO_MEMORY);
            if (pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer)
            {
                PHYSICAL_ADDRESS    allocAddress;

                allocAddress.QuadPart = allocation->PhysicalAddress.QuadPart + (LONGLONG)patch->AllocationOffset;
                *((PHYSICAL_ADDRESS *)(pDmaBuf + patch->PatchOffset)) = allocAddress;
            }
            else
            {
                // Patch HW command buffer
#if VC4
                UINT    physicalAddress =
                    RosKmdGlobal::s_videoMemoryPhysicalAddress.LowPart +
                    allocation->PhysicalAddress.LowPart +
                    patch->AllocationOffset;

                switch (patch->SlotId)
                {
                case VC4_SLOT_RT_BINNING_CONFIG:
                    pDmaBufInfo->m_RenderTargetPhysicalAddress = physicalAddress;
                    break;
                case VC4_SLOT_TILE_ALLOCATION_MEMORY:
                    *((UINT *)(pDmaBuf + patch->PatchOffset)) = m_tileAllocationMemoryPhysicalAddress + m_busAddressOffset;
                    break;
                case VC4_SLOT_TILE_STATE_DATA_ARRAY:
                    *((UINT *)(pDmaBuf + patch->PatchOffset)) = m_tileStateDataArrayPhysicalAddress + m_busAddressOffset;
                    break;
                case VC4_SLOT_NV_SHADER_STATE:
                case VC4_SLOT_BRANCH:
                case VC4_SLOT_GL_SHADER_STATE:
                case VC4_SLOT_FS_UNIFORM_ADDRESS:
                case VC4_SLOT_VS_UNIFORM_ADDRESS:
                case VC4_SLOT_CS_UNIFORM_ADDRESS:
                    // When PrePatch happens in DdiRender, DMA buffer physical
                    // address is not available, so DMA buffer self-reference
                    // patches are handled in SubmitCommand
                    break;
                default:
                    *((UINT *)(pDmaBuf + patch->PatchOffset)) = physicalAddress + m_busAddressOffset;
                }
#endif
            }
        }
    }
}

//
// TODO[indyz]: Add proper validation for DMA buffer
//
bool
RosKmAdapter::ValidateDmaBuffer(
    ROSDMABUFINFO*                  pDmaBufInfo,
    CONST DXGK_ALLOCATIONLIST*      pAllocationList,
    UINT                            allocationListSize,
    CONST D3DDDI_PATCHLOCATIONLIST* pPatchLocationList,
    UINT                            patchAllocationList)
{
    PBYTE           pDmaBuf = (PBYTE)pDmaBufInfo->m_pDmaBuffer;
    bool            bValidateDmaBuffer = true;
    ROSDMABUFSTATE* pDmaBufState = &pDmaBufInfo->m_DmaBufState;

    pDmaBuf;

    if (! pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer)
    {
        for (UINT i = 0; i < patchAllocationList; i++)
        {
            auto patch = &pPatchLocationList[i];

            allocationListSize;
            NT_ASSERT(patch->AllocationIndex < allocationListSize);

            auto allocation = &pAllocationList[patch->AllocationIndex];

            RosKmdDeviceAllocation * pRosKmdDeviceAllocation = (RosKmdDeviceAllocation *)allocation->hDeviceSpecificAllocation;

#if VC4

            switch (patch->SlotId)
            {
            case VC4_SLOT_TILE_ALLOCATION_MEMORY:
                if (pDmaBufState->m_bTileAllocMemRef)
                {
                    return false;   // Allow one per DMA buffer
                }
                else
                {
                    pDmaBufState->m_bTileAllocMemRef = 1;
                }
                break;
            case VC4_SLOT_TILE_STATE_DATA_ARRAY:
                if (pDmaBufState->m_bTileStateDataRef)
                {
                    return false;   // Allow one per DMA buffer
                }
                else
                {
                    pDmaBufState->m_bTileStateDataRef = 1;
                }
                break;
            case VC4_SLOT_RT_BINNING_CONFIG:
                if (pDmaBufState->m_bRenderTargetRef)
                {
                    return false;   // Allow one per DMA buffer
                }
                else
                {
                    pDmaBufInfo->m_pRenderTarget = pRosKmdDeviceAllocation->m_pRosKmdAllocation;
                    pDmaBufState->m_bRenderTargetRef = 1;
                }
                break;
            case VC4_SLOT_NV_SHADER_STATE:
            case VC4_SLOT_BRANCH:
            case VC4_SLOT_GL_SHADER_STATE:
            case VC4_SLOT_FS_UNIFORM_ADDRESS:
            case VC4_SLOT_VS_UNIFORM_ADDRESS:
            case VC4_SLOT_CS_UNIFORM_ADDRESS:
                if (pDmaBufState->m_NumDmaBufSelfRef == VC4_MAX_DMA_BUFFER_SELF_REF)
                {
                    return false;   // Allow up to VC4_MAX_DMA_BUFFER_SELF_REF
                }
                else
                {
                    pDmaBufInfo->m_DmaBufSelfRef[pDmaBufState->m_NumDmaBufSelfRef] = *patch;
                    pDmaBufState->m_NumDmaBufSelfRef++;
                }
                break;
            default:
                break;
            }

#endif
        }

        if ((0 == pDmaBufState->m_bRenderTargetRef) ||
            (0 == pDmaBufState->m_bTileAllocMemRef) ||
            (0 == pDmaBufState->m_bTileStateDataRef))
        {
            bValidateDmaBuffer = false;
        }
    }

    return bValidateDmaBuffer;
}

void
RosKmAdapter::QueueDmaBuffer(
    IN_CONST_PDXGKARG_SUBMITCOMMAND pSubmitCommand)
{
    ROSDMABUFINFO *         pDmaBufInfo = (ROSDMABUFINFO *)pSubmitCommand->pDmaBufferPrivateData;
    KIRQL                   OldIrql;
    ROSDMABUFSUBMISSION *   pDmaBufSubmission;

    KeAcquireSpinLock(&m_dmaBufQueueLock, &OldIrql);

    //
    // Combination indicating preparation error, thus the DMA buffer should be discarded
    //
    if ((pSubmitCommand->DmaBufferPhysicalAddress.QuadPart == 0) &&
        (pSubmitCommand->DmaBufferSubmissionStartOffset == 0) &&
        (pSubmitCommand->DmaBufferSubmissionEndOffset == 0))
    {
        m_ErrorHit.m_PreparationError = 1;
    }

    if (!pDmaBufInfo->m_DmaBufState.m_bSubmittedOnce)
    {
        pDmaBufInfo->m_DmaBufState.m_bSubmittedOnce = 1;
    }

    NT_ASSERT(!IsListEmpty(&m_dmaBufSubmissionFree));

    pDmaBufSubmission = CONTAINING_RECORD(RemoveHeadList(&m_dmaBufSubmissionFree), ROSDMABUFSUBMISSION, m_QueueEntry);

    pDmaBufSubmission->m_pDmaBufInfo = pDmaBufInfo;

    pDmaBufSubmission->m_StartOffset = pSubmitCommand->DmaBufferSubmissionStartOffset;
    pDmaBufSubmission->m_EndOffset = pSubmitCommand->DmaBufferSubmissionEndOffset;
    pDmaBufSubmission->m_SubmissionFenceId = pSubmitCommand->SubmissionFenceId;

    InsertTailList(&m_dmaBufQueue, &pDmaBufSubmission->m_QueueEntry);

    KeReleaseSpinLock(&m_dmaBufQueueLock, OldIrql);
}

void
RosKmAdapter::HwDmaBufCompletionDpcRoutine(
    KDPC   *pDPC,
    PVOID   deferredContext,
    PVOID   systemArgument1,
    PVOID   systemArgument2)
{
    RosKmAdapter   *pRosKmAdapter = RosKmAdapter::Cast(deferredContext);

    UNREFERENCED_PARAMETER(pDPC);
    UNREFERENCED_PARAMETER(systemArgument1);
    UNREFERENCED_PARAMETER(systemArgument2);

    // Signal to the worker thread that a HW DMA buffer has completed
    KeSetEvent(&pRosKmAdapter->m_hwDmaBufCompletionEvent, 0, FALSE);
}

ROS_NONPAGED_SEGMENT_BEGIN; //================================================

_Use_decl_annotations_
NTSTATUS RosKmAdapter::SetVidPnSourceAddress (
    const DXGKARG_SETVIDPNSOURCEADDRESS* SetVidPnSourceAddressPtr
    )
{
    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.SetVidPnSourceAddress(SetVidPnSourceAddressPtr);
}

ROS_NONPAGED_SEGMENT_END; //==================================================
ROS_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
NTSTATUS RosKmAdapter::QueryInterface (QUERY_INTERFACE* Args)
{
    ROS_LOG_WARNING(
        "Received QueryInterface for unsupported interface. (InterfaceType=%!GUID!)",
        Args->InterfaceType);
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::GetStandardAllocationDriverData (
    DXGKARG_GETSTANDARDALLOCATIONDRIVERDATA* Args
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    //
    // ResourcePrivateDriverDataSize gets passed to CreateAllocation as
    // PrivateDriverDataSize.
    // AllocationPrivateDriverDataSize get passed to CreateAllocation as
    // pAllocationInfo->PrivateDriverDataSize.
    //

    if (!Args->pResourcePrivateDriverData && !Args->pResourcePrivateDriverData)
    {
        Args->ResourcePrivateDriverDataSize = sizeof(RosAllocationGroupExchange);
        Args->AllocationPrivateDriverDataSize = sizeof(RosAllocationExchange);
        return STATUS_SUCCESS;
    }

    // we expect them to both be null or both be valid
    NT_ASSERT(Args->pResourcePrivateDriverData && Args->pResourcePrivateDriverData);
    NT_ASSERT(
        Args->ResourcePrivateDriverDataSize ==
        sizeof(RosAllocationGroupExchange));

    NT_ASSERT(
        Args->AllocationPrivateDriverDataSize ==
        sizeof(RosAllocationExchange));

    new (Args->pResourcePrivateDriverData) RosAllocationGroupExchange();
    auto allocParams = new (Args->pAllocationPrivateDriverData) RosAllocationExchange();

    switch (Args->StandardAllocationType)
    {
    case D3DKMDT_STANDARDALLOCATION_SHAREDPRIMARYSURFACE:
    {
        const D3DKMDT_SHAREDPRIMARYSURFACEDATA* surfData =
                Args->pCreateSharedPrimarySurfaceData;

        ROS_LOG_TRACE(
            "Preparing private allocation data for SHAREDPRIMARYSURFACEDATA. (Width=%d, Height=%d, Format=%d, RefreshRate=%d/%d, VidPnSourceId=%d)",
            surfData->Width,
            surfData->Height,
            surfData->Format,
            surfData->RefreshRate.Numerator,
            surfData->RefreshRate.Denominator,
            surfData->VidPnSourceId);

        allocParams->m_resourceDimension = D3D10DDIRESOURCE_TEXTURE2D;
        allocParams->m_mip0Info.TexelWidth = surfData->Width;
        allocParams->m_mip0Info.TexelHeight = surfData->Height;
        allocParams->m_mip0Info.TexelDepth = 0;
        allocParams->m_mip0Info.PhysicalWidth = surfData->Width;
        allocParams->m_mip0Info.PhysicalHeight = surfData->Height;
        allocParams->m_mip0Info.PhysicalDepth = 0;

        allocParams->m_usage = D3D10_DDI_USAGE_IMMUTABLE;

        // We must ensure that the D3D10_DDI_BIND_PRESENT is set so that
        // CreateAllocation() creates an allocation that is suitable
        // for the primary, which must be flippable.
        // The primary cannot be cached.
        allocParams->m_bindFlags = D3D10_DDI_BIND_RENDER_TARGET | D3D10_DDI_BIND_PRESENT;

        allocParams->m_mapFlags = 0;

        // The shared primary allocation is shared by definition
        allocParams->m_miscFlags = D3D10_DDI_RESOURCE_MISC_SHARED;

        allocParams->m_format = DxgiFormatFromD3dDdiFormat(surfData->Format);
        allocParams->m_sampleDesc.Count = 1;
        allocParams->m_sampleDesc.Quality = 0;
        allocParams->m_mipLevels = 1;
        allocParams->m_arraySize = 1;
        allocParams->m_isPrimary = true;
        allocParams->m_primaryDesc.Flags = 0;
        allocParams->m_primaryDesc.VidPnSourceId = surfData->VidPnSourceId;
        allocParams->m_primaryDesc.ModeDesc.Width = surfData->Width;
        allocParams->m_primaryDesc.ModeDesc.Height = surfData->Height;
        allocParams->m_primaryDesc.ModeDesc.Format = DxgiFormatFromD3dDdiFormat(surfData->Format);
        allocParams->m_primaryDesc.ModeDesc.RefreshRate.Numerator = surfData->RefreshRate.Numerator;
        allocParams->m_primaryDesc.ModeDesc.RefreshRate.Denominator = surfData->RefreshRate.Denominator;
        allocParams->m_primaryDesc.ModeDesc.ScanlineOrdering = DXGI_DDI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        allocParams->m_primaryDesc.ModeDesc.Rotation = DXGI_DDI_MODE_ROTATION_UNSPECIFIED;
        allocParams->m_primaryDesc.ModeDesc.Scaling = DXGI_DDI_MODE_SCALING_UNSPECIFIED;
        allocParams->m_primaryDesc.DriverFlags = 0;

        allocParams->m_hwLayout = RosHwLayout::Linear;
        allocParams->m_hwWidthPixels = surfData->Width;
        allocParams->m_hwHeightPixels = surfData->Height;

        NT_ASSERT(surfData->Format == D3DDDIFMT_A8R8G8B8);
        allocParams->m_hwFormat = RosHwFormat::X8888;
        allocParams->m_hwPitchBytes = surfData->Width * 4;
        allocParams->m_hwSizeBytes = allocParams->m_hwPitchBytes * surfData->Height;

        return STATUS_SUCCESS;
    }
    case D3DKMDT_STANDARDALLOCATION_SHADOWSURFACE:
    {
        const D3DKMDT_SHADOWSURFACEDATA* surfData = Args->pCreateShadowSurfaceData;
        ROS_LOG_TRACE(
            "Preparing private allocation data for SHADOWSURFACE. (Width=%d, Height=%d, Format=%d)",
            surfData->Width,
            surfData->Height,
            surfData->Format);

        allocParams->m_resourceDimension = D3D10DDIRESOURCE_TEXTURE2D;
        allocParams->m_mip0Info.TexelWidth = surfData->Width;
        allocParams->m_mip0Info.TexelHeight = surfData->Height;
        allocParams->m_mip0Info.TexelDepth = 0;
        allocParams->m_mip0Info.PhysicalWidth = surfData->Width;
        allocParams->m_mip0Info.PhysicalHeight = surfData->Height;
        allocParams->m_mip0Info.PhysicalDepth = 0;
        allocParams->m_usage = D3D10_DDI_USAGE_DEFAULT;

        // The shadow allocation does not get flipped directly
        static_assert(
            !(D3D10_DDI_BIND_PIPELINE_MASK & D3D10_DDI_BIND_PRESENT),
            "BIND_PRESENT must not be part of BIND_MASK");
        allocParams->m_bindFlags = D3D10_DDI_BIND_PIPELINE_MASK;

        allocParams->m_mapFlags = D3D10_DDI_MAP_READWRITE;
        allocParams->m_miscFlags = D3D10_DDI_RESOURCE_MISC_SHARED;

        allocParams->m_format = DxgiFormatFromD3dDdiFormat(surfData->Format);
        allocParams->m_sampleDesc.Count = 1;
        allocParams->m_sampleDesc.Quality = 0;
        allocParams->m_mipLevels = 1;
        allocParams->m_arraySize = 1;
        allocParams->m_isPrimary = true;
        allocParams->m_primaryDesc.Flags = 0;
        allocParams->m_primaryDesc.ModeDesc.Width = surfData->Width;
        allocParams->m_primaryDesc.ModeDesc.Height = surfData->Height;
        allocParams->m_primaryDesc.ModeDesc.Format = DxgiFormatFromD3dDdiFormat(surfData->Format);
        allocParams->m_primaryDesc.DriverFlags = 0;
        allocParams->m_hwLayout = RosHwLayout::Linear;
        allocParams->m_hwWidthPixels = surfData->Width;
        allocParams->m_hwHeightPixels = surfData->Height;

        NT_ASSERT(surfData->Format == D3DDDIFMT_A8R8G8B8);
        allocParams->m_hwFormat = RosHwFormat::X8888;
        allocParams->m_hwPitchBytes = surfData->Width * 4;
        allocParams->m_hwSizeBytes = allocParams->m_hwPitchBytes * surfData->Height;

        Args->pCreateShadowSurfaceData->Pitch = allocParams->m_hwPitchBytes;
        return STATUS_SUCCESS;
    }
    case D3DKMDT_STANDARDALLOCATION_STAGINGSURFACE:
    {
        const D3DKMDT_STAGINGSURFACEDATA* surfData = Args->pCreateStagingSurfaceData;
        ROS_LOG_ASSERTION(
            "STAGINGSURFACEDATA is not implemented. (Width=%d, Height=%d, Pitch=%d)",
            surfData->Width,
            surfData->Height,
            surfData->Pitch);
        return STATUS_NOT_IMPLEMENTED;
    }
    case D3DKMDT_STANDARDALLOCATION_GDISURFACE:
    {
        const D3DKMDT_GDISURFACEDATA* surfData = Args->pCreateGdiSurfaceData;
        ROS_LOG_ASSERTION(
            "GDISURFACEDATA is not implemented. We must return a nonzero Pitch if allocation is CPU visible. (Width=%d, Height=%d, Format=%d, Type=%d, Flags=0x%x, Pitch=%d)",
            surfData->Width,
            surfData->Height,
            surfData->Format,
            surfData->Type,
            surfData->Flags.Value,
            surfData->Pitch);
        return STATUS_NOT_IMPLEMENTED;
    }
    default:
        ROS_LOG_ASSERTION(
            "Unknown standard allocation type. (StandardAllocationType=%d)",
            Args->StandardAllocationType);
        return STATUS_INVALID_PARAMETER;
    }
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::SetPalette (const DXGKARG_SETPALETTE* /*SetPalettePtr*/)
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_ASSERTION("Not implemented.");
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::SetPointerPosition (
    const DXGKARG_SETPOINTERPOSITION* SetPointerPositionPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.SetPointerPosition(SetPointerPositionPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::SetPointerShape (
    const DXGKARG_SETPOINTERSHAPE* SetPointerShapePtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.SetPointerShape(SetPointerShapePtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::IsSupportedVidPn (
    DXGKARG_ISSUPPORTEDVIDPN* IsSupportedVidPnPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.IsSupportedVidPn(IsSupportedVidPnPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::RecommendFunctionalVidPn (
    const DXGKARG_RECOMMENDFUNCTIONALVIDPN* const RecommendFunctionalVidPnPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.RecommendFunctionalVidPn(RecommendFunctionalVidPnPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::EnumVidPnCofuncModality (
    const DXGKARG_ENUMVIDPNCOFUNCMODALITY* const EnumCofuncModalityPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.EnumVidPnCofuncModality(EnumCofuncModalityPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::SetVidPnSourceVisibility (
    const DXGKARG_SETVIDPNSOURCEVISIBILITY* SetVidPnSourceVisibilityPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.SetVidPnSourceVisibility(SetVidPnSourceVisibilityPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::CommitVidPn (
    const DXGKARG_COMMITVIDPN* const CommitVidPnPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.CommitVidPn(CommitVidPnPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::UpdateActiveVidPnPresentPath (
    const DXGKARG_UPDATEACTIVEVIDPNPRESENTPATH* const UpdateActiveVidPnPresentPathPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.UpdateActiveVidPnPresentPath(UpdateActiveVidPnPresentPathPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::RecommendMonitorModes (
    const DXGKARG_RECOMMENDMONITORMODES* const RecommendMonitorModesPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.RecommendMonitorModes(RecommendMonitorModesPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::GetScanLine (DXGKARG_GETSCANLINE* /*GetScanLinePtr*/)
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    ROS_LOG_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::ControlInterrupt (
    const DXGK_INTERRUPT_TYPE InterruptType,
    BOOLEAN EnableInterrupt
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.ControlInterrupt(InterruptType, EnableInterrupt);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::QueryVidPnHWCapability (
    DXGKARG_QUERYVIDPNHWCAPABILITY* VidPnHWCapsPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.QueryVidPnHWCapability(VidPnHWCapsPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::QueryDependentEngineGroup (
    DXGKARG_QUERYDEPENDENTENGINEGROUP* ArgsPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(ArgsPtr->NodeOrdinal == 0);
    NT_ASSERT(ArgsPtr->EngineOrdinal == 0);

    ArgsPtr->DependentNodeOrdinalMask = 0;
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS RosKmAdapter::StopDeviceAndReleasePostDisplayOwnership (
    D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId,
    DXGK_DISPLAY_INFORMATION* DisplayInfoPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!RosKmdGlobal::IsRenderOnly());
    return m_display.StopDeviceAndReleasePostDisplayOwnership(
            TargetId,
            DisplayInfoPtr);
}


ROS_PAGED_SEGMENT_END; //=====================================================
