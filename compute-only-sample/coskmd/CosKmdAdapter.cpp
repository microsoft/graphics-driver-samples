
#include "CosKmd.h"

#include "CosKmdLogging.h"
#include "CosKmdAdapter.tmh"

#include "CosKmdAdapter.h"
#include "CosKmdSoftAdapter.h"
#include "CosKmdAllocation.h"
#include "CosKmdContext.h"
#include "CosKmdResource.h"
#include "CosKmdGlobal.h"
#include "CosKmdUtil.h"
#include "CosGpuCommand.h"
#include "CosKmdAcpi.h"
#include "CosKmdUtil.h"

void * CosKmAdapter::operator new(size_t size)
{
    return ExAllocatePoolWithTag(NonPagedPoolNx, size, 'COSD');
}

void CosKmAdapter::operator delete(void * ptr)
{
    ExFreePool(ptr);
}

CosKmAdapter::CosKmAdapter(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext)
{
    m_magic = kMagic;
    m_pPhysicalDevice = PhysicalDeviceObject;

    // Enable in CosKmAdapter::Start() when device is ready for interrupt
    m_bReadyToHandleInterrupt = FALSE;

    // Set initial power management state.
    m_PowerManagementStarted = FALSE;
    m_AdapterPowerDState = PowerDeviceD0; // Device is at D0 at startup
    m_NumPowerComponents = 0;
    RtlZeroMemory(&m_EnginePowerFState[0], sizeof(m_EnginePowerFState)); // Components are F0 at startup.

    RtlZeroMemory(&m_deviceId, sizeof(m_deviceId));
    m_deviceIdLength = 0;

    m_flags.m_value = 0;

    *MiniportDeviceContext = this;
}

CosKmAdapter::~CosKmAdapter()
{
    // do nothing
}

NTSTATUS
CosKmAdapter::AddAdapter(
    IN_CONST_PDEVICE_OBJECT     PhysicalDeviceObject,
    OUT_PPVOID                  MiniportDeviceContext)
{
    NTSTATUS status;
    WCHAR deviceID[512];
    ULONG dataLen;

    status = IoGetDeviceProperty(PhysicalDeviceObject, DevicePropertyHardwareID, sizeof(deviceID), deviceID, &dataLen);
    if (!NT_SUCCESS(status))
    {
        COS_LOG_ERROR(
            "Failed to get DevicePropertyHardwareID from PDO. (status=%!STATUS!)",
            status);
        return status;
    }

    CosKmAdapter  *pCosKmAdapter = new CosKmdSoftAdapter(PhysicalDeviceObject, MiniportDeviceContext);
    if (!pCosKmAdapter) {
        COS_LOG_LOW_MEMORY("Failed to allocate CosKmdSoftAdapter.");
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::QueryEngineStatus(
    DXGKARG_QUERYENGINESTATUS  *pQueryEngineStatus)
{
    COS_LOG_TRACE("QueryEngineStatus was called.");

    pQueryEngineStatus->EngineStatus.Responsive = 1;
    return STATUS_SUCCESS;
}

void CosKmAdapter::WorkerThread(void * inThis)
{
    CosKmAdapter  *pCosKmAdapter = CosKmAdapter::Cast(inThis);

    pCosKmAdapter->DoWork();
}

void CosKmAdapter::DoWork(void)
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
            COSDMABUFSUBMISSION *   pDmaBufSubmission = DequeueDmaBuffer(&m_dmaBufQueueLock);
            if (pDmaBufSubmission == NULL)
            {
                break;
            }

            COSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

            if (pDmaBufInfo->m_DmaBufState.m_bPaging)
            {
                //
                // Run paging buffer in software
                //

                ProcessPagingBuffer(pDmaBufSubmission);

            }
            else if (pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer)
            {
                //
                // Process render DMA buffer
                //

                ProcessRenderBuffer(pDmaBufSubmission);
            }
            else
            {
                ProcessHWRenderBuffer(pDmaBufSubmission);
            }

            NotifyDmaBufCompletion(pDmaBufSubmission);

            ExInterlockedInsertTailList(&m_dmaBufSubmissionFree, &pDmaBufSubmission->m_QueueEntry, &m_dmaBufQueueLock);
        }
    }
}

COSDMABUFSUBMISSION *
CosKmAdapter::DequeueDmaBuffer(
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

    return CONTAINING_RECORD(pDmaEntry, COSDMABUFSUBMISSION, m_QueueEntry);
}

void
CosKmAdapter::ProcessPagingBuffer(
    COSDMABUFSUBMISSION * pDmaBufSubmission)
{
    COSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

    NT_ASSERT(0 == (pDmaBufSubmission->m_EndOffset - pDmaBufSubmission->m_StartOffset) % sizeof(DXGKARG_BUILDPAGINGBUFFER));

    DXGKARG_BUILDPAGINGBUFFER * pPagingBuffer = (DXGKARG_BUILDPAGINGBUFFER *)(pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_StartOffset);
    DXGKARG_BUILDPAGINGBUFFER * pEndofBuffer = (DXGKARG_BUILDPAGINGBUFFER *)(pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_EndOffset);

    for (; pPagingBuffer < pEndofBuffer; pPagingBuffer++)
    {
        switch (pPagingBuffer->Operation)
        {
        case DXGK_OPERATION_FILL:
        {
            NT_ASSERT(pPagingBuffer->Fill.Destination.SegmentId == COSD_SEGMENT_VIDEO_MEMORY);
            NT_ASSERT(pPagingBuffer->Fill.FillSize % sizeof(ULONG) == 0);

            ULONG * const startAddress = reinterpret_cast<ULONG*>(
                (BYTE *)CosKmdGlobal::s_pVideoMemory +
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

            if (pPagingBuffer->Transfer.Source.SegmentId == COSD_SEGMENT_VIDEO_MEMORY)
            {
                pSource = ((BYTE *)CosKmdGlobal::s_pVideoMemory) + pPagingBuffer->Transfer.Source.SegmentAddress.QuadPart;
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

            if (pPagingBuffer->Transfer.Destination.SegmentId == COSD_SEGMENT_VIDEO_MEMORY)
            {
                pDestination = ((BYTE *)CosKmdGlobal::s_pVideoMemory) + pPagingBuffer->Transfer.Destination.SegmentAddress.QuadPart;
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
CosKmAdapter::NotifyDmaBufCompletion(
    COSDMABUFSUBMISSION * pDmaBufSubmission)
{
    COSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

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

BOOLEAN CosKmAdapter::SynchronizeNotifyInterrupt(PVOID inThis)
{
    CosKmAdapter  *pCosKmAdapter = CosKmAdapter::Cast(inThis);

    return pCosKmAdapter->SynchronizeNotifyInterrupt();
}

BOOLEAN CosKmAdapter::SynchronizeNotifyInterrupt(void)
{
    m_DxgkInterface.DxgkCbNotifyInterrupt(m_DxgkInterface.DeviceHandle, &m_interruptData);

    return m_DxgkInterface.DxgkCbQueueDpc(m_DxgkInterface.DeviceHandle);
}

NTSTATUS
CosKmAdapter::Start(
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
    // Sample for 2.0 model currently
    //
    m_WDDMVersion = DXGKDDI_WDDMv2;

    m_NumNodes = C_COSD_GPU_ENGINE_COUNT;

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
        (PKSTART_ROUTINE) CosKmAdapter::WorkerThread,
        this);

    if (status != STATUS_SUCCESS)
    {
        COS_LOG_ERROR(
            "PsCreateSystemThread(...) failed for CosKmAdapter::WorkerThread. (status=%!STATUS!)",
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
        COS_LOG_ERROR(
            "ObReferenceObjectByHandle(...) failed for worker thread. (status=%!STATUS!)",
            status);
        return status;
    }

    status = m_DxgkInterface.DxgkCbGetDeviceInformation(
        m_DxgkInterface.DeviceHandle,
        &m_deviceInfo);
    if (!NT_SUCCESS(status))
    {
        COS_LOG_ERROR(
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

        CosKmAcpiReader acpiReader(this, DISPLAY_ADAPTER_HW_ID);
        acpiStatus = acpiReader.Read(ACPI_METHOD_HARDWARE_ID);
        if (NT_SUCCESS(acpiStatus) && (acpiReader.GetOutputArgumentCount() == 1))
        {
            CosKmAcpiArgumentParser acpiParser(&acpiReader, NULL);
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

    COS_LOG_TRACE("Adapter was successfully started.");
    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::Stop()
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

    COS_LOG_TRACE("Adapter was successfully stopped.");
    return STATUS_SUCCESS;
}

void CosKmAdapter::DpcRoutine(void)
{
    // dp nothing other than calling back into dxgk

    m_DxgkInterface.DxgkCbNotifyDpc(m_DxgkInterface.DeviceHandle);
}

NTSTATUS
CosKmAdapter::BuildPagingBuffer(
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
        CosKmdAllocation * pCosKmdAllocation = (CosKmdAllocation *)pArgs->Fill.hAllocation;
        pCosKmdAllocation;

        COS_LOG_TRACE(
            "Filling DMA buffer. (Destination.SegmentAddress=0x%I64x, FillPattern=0x%lx, FillSize=%Id)",
            pArgs->Fill.Destination.SegmentAddress.QuadPart,
            pArgs->Fill.FillPattern,
            pArgs->Fill.FillSize);

        if (pArgs->DmaSize < sizeof(DXGKARG_BUILDPAGINGBUFFER))
        {
            COS_LOG_ERROR(
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
            COS_LOG_ERROR(
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
    COSDMABUFINFO * pDmaBufInfo = (COSDMABUFINFO *)pArgs->pDmaBufferPrivateData;
    if (pDmaBufInfo && (pArgs->DmaSize == COSD_PAGING_BUFFER_SIZE))
    {
        pDmaBufInfo->m_DmaBufState.m_Value = 0;
        pDmaBufInfo->m_DmaBufState.m_bPaging = 1;

        pDmaBufInfo->m_pDmaBuffer = pDmaBufStart;
        pDmaBufInfo->m_DmaBufferSize = pArgs->DmaSize;
    }

    return Status;
}

NTSTATUS
CosKmAdapter::DispatchIoRequest(
    IN_ULONG                    VidPnSourceId,
    IN_PVIDEO_REQUEST_PACKET    VideoRequestPacket)
{
    VidPnSourceId;
    VideoRequestPacket;

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
CosKmAdapter::SubmitCommand(
    IN_CONST_PDXGKARG_SUBMITCOMMAND     pSubmitCommand)
{
    NTSTATUS        Status = STATUS_SUCCESS;

    // NOTE: pCosKmContext will be NULL for paging operations
    CosKmContext *pCosKmContext = (CosKmContext *)pSubmitCommand->hContext;
    pCosKmContext;

    QueueDmaBuffer(pSubmitCommand);

    //
    // Wake up the worker thread for the GPU node
    //
    KeSetEvent(&m_workerThreadEvent, 0, FALSE);

    return Status;
}

NTSTATUS
CosKmAdapter::Patch(
    IN_CONST_PDXGKARG_PATCH     pPatch)
{
    COSDMABUFINFO *pDmaBufInfo = (COSDMABUFINFO *)pPatch->pDmaBufferPrivateData;

    CosKmContext * pCosKmContext = (CosKmContext *)pPatch->hContext;
    pCosKmContext;

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
CosKmAdapter::CreateAllocation(
    INOUT_PDXGKARG_CREATEALLOCATION     pCreateAllocation)
{
    // TODO: Find out why this is 0 for some allocations (Breadcrumb buffer)
    if (pCreateAllocation->PrivateDriverDataSize != 0) {
        NT_ASSERT(pCreateAllocation->PrivateDriverDataSize == sizeof(CosAllocationGroupExchange));
        CosAllocationGroupExchange * pCosAllocationGroupExchange = (CosAllocationGroupExchange *)pCreateAllocation->pPrivateDriverData;

        pCosAllocationGroupExchange;
        NT_ASSERT(pCosAllocationGroupExchange->m_dummy == 0);
    }

    CosKmdResource * pCosKmdResource = NULL;

    if (pCreateAllocation->Flags.Resource)
    {
        if (pCreateAllocation->hResource == NULL)
        {
            pCosKmdResource = (CosKmdResource *)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(CosKmdResource), 'COSD');
            if (!pCosKmdResource)
            {
                COS_LOG_LOW_MEMORY(
                    "Failed to allocate nonpaged pool for sizeof(CosKmdResource) structure. (sizeof(CosKmdResource)=%d)",
                    sizeof(CosKmdResource));
                return STATUS_NO_MEMORY;
            }
            pCosKmdResource->m_dummy = 0;
        }
        else
        {
            pCosKmdResource = (CosKmdResource *)pCreateAllocation->hResource;
        }
    }

    NT_ASSERT(pCreateAllocation->NumAllocations == 1);

    DXGK_ALLOCATIONINFO * pAllocationInfo = pCreateAllocation->pAllocationInfo;

    NT_ASSERT(pAllocationInfo->PrivateDriverDataSize == sizeof(CosAllocationExchange));
    CosAllocationExchange * pCosAllocation = (CosAllocationExchange *)pAllocationInfo->pPrivateDriverData;

    if (pCosAllocation->m_magic != CosAllocationExchange::kMagic)
        COS_LOG_ERROR("Allocation magic");

    CosKmdAllocation * pCosKmdAllocation = (CosKmdAllocation *)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(CosKmdAllocation), 'COSD');
    if (!pCosKmdAllocation)
    {
        if (pCosKmdResource != NULL) ExFreePoolWithTag(pCosKmdResource, 'COSD');

        COS_LOG_ERROR(
            "Failed to allocated nonpaged pool for CosKmdAllocation. (sizeof(CosKmdAllocation)=%d)",
            sizeof(CosKmdAllocation));
        return STATUS_NO_MEMORY;
    }

    *(CosAllocationExchange *)pCosKmdAllocation = *pCosAllocation;

    pAllocationInfo->hAllocation = pCosKmdAllocation;

    pAllocationInfo->Alignment = 64;
    pAllocationInfo->AllocationPriority = D3DDDI_ALLOCATIONPRIORITY_NORMAL;
    pAllocationInfo->EvictionSegmentSet = 0; // don't use apperture for eviction

    pAllocationInfo->FlagsWddm2.Value = 0;

    //
    // Set CpuVisible per UMD request
    //
    pAllocationInfo->FlagsWddm2.CpuVisible = pCosKmdAllocation->m_cpuVisible;

    //
    // TODO: Investigate how to support Cached allocation 
    //
    // pAllocationInfo->FlagsWddm2.Cached = true;

    pAllocationInfo->HintedBank.Value = 0;
    pAllocationInfo->MaximumRenamingListLength = 0;
    pAllocationInfo->pAllocationUsageHint = NULL;
    pAllocationInfo->PhysicalAdapterIndex = 0;
    pAllocationInfo->PitchAlignedSize = 0;
    pAllocationInfo->PreferredSegment.Value = 0;
    pAllocationInfo->PreferredSegment.SegmentId0 = COSD_SEGMENT_VIDEO_MEMORY;
    pAllocationInfo->PreferredSegment.Direction0 = 0;

    // zero-size allocations are not allowed
    NT_ASSERT(pCosAllocation->m_hwSizeBytes != 0);
    pAllocationInfo->Size = pCosAllocation->m_hwSizeBytes;

    pAllocationInfo->SupportedReadSegmentSet = 1 << (COSD_SEGMENT_VIDEO_MEMORY - 1);
    pAllocationInfo->SupportedWriteSegmentSet = 1 << (COSD_SEGMENT_VIDEO_MEMORY - 1);

    if (pCreateAllocation->Flags.Resource && pCreateAllocation->hResource == NULL && pCosKmdResource != NULL)
    {
        pCreateAllocation->hResource = pCosKmdResource;
    }

    COS_LOG_TRACE(
        "Created allocation. (Flags.CpuVisible=%d, Flags.Cacheable=%d, Size=%Id)",
        pAllocationInfo->Flags.CpuVisible,
        pAllocationInfo->Flags.Cached,
        pAllocationInfo->Size);

    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::DestroyAllocation(
    IN_CONST_PDXGKARG_DESTROYALLOCATION     pDestroyAllocation)
{
    CosKmdResource * pCosKmdResource = NULL;

    if (pDestroyAllocation->Flags.DestroyResource)
    {
        pCosKmdResource = (CosKmdResource *)pDestroyAllocation->hResource;
    }

    if (pDestroyAllocation->NumAllocations)
    {
        CosKmdAllocation * pCosKmdAllocation = (CosKmdAllocation *)pDestroyAllocation->pAllocationList[0];

        ExFreePoolWithTag(pCosKmdAllocation, 'COSD');
    }

    if (pCosKmdResource != NULL)
    {
        ExFreePoolWithTag(pCosKmdResource, 'COSD');
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::QueryAdapterInfo(
    IN_CONST_PDXGKARG_QUERYADAPTERINFO      pQueryAdapterInfo)
{
    COS_LOG_TRACE(
        "QueryAdapterInfo was called. (Type=%d)",
        pQueryAdapterInfo->Type);

    switch (pQueryAdapterInfo->Type)
    {
    case DXGKQAITYPE_UMDRIVERPRIVATE:
    {
        if (pQueryAdapterInfo->OutputDataSize < sizeof(COSADAPTERINFO))
        {
            COS_LOG_ERROR(
                "Output buffer is too small. (pQueryAdapterInfo->OutputDataSize=%d, sizeof(COSADAPTERINFO)=%d)",
                pQueryAdapterInfo->OutputDataSize,
                sizeof(COSADAPTERINFO));
            return STATUS_BUFFER_TOO_SMALL;
        }
        COSADAPTERINFO* pCosAdapterInfo = (COSADAPTERINFO*)pQueryAdapterInfo->pOutputData;

        pCosAdapterInfo->m_version = COSD_VERSION;
        pCosAdapterInfo->m_wddmVersion = m_WDDMVersion;

        // Software APCI device only claims an interrupt resource
#if VC4
        pCosAdapterInfo->m_isSoftwareDevice = (m_flags.m_isVC4 != 1);
#else
        pCosAdapterInfo->m_isSoftwareDevice = TRUE;
#endif

        RtlCopyMemory(
            pCosAdapterInfo->m_deviceId,
            m_deviceId,
            m_deviceIdLength);
    }
    break;

    case DXGKQAITYPE_DRIVERCAPS:
    {
        if (pQueryAdapterInfo->OutputDataSize < sizeof(DXGK_DRIVERCAPS))
        {
            COS_ASSERTION(
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
            COS_ASSERTION(
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
            pSegmentInfo->PagingBufferPrivateDataSize = sizeof(COSUMDDMAPRIVATEDATA2);

            pSegmentInfo->PagingBufferSegmentId = COSD_SEGMENT_APERTURE;
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

            pSegmentDesc[0].BaseAddress.QuadPart = COSD_SEGMENT_APERTURE_BASE_ADDRESS;

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
            pSegmentDesc[1].CpuTranslatedAddress = CosKmdGlobal::s_videoMemoryPhysicalAddress; // cpu base physical address
#if !VC4
            pSegmentDesc[1].Size = kVidMemSegementSize;
#else
            pSegmentDesc[1].Size = m_localVidMemSegmentSize;
#endif

        }
    }
    break;

    case DXGKQAITYPE_QUERYSEGMENT4:
    {
        if (pQueryAdapterInfo->OutputDataSize < sizeof(DXGK_QUERYSEGMENTOUT4))
        {
            COS_ASSERTION(
                "Output buffer is too small. (pQueryAdapterInfo->OutputDataSize=%d, sizeof(DXGK_QUERYSEGMENTOUT4)=%d)",
                pQueryAdapterInfo->OutputDataSize,
                sizeof(DXGK_QUERYSEGMENTOUT4));
            return STATUS_BUFFER_TOO_SMALL;
        }

        DXGK_QUERYSEGMENTOUT4   *pSegmentInfo = (DXGK_QUERYSEGMENTOUT4*)pQueryAdapterInfo->pOutputData;

        if (!pSegmentInfo[0].pSegmentDescriptor)
        {
            pSegmentInfo->NbSegment = 1;
        }
        else
        {
            //
            // Private data size should be the maximum of UMD and KMD and the same size must
            // be reported in DxgkDdiCreateContext for paging engine
            //
            pSegmentInfo->PagingBufferPrivateDataSize = sizeof(COSUMDDMAPRIVATEDATA2);

            pSegmentInfo->PagingBufferSegmentId = 0;    // Use physical contiguous memory
            pSegmentInfo->PagingBufferSize = PAGE_SIZE;

            //
            // Setup local video memory segment
            //
            DXGK_SEGMENTDESCRIPTOR4 *pLocalVidMemSegmentDesc = (DXGK_SEGMENTDESCRIPTOR4 *)(pSegmentInfo->pSegmentDescriptor);

            memset(pLocalVidMemSegmentDesc, 0, sizeof(*pLocalVidMemSegmentDesc));

            pLocalVidMemSegmentDesc->BaseAddress.QuadPart = 0LL; // Gpu base physical address
            pLocalVidMemSegmentDesc->Flags.CpuVisible = true;
            pLocalVidMemSegmentDesc->Flags.CacheCoherent = true;
            pLocalVidMemSegmentDesc->Flags.DirectFlip = true;
            pLocalVidMemSegmentDesc->CpuTranslatedAddress = CosKmdGlobal::s_videoMemoryPhysicalAddress; // cpu base physical address
            pLocalVidMemSegmentDesc->Size = kVidMemSegementSize;
        }
    }
    break;

    case DXGKQAITYPE_NUMPOWERCOMPONENTS:
    {
        if (pQueryAdapterInfo->OutputDataSize != sizeof(UINT))
        {
            COS_ASSERTION(
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
            COS_ASSERTION(
                "Input buffer is not of the expected size. (pQueryAdapterInfo->InputDataSize=%d, sizeof(UINT)=%d)",
                pQueryAdapterInfo->InputDataSize,
                sizeof(UINT));
            return STATUS_INVALID_PARAMETER;
        }

        if (pQueryAdapterInfo->OutputDataSize < sizeof(DXGK_POWER_RUNTIME_COMPONENT))
        {
            COS_ASSERTION(
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
            COS_LOG_ERROR(
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
        COS_LOG_WARNING(
            "Unsupported query type. (pQueryAdapterInfo->Type=%d, pQueryAdapterInfo=0x%p)",
            pQueryAdapterInfo->Type,
            pQueryAdapterInfo);
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::DescribeAllocation(
    INOUT_PDXGKARG_DESCRIBEALLOCATION       pDescribeAllocation)
{
    CosKmdAllocation *pAllocation = (CosKmdAllocation *)pDescribeAllocation->hAllocation;

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
CosKmAdapter::GetNodeMetadata(
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
CosKmAdapter::SubmitCommandVirtual(
    IN_CONST_PDXGKARG_SUBMITCOMMANDVIRTUAL  /*pSubmitCommandVirtual*/)
{
    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::PreemptCommand(
    IN_CONST_PDXGKARG_PREEMPTCOMMAND    /*pPreemptCommand*/)
{
    COS_LOG_WARNING("Not implemented");
    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::RestartFromTimeout(void)
{
    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::CancelCommand(
    IN_CONST_PDXGKARG_CANCELCOMMAND /*pCancelCommand*/)
{
    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::QueryCurrentFence(
    INOUT_PDXGKARG_QUERYCURRENTFENCE pCurrentFence)
{
    COS_LOG_WARNING("Not implemented");

    NT_ASSERT(pCurrentFence->NodeOrdinal == 0);
    NT_ASSERT(pCurrentFence->EngineOrdinal == 0);

    pCurrentFence->CurrentFence = 0;
    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::ResetEngine(
    INOUT_PDXGKARG_RESETENGINE  /*pResetEngine*/)
{
    COS_LOG_WARNING("Not implemented");
    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::CollectDbgInfo(
    IN_CONST_PDXGKARG_COLLECTDBGINFO        /*pCollectDbgInfo*/)
{
    COS_LOG_WARNING("Not implemented");
    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::CreateProcess(
    IN DXGKARG_CREATEPROCESS* /*pArgs*/)
{
    // pArgs->hKmdProcess = 0;
    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::DestroyProcess(
    IN HANDLE /*KmdProcessHandle*/)
{
    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

void
CosKmAdapter::SetStablePowerState(
    IN_CONST_PDXGKARG_SETSTABLEPOWERSTATE  pArgs)
{
    UNREFERENCED_PARAMETER(pArgs);
    COS_ASSERTION("Not implemented");
}

NTSTATUS
CosKmAdapter::CalibrateGpuClock(
    IN UINT32                                   /*NodeOrdinal*/,
    IN UINT32                                   /*EngineOrdinal*/,
    OUT_PDXGKARG_CALIBRATEGPUCLOCK              /*pClockCalibration*/
    )
{
    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::Escape(
    IN_CONST_PDXGKARG_ESCAPE        pEscape)
{
    NTSTATUS        Status;

    if (pEscape->PrivateDriverDataSize < sizeof(UINT))
    {
        COS_LOG_ERROR(
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

    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::ResetFromTimeout(void)
{
    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::QueryChildRelations(
    INOUT_PDXGK_CHILD_DESCRIPTOR    ChildRelations,
    IN_ULONG                        ChildRelationsSize)
{
    ChildRelations;
    ChildRelationsSize;

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::QueryChildStatus(
    IN_PDXGK_CHILD_STATUS   ChildStatus,
    IN_BOOLEAN              NonDestructiveOnly)
{
    ChildStatus;
    NonDestructiveOnly;

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::QueryDeviceDescriptor(
    IN_ULONG                        ChildUid,
    INOUT_PDXGK_DEVICE_DESCRIPTOR   pDeviceDescriptor)
{
    ChildUid;
    pDeviceDescriptor;

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
CosKmAdapter::NotifyAcpiEvent(
    IN_DXGK_EVENT_TYPE  EventType,
    IN_ULONG            Event,
    IN_PVOID            Argument,
    OUT_PULONG          AcpiFlags)
{
    EventType;
    Event;
    Argument;
    AcpiFlags;

    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

void
CosKmAdapter::ResetDevice(void)
{
    // Do nothing
    COS_ASSERTION("Not implemented");
}

void
CosKmAdapter::PatchDmaBuffer(
    COSDMABUFINFO*                  pDmaBufInfo,
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

        CosKmdDeviceAllocation * pCosKmdDeviceAllocation = (CosKmdDeviceAllocation *)allocation->hDeviceSpecificAllocation;

        if (allocation->SegmentId != 0)
        {
            DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "Patch CosKmdDeviceAllocation %lx at %lx\n", pCosKmdDeviceAllocation, allocation->PhysicalAddress);
            DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "Patch buffer offset %lx allocation offset %lx\n", patch->PatchOffset, patch->AllocationOffset);

            // Patch in dma buffer
            NT_ASSERT(allocation->SegmentId == COSD_SEGMENT_VIDEO_MEMORY);
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
                    CosKmdGlobal::s_videoMemoryPhysicalAddress.LowPart +
                    allocation->PhysicalAddress.LowPart +
                    patch->AllocationOffset;

                switch (patch->SlotId)
                {
                case VC4_SLOT_RT_BINNING_CONFIG:
                    pDmaBufInfo->m_RenderTargetPhysicalAddress = physicalAddress;
                    pDmaBufInfo->m_RenderTargetVirtualAddress = 
                        static_cast<const BYTE*>(CosKmdGlobal::s_pVideoMemory) +
                        allocation->PhysicalAddress.LowPart +
                        patch->AllocationOffset;
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
#else
                UINT    physicalAddress =
                    CosKmdGlobal::s_videoMemoryPhysicalAddress.LowPart +
                    allocation->PhysicalAddress.LowPart +
                    patch->AllocationOffset;

                *((UINT *)(pDmaBuf + patch->PatchOffset)) = physicalAddress;
#endif
            }
        }
    }
}

//
// TODO[indyz]: Add proper validation for DMA buffer
//
bool
CosKmAdapter::ValidateDmaBuffer(
    COSDMABUFINFO*                  pDmaBufInfo,
    CONST DXGK_ALLOCATIONLIST*      pAllocationList,
    UINT                            allocationListSize,
    CONST D3DDDI_PATCHLOCATIONLIST* pPatchLocationList,
    UINT                            patchAllocationList)
{
    PBYTE           pDmaBuf = (PBYTE)pDmaBufInfo->m_pDmaBuffer;
    bool            bValidateDmaBuffer = true;
#if VC4
    COSDMABUFSTATE* pDmaBufState = &pDmaBufInfo->m_DmaBufState;
#endif

    pDmaBuf;
    pAllocationList;

    if (! pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer)
    {
        for (UINT i = 0; i < patchAllocationList; i++)
        {
            auto patch = &pPatchLocationList[i];

            allocationListSize;
            if (patch->AllocationIndex >= allocationListSize)
            {
                NT_ASSERT(false);
                return false;
            }

#if VC4
            auto allocation = &pAllocationList[patch->AllocationIndex];

            CosKmdDeviceAllocation * pCosKmdDeviceAllocation = (CosKmdDeviceAllocation *)allocation->hDeviceSpecificAllocation;


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
                    pDmaBufInfo->m_pRenderTarget = pCosKmdDeviceAllocation->m_pCosKmdAllocation;
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

#if VC4
        if ((0 == pDmaBufState->m_bRenderTargetRef) ||
            (0 == pDmaBufState->m_bTileAllocMemRef) ||
            (0 == pDmaBufState->m_bTileStateDataRef))
        {
            bValidateDmaBuffer = false;
        }
#endif

    }

    return bValidateDmaBuffer;
}

void
CosKmAdapter::QueueDmaBuffer(
    IN_CONST_PDXGKARG_SUBMITCOMMAND pSubmitCommand)
{
    COSDMABUFINFO *         pDmaBufInfo = (COSDMABUFINFO *)pSubmitCommand->pDmaBufferPrivateData;
    KIRQL                   OldIrql;
    COSDMABUFSUBMISSION *   pDmaBufSubmission;

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

    pDmaBufSubmission = CONTAINING_RECORD(RemoveHeadList(&m_dmaBufSubmissionFree), COSDMABUFSUBMISSION, m_QueueEntry);

    pDmaBufSubmission->m_pDmaBufInfo = pDmaBufInfo;

    pDmaBufSubmission->m_StartOffset = pSubmitCommand->DmaBufferSubmissionStartOffset;
    pDmaBufSubmission->m_EndOffset = pSubmitCommand->DmaBufferSubmissionEndOffset;
    pDmaBufSubmission->m_SubmissionFenceId = pSubmitCommand->SubmissionFenceId;

    InsertTailList(&m_dmaBufQueue, &pDmaBufSubmission->m_QueueEntry);

    KeReleaseSpinLock(&m_dmaBufQueueLock, OldIrql);
}

void
CosKmAdapter::HwDmaBufCompletionDpcRoutine(
    KDPC   *pDPC,
    PVOID   deferredContext,
    PVOID   systemArgument1,
    PVOID   systemArgument2)
{
    CosKmAdapter   *pCosKmAdapter = CosKmAdapter::Cast(deferredContext);

    UNREFERENCED_PARAMETER(pDPC);
    UNREFERENCED_PARAMETER(systemArgument1);
    UNREFERENCED_PARAMETER(systemArgument2);

    // Signal to the worker thread that a HW DMA buffer has completed
    KeSetEvent(&pCosKmAdapter->m_hwDmaBufCompletionEvent, 0, FALSE);
}

COS_NONPAGED_SEGMENT_BEGIN; //================================================

_Use_decl_annotations_
NTSTATUS CosKmAdapter::SetVidPnSourceAddress (
    const DXGKARG_SETVIDPNSOURCEADDRESS* SetVidPnSourceAddressPtr
    )
{
    SetVidPnSourceAddressPtr;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

COS_NONPAGED_SEGMENT_END; //==================================================
COS_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
NTSTATUS CosKmAdapter::QueryInterface (QUERY_INTERFACE* Args)
{
    COS_LOG_WARNING(
        "Received QueryInterface for unsupported interface. (InterfaceType=%!GUID!)",
        Args->InterfaceType);
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::GetStandardAllocationDriverData (
    DXGKARG_GETSTANDARDALLOCATIONDRIVERDATA* Args
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    //
    // ResourcePrivateDriverDataSize gets passed to CreateAllocation as
    // PrivateDriverDataSize.
    // AllocationPrivateDriverDataSize get passed to CreateAllocation as
    // pAllocationInfo->PrivateDriverDataSize.
    //

    if (!Args->pResourcePrivateDriverData && !Args->pResourcePrivateDriverData)
    {
        Args->ResourcePrivateDriverDataSize = sizeof(CosAllocationGroupExchange);
        Args->AllocationPrivateDriverDataSize = sizeof(CosAllocationExchange);
        return STATUS_SUCCESS;
    }

    // we expect them to both be null or both be valid
    NT_ASSERT(Args->pResourcePrivateDriverData && Args->pResourcePrivateDriverData);
    NT_ASSERT(
        Args->ResourcePrivateDriverDataSize ==
        sizeof(CosAllocationGroupExchange));

    NT_ASSERT(
        Args->AllocationPrivateDriverDataSize ==
        sizeof(CosAllocationExchange));

    new (Args->pResourcePrivateDriverData) CosAllocationGroupExchange();
    auto allocParams = new (Args->pAllocationPrivateDriverData) CosAllocationExchange();

    allocParams->m_magic = CosAllocationExchange::kMagic;
    allocParams->m_shared = false;
    allocParams->m_cpuVisible = false;

    switch (Args->StandardAllocationType)
    {
    case D3DKMDT_STANDARDALLOCATION_SHAREDPRIMARYSURFACE:
    {
        const D3DKMDT_SHAREDPRIMARYSURFACEDATA* surfData =
                Args->pCreateSharedPrimarySurfaceData;

        COS_LOG_TRACE(
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
        allocParams->m_mip0Info.TexelDepth = 1;
        allocParams->m_mip0Info.PhysicalWidth = surfData->Width;
        allocParams->m_mip0Info.PhysicalHeight = surfData->Height;
        allocParams->m_mip0Info.PhysicalDepth = 1;

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

        allocParams->m_hwLayout = CosHwLayout::Linear;
        allocParams->m_hwWidthPixels = surfData->Width;
        allocParams->m_hwHeightPixels = surfData->Height;

        NT_ASSERT(surfData->Format == D3DDDIFMT_A8R8G8B8);
        allocParams->m_hwSizeBytes = surfData->Width * 4 * surfData->Height;

        allocParams->m_dataSize = surfData->Width * 4 * surfData->Height;
        allocParams->m_textureLayout = D3D12DDI_TL_ROW_MAJOR;

        return STATUS_SUCCESS;
    }
    case D3DKMDT_STANDARDALLOCATION_SHADOWSURFACE:
    {
        const D3DKMDT_SHADOWSURFACEDATA* surfData = Args->pCreateShadowSurfaceData;
        COS_LOG_TRACE(
            "Preparing private allocation data for SHADOWSURFACE. (Width=%d, Height=%d, Format=%d)",
            surfData->Width,
            surfData->Height,
            surfData->Format);

        allocParams->m_resourceDimension = D3D10DDIRESOURCE_TEXTURE2D;
        allocParams->m_mip0Info.TexelWidth = surfData->Width;
        allocParams->m_mip0Info.TexelHeight = surfData->Height;
        allocParams->m_mip0Info.TexelDepth = 1;
        allocParams->m_mip0Info.PhysicalWidth = surfData->Width;
        allocParams->m_mip0Info.PhysicalHeight = surfData->Height;
        allocParams->m_mip0Info.PhysicalDepth = 1;
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
        allocParams->m_hwLayout = CosHwLayout::Linear;
        allocParams->m_hwWidthPixels = surfData->Width;
        allocParams->m_hwHeightPixels = surfData->Height;

        NT_ASSERT(surfData->Format == D3DDDIFMT_A8R8G8B8);
        allocParams->m_hwSizeBytes = surfData->Width * 4 * surfData->Height;

        Args->pCreateShadowSurfaceData->Pitch = surfData->Width * 4; //allocParams->m_hwPitchBytes;

        allocParams->m_dataSize = surfData->Width * 4 * surfData->Height;
        allocParams->m_textureLayout = D3D12DDI_TL_ROW_MAJOR;

        return STATUS_SUCCESS;
    }
    case D3DKMDT_STANDARDALLOCATION_STAGINGSURFACE:
    {
        const D3DKMDT_STAGINGSURFACEDATA* surfData = Args->pCreateStagingSurfaceData;
        COS_ASSERTION(
            "STAGINGSURFACEDATA is not implemented. (Width=%d, Height=%d, Pitch=%d)",
            surfData->Width,
            surfData->Height,
            surfData->Pitch);
        return STATUS_NOT_IMPLEMENTED;
    }
    case D3DKMDT_STANDARDALLOCATION_GDISURFACE:
    {
        D3DKMDT_GDISURFACEDATA* surfData = Args->pCreateGdiSurfaceData;

        if (surfData->Type == D3DKMDT_GDISURFACE_TEXTURE_CROSSADAPTER ||
            surfData->Type == D3DKMDT_GDISURFACE_TEXTURE_CPUVISIBLE_CROSSADAPTER)
        {
            if (surfData->Format == D3DDDIFMT_UNKNOWN)
            {
                //
                // DX12 cross-adapter heap
                //

                allocParams->m_resourceDimension = D3D10DDIRESOURCE_BUFFER;
                allocParams->m_mip0Info.TexelWidth = surfData->Width;
                allocParams->m_mip0Info.TexelHeight = surfData->Height;
                allocParams->m_mip0Info.TexelDepth = 1;
                allocParams->m_mip0Info.PhysicalWidth = surfData->Width;
                allocParams->m_mip0Info.PhysicalHeight = surfData->Height;
                allocParams->m_mip0Info.PhysicalDepth = 1;
                allocParams->m_usage = D3D10_DDI_USAGE_DEFAULT;     // TODO: what should usage be?

                allocParams->m_bindFlags = D3D10_DDI_BIND_PIPELINE_MASK;

                allocParams->m_mapFlags = D3D10_DDI_MAP_READWRITE;
                allocParams->m_miscFlags = D3D10_DDI_RESOURCE_MISC_SHARED;

                allocParams->m_format = DxgiFormatFromD3dDdiFormat(surfData->Format);
                allocParams->m_sampleDesc.Count = 1;
                allocParams->m_sampleDesc.Quality = 0;
                allocParams->m_mipLevels = 1;
                allocParams->m_arraySize = 1;
                allocParams->m_isPrimary = false;

                allocParams->m_hwLayout = CosHwLayout::Linear;
                allocParams->m_hwWidthPixels = surfData->Width;
                allocParams->m_hwHeightPixels = surfData->Height;

                allocParams->m_hwSizeBytes = surfData->Width;

                allocParams->m_shared = true;
                allocParams->m_cpuVisible = true;   // TODO: check that these will always be CPU visible
                
                allocParams->m_dataSize = surfData->Width;
                allocParams->m_textureLayout = D3D12DDI_TL_ROW_MAJOR;

                surfData->Pitch = surfData->Width;
            }
            else
            {
                //
                // Cross adapter resource should have its pitch aligned to a pre-defined value.
                //

                COS_ASSERTION("Needs to be implemented");
                return STATUS_NOT_IMPLEMENTED;
            }

        } else {
            COS_ASSERTION("Needs to be implemented");
            return STATUS_NOT_IMPLEMENTED;
        }
         

        return STATUS_SUCCESS;
    }
    default:
        COS_ASSERTION(
            "Unknown standard allocation type. (StandardAllocationType=%d)",
            Args->StandardAllocationType);
        return STATUS_INVALID_PARAMETER;
    }
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::SetPalette (const DXGKARG_SETPALETTE* /*SetPalettePtr*/)
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    COS_ASSERTION("Not implemented.");
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::SetPointerPosition (
    const DXGKARG_SETPOINTERPOSITION* SetPointerPositionPtr
    )
{
    SetPointerPositionPtr;

    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::SetPointerShape (
    const DXGKARG_SETPOINTERSHAPE* SetPointerShapePtr
    )
{
    SetPointerShapePtr;

    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::IsSupportedVidPn (
    DXGKARG_ISSUPPORTEDVIDPN* IsSupportedVidPnPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    IsSupportedVidPnPtr;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::RecommendFunctionalVidPn (
    const DXGKARG_RECOMMENDFUNCTIONALVIDPN* const RecommendFunctionalVidPnPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    RecommendFunctionalVidPnPtr;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::EnumVidPnCofuncModality (
    const DXGKARG_ENUMVIDPNCOFUNCMODALITY* const EnumCofuncModalityPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    EnumCofuncModalityPtr;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::SetVidPnSourceVisibility (
    const DXGKARG_SETVIDPNSOURCEVISIBILITY* SetVidPnSourceVisibilityPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    SetVidPnSourceVisibilityPtr;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::CommitVidPn (
    const DXGKARG_COMMITVIDPN* const CommitVidPnPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    CommitVidPnPtr;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::UpdateActiveVidPnPresentPath (
    const DXGKARG_UPDATEACTIVEVIDPNPRESENTPATH* const UpdateActiveVidPnPresentPathPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    UpdateActiveVidPnPresentPathPtr;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::RecommendMonitorModes (
    const DXGKARG_RECOMMENDMONITORMODES* const RecommendMonitorModesPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    RecommendMonitorModesPtr;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::GetScanLine (DXGKARG_GETSCANLINE* /*GetScanLinePtr*/)
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::ControlInterrupt (
    const DXGK_INTERRUPT_TYPE InterruptType,
    BOOLEAN EnableInterrupt
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    InterruptType;
    EnableInterrupt;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::QueryVidPnHWCapability (
    DXGKARG_QUERYVIDPNHWCAPABILITY* VidPnHWCapsPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    VidPnHWCapsPtr;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::QueryDependentEngineGroup (
    DXGKARG_QUERYDEPENDENTENGINEGROUP* ArgsPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(ArgsPtr->NodeOrdinal == 0);
    NT_ASSERT(ArgsPtr->EngineOrdinal == 0);

    ArgsPtr->DependentNodeOrdinalMask = 0;
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::StopDeviceAndReleasePostDisplayOwnership (
    D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId,
    DXGK_DISPLAY_INFORMATION* DisplayInfoPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    DisplayInfoPtr;
    TargetId;

    NT_ASSERT(!CosKmdGlobal::IsRenderOnly());
    return STATUS_NOT_SUPPORTED;
}


COS_PAGED_SEGMENT_END; //=====================================================
