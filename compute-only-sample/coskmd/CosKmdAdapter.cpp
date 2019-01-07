
#include "CosKmd.h"

#include "CosKmdLogging.h"
#include "CosKmdAdapter.tmh"

#include "CosKmdAdapter.h"
#include "CosKmdSoftAdapter.h"
#include "CosKmdAllocation.h"
#include "CosKmdContext.h"
#include "CosKmdResource.h"
#include "CosKmdProcess.h"
#include "CosKmdGlobal.h"
#include "CosKmdUtil.h"
#include "CosGpuCommand.h"
#include "CosKmdAcpi.h"
#include "CosKmdUtil.h"

#include "bugcodes.h"

//
// Global variable to set in kernel debugger for triggering Preemption
//

BOOLEAN g_bTriggerPreemption = FALSE;

//
// 60 seconds : Stall duration for "preemptable" long running DMA buffer
//

#define COS_DMA_BUF_STALL_DURATION  -60*1000*1000*10L

//
// Global variable to set in kernel debugger for triggering Engine Reset (lightweight reset)
//

BOOLEAN g_bTriggerEngineReset = false;

//
// Global variable to set in kernel debugger for triggering TDR (heavyweight reset)
//

BOOLEAN g_bTriggerTDR = false;

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

void CosKmAdapter::WorkerThread(void * inThis)
{
    CosKmAdapter  *pCosKmAdapter = CosKmAdapter::Cast(inThis);

    pCosKmAdapter->DoWork();
}

void CosKmAdapter::DoWork(void)
{
    bool done = false;
    KEVENT dmaBufStallEvent;

    KeInitializeEvent(&dmaBufStallEvent, SynchronizationEvent, FALSE);

    while (!done)
    {
        PVOID       waitEvents[3];
        NTSTATUS    status;
        
        waitEvents[0] = &m_workerThreadEvent;
        waitEvents[1] = &m_preemptionEvent;
        waitEvents[2] = &m_resetRequestEvent;

        status = KeWaitForMultipleObjects(
                    ARRAYSIZE(waitEvents),
                    waitEvents,
                    WaitAny,
                    Executive,
                    KernelMode,
                    FALSE,          // Alertable
                    NULL,
                    NULL);

        if (STATUS_WAIT_1 == status)
        {
            //
            // Notify completion of Preemption request
            //

            NotifyPreemptionCompletion();

            continue;
        }
        if (STATUS_WAIT_2 == status)
        {
            EmptyDmaBufferQueue();

            //
            // Signal back to the waiting DDI thread
            //

            KeSetEvent(&m_resetCompletionEvent, 0, FALSE);

            continue;
        }

        NT_ASSERT(status == STATUS_WAIT_0);

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

            if (0 != pDmaBufInfo->m_DmaBufStallDuration)
            {
                LARGE_INTEGER   timeout, waitStart, waitEnd;
                ULONG64         waitTicks;

                //
                // Simulate long running DMA buffer with stall so that it can be preempted
                //

                waitEvents[0] = &dmaBufStallEvent;
                waitEvents[1] = &m_preemptionEvent;

                timeout.QuadPart = pDmaBufInfo->m_DmaBufStallDuration;

                KeQueryTickCount(&waitStart);

                status =  KeWaitForMultipleObjects(
                            2,
                            waitEvents,
                            WaitAny,
                            Executive,
                            KernelMode,
                            FALSE,          // Alertable
                            &timeout,
                            NULL);

                KeQueryTickCount(&waitEnd);

                if (STATUS_WAIT_1 == status)
                {
                    //
                    // Preemption requested
                    //
                    // HW driver need to save progression state of the preempted DMA buffer for resumption
                    //

                    pDmaBufInfo->m_DmaBufState.m_bPreempted = 1;

                    waitTicks = waitEnd.QuadPart - waitStart.QuadPart;
                    if (0 == waitTicks)
                    {
                        waitTicks = 1;
                    }

                    //
                    // Subtract the time already stalled
                    //

                    pDmaBufInfo->m_DmaBufStallDuration += waitTicks*KeQueryTimeIncrement();
                    if (pDmaBufInfo->m_DmaBufStallDuration > 0)
                    {
                        pDmaBufInfo->m_DmaBufStallDuration = 0;
                    }

                    ExInterlockedInsertTailList(&m_dmaBufSubmissionFree, &pDmaBufSubmission->m_QueueEntry, &m_dmaBufQueueLock);

                    //
                    // Notify completion of Preemption request
                    //

                    NotifyPreemptionCompletion();

                    break;
                }
                else if (STATUS_TIMEOUT == status)
                {
                    pDmaBufInfo->m_DmaBufStallDuration = 0;
                }
                else
                {
                    NT_ASSERT(false);
                }
            }

            if (pDmaBufInfo->m_DmaBufState.m_bPaging)
            {
                //
                // Run paging buffer in software
                //

                ProcessPagingBuffer(pDmaBufSubmission);

            }
#if COS_GPUVA_SUPPORT
            else if (pDmaBufInfo->m_DmaBufState.m_bGpuVaCommandBuffer)
            {
                ProcessGpuVaRenderBuffer(pDmaBufSubmission);
            }
#else
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
#endif

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
CosKmAdapter::EmptyDmaBufferQueue()
{
    KIRQL                   OldIrql;

    KeAcquireSpinLock(&m_dmaBufQueueLock, &OldIrql);

    while (!IsListEmpty(&m_dmaBufQueue))
    {
        LIST_ENTRY *pQueueEntry;

        pQueueEntry = RemoveHeadList(&m_dmaBufQueue);

        InsertTailList(&m_dmaBufSubmissionFree, pQueueEntry);
    }

    KeReleaseSpinLock(&m_dmaBufQueueLock, OldIrql);
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
            NT_ASSERT(pPagingBuffer->Fill.Destination.SegmentId == COS_SEGMENT_VIDEO_MEMORY);
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
            //
            // TODO: Improve paging operation efficiency for HW with DMA engine
            //       for paging operation but not aperture segment.
            //

            PBYTE   pSource, pDestination;
            MDL *   pMdlToRestore = NULL;
            CSHORT  savedMdlFlags = 0;
            PBYTE   pKmAddrToUnmap = NULL;

            if (pPagingBuffer->Transfer.Source.SegmentId == COS_SEGMENT_VIDEO_MEMORY)
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

                // Adjust the source address by TransferOffset % pMdl->ByteCount
                // Handle the situation when smaller Mdl source buffer is "slided" across the destionation
                pSource += (pPagingBuffer->Transfer.TransferOffset % pPagingBuffer->Transfer.Source.pMdl->ByteCount);
            }

            if (pPagingBuffer->Transfer.Destination.SegmentId == COS_SEGMENT_VIDEO_MEMORY)
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

                // Adjust the destination address by TransferOffset % pMdl->ByteCount
                // Handle the situation when smaller Mdl destination buffer is "slided" across the source
                pDestination += (pPagingBuffer->Transfer.TransferOffset % pPagingBuffer->Transfer.Destination.pMdl->ByteCount);
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

    if (pDmaBufSubmission->m_bSimulateHang)
    {
        //
        // Emulate HW hang by NOT reporting DMA buffer completion
        //

        return;
    }

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

    //
    // Keep track of last completed fence ID for Preemption request afterward
    //

    m_lastCompletetdFenceId = pDmaBufSubmission->m_SubmissionFenceId;
}

void
CosKmAdapter::NotifyPreemptionCompletion()
{
    //
    // Remove the queued DMA buffers which will be submitted again
    //

    EmptyDmaBufferQueue();

    //
    // Notify the VidSch of the completion of the Preemption request
    //

    NTSTATUS    Status;

    RtlZeroMemory(&m_interruptData, sizeof(m_interruptData));

    m_interruptData.InterruptType = DXGK_INTERRUPT_DMA_PREEMPTED;
    m_interruptData.DmaPreempted.PreemptionFenceId = m_preemptionRequest.PreemptionFenceId;
    m_interruptData.DmaPreempted.LastCompletedFenceId = m_lastCompletetdFenceId;
    m_interruptData.DmaPreempted.NodeOrdinal = m_preemptionRequest.NodeOrdinal;
    m_interruptData.DmaPreempted.EngineOrdinal = m_preemptionRequest.EngineOrdinal;

    BOOLEAN bRet;

    Status = m_DxgkInterface.DxgkCbSynchronizeExecution(
        m_DxgkInterface.DeviceHandle,
        SynchronizeNotifyInterrupt,
        this,
        0,
        &bRet);

    if (!NT_SUCCESS(Status))
    {
        m_ErrorHit.m_NotifyPreemptionCompletion = 1;
    }

    m_lastCompeletedPreemptionFenceId = m_preemptionRequest.PreemptionFenceId;
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
    // 2.6 model is required for Compute Only devices
    //
    m_WDDMVersion = DXGKDDI_WDDMv2_6;

    m_NumNodes = C_COS_GPU_ENGINE_COUNT;

    //
    // Initialize work thread and Preemption request events
    //

    KeInitializeEvent(&m_workerThreadEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent(&m_preemptionEvent, SynchronizationEvent, FALSE);

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

    //
    // Initialize Fence IDs
    //

    m_lastSubmittedFenceId = 0;
    m_lastCompletetdFenceId = 0;

    m_lastCompeletedPreemptionFenceId = 0;

    //
    // Initialize TDR related fields
    //

    m_bInHangState = false;

    KeInitializeEvent(&m_resetRequestEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent(&m_resetCompletionEvent, SynchronizationEvent, FALSE);

    m_workerExit = false;

    //
    // Initialize worker thread
    //

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

#if COS_GPUVA_SUPPORT

    case DXGK_OPERATION_UPDATE_PAGE_TABLE:
    {
        DXGK_PTE *  pHwPte = (DXGK_PTE *)pArgs->UpdatePageTable.PageTableAddress.CpuVirtual;
        DXGK_PTE *  pOsPte = pArgs->UpdatePageTable.pPageTableEntries;

        //
        // When Repeat bit is set, HW PTEs are replicated from OS PTE
        //
        UINT        osPteInc = pArgs->UpdatePageTable.Flags.Repeat ? 0 : 1;

        pHwPte += pArgs->UpdatePageTable.StartIndex;

        for (UINT i = 0; i < pArgs->UpdatePageTable.NumPageTableEntries; i++)
        {
            *pHwPte = *pOsPte;

            pHwPte++;
            pOsPte += osPteInc;
        }
    }
    break;

    case DXGK_OPERATION_FLUSH_TLB:
    {
        // HW driver should flush the TLB
    }
    break;

#endif

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
    if (pDmaBufInfo && (pArgs->DmaSize == COS_PAGING_BUFFER_SIZE))
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

#if COS_PHYSICAL_SUPPORT
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
#endif

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

    // zero-size allocations are not allowed
    NT_ASSERT(pCosAllocation->m_hwSizeBytes != 0);
    pAllocationInfo->Size = pCosAllocation->m_hwSizeBytes;

#if COS_GPUVA_SUPPORT

    //
    // If the allocation is backed by system memory, then the driver should
    // specify the implicit system memory segment in the allocation info.
    //

    pAllocationInfo->PreferredSegment.Value = 0;
    pAllocationInfo->PreferredSegment.SegmentId0 = IMPLICIT_SYSTEM_MEMORY_SEGMENT_ID;
    pAllocationInfo->PreferredSegment.Direction0 = 0;

    pAllocationInfo->SupportedReadSegmentSet = 1 << (IMPLICIT_SYSTEM_MEMORY_SEGMENT_ID - 1);
    pAllocationInfo->SupportedWriteSegmentSet = 1 << (IMPLICIT_SYSTEM_MEMORY_SEGMENT_ID - 1);

#else

    pAllocationInfo->PreferredSegment.Value = 0;
    pAllocationInfo->PreferredSegment.SegmentId0 = COS_SEGMENT_VIDEO_MEMORY;
    pAllocationInfo->PreferredSegment.Direction0 = 0;

    pAllocationInfo->SupportedReadSegmentSet = 1 << (COS_SEGMENT_VIDEO_MEMORY - 1);
    pAllocationInfo->SupportedWriteSegmentSet = 1 << (COS_SEGMENT_VIDEO_MEMORY - 1);

#endif

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

        pCosAdapterInfo->m_version = COS_VERSION;
        pCosAdapterInfo->m_wddmVersion = m_WDDMVersion;

        // Software APCI device only claims an interrupt resource
        pCosAdapterInfo->m_isSoftwareDevice = TRUE;

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

#if COS_GPUVA_SUPPORT

        pDriverCaps->MemoryManagementCaps.VirtualAddressingSupported = 1;
        pDriverCaps->MemoryManagementCaps.GpuMmuSupported = 1;

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

        //
        // Indicate ComputeOnly adapter
        //
        pDriverCaps->MiscCaps.ComputeOnly = 1;
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

#if COS_GPUVA_SUPPORT

            //
            // Setup aperture segment, which is only used for allocation with AccessedPhysically
            //
            DXGK_SEGMENTDESCRIPTOR4 *pApertureSegmentDesc = (DXGK_SEGMENTDESCRIPTOR4 *)(pSegmentInfo->pSegmentDescriptor);

            memset(pApertureSegmentDesc, 0, sizeof(*pApertureSegmentDesc));

            pApertureSegmentDesc->Flags.Aperture = true;
            pApertureSegmentDesc->Flags.CpuVisible = true;
            pApertureSegmentDesc->Flags.CacheCoherent = true;
            pApertureSegmentDesc->Flags.PitchAlignment = true;
            pApertureSegmentDesc->Flags.PreservedDuringStandby = true;
            pApertureSegmentDesc->Flags.PreservedDuringHibernate = true;

            pApertureSegmentDesc->BaseAddress.QuadPart = COS_APERTURE_SEGMENT_BASE_ADDRESS;    // Gpu base physical address
            pApertureSegmentDesc->CpuTranslatedAddress.QuadPart = 0xFFFFFFFE00000000;           // Cpu base physical address
            pApertureSegmentDesc->Size = COS_APERTURE_SEGMENT_SIZE;

#else

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
            pLocalVidMemSegmentDesc->Size = CosKmdGlobal::s_videoMemorySize;

#endif
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

#if COS_GPUVA_SUPPORT

    case DXGKQAITYPE_GPUMMUCAPS:
    {
        DXGK_GPUMMUCAPS *pGpuMmuCaps = (DXGK_GPUMMUCAPS *)pQueryAdapterInfo->pOutputData;

        memset(pGpuMmuCaps, 0, sizeof(DXGK_GPUMMUCAPS));

        pGpuMmuCaps->ZeroInPteSupported = 1;
        pGpuMmuCaps->CacheCoherentMemorySupported = 1;

        pGpuMmuCaps->PageTableUpdateMode = DXGK_PAGETABLEUPDATE_CPU_VIRTUAL;

        pGpuMmuCaps->VirtualAddressBitCount = COS_GPU_VA_BIT_COUNT;
        pGpuMmuCaps->PageTableLevelCount = COS_PAGE_TABLE_LEVEL_COUNT;
    }
    break;

    case DXGKQAITYPE_PAGETABLELEVELDESC:
    {
        //
        // In GpuVA mode, COSD supports 1 aperture segment with SegmentId of 1.
        // So the implicit system memory segment has the SegmentId of 2.
        //
        // COSD uses page table in system memory, so sets PageTableSegmentId to 2
        //
        // COSD reuses DXGK_PTE for page table entry, so each page table is 16K.
        //

        static DXGK_PAGE_TABLE_LEVEL_DESC s_CosPageTableLevelDesc[2] =
        {
            {
                10,                     // PageTableIndexBitCount
                2,                      // PageTableSegmentId
                2,                      // PagingProcessPageTableSegmentId
                COS_PAGE_TABLE_SIZE,    // PageTableSizeInBytes
                0,                      // PageTableAlignmentInBytes
            },
            {
                10,                     // PageTableIndexBitCount
                2,                      // PageTableSegmentId
                2,                      // PagingProcessPageTableSegmentId
                COS_PAGE_TABLE_SIZE,    // PageTableSizeInBytes
                0,                      // PageTableAlignmentInBytes
            }
        };

        UINT PageTableLevel = *((UINT *)pQueryAdapterInfo->pInputData);
        DXGK_PAGE_TABLE_LEVEL_DESC *pPageTableLevelDesc = (DXGK_PAGE_TABLE_LEVEL_DESC *)pQueryAdapterInfo->pOutputData;

        *pPageTableLevelDesc = s_CosPageTableLevelDesc[PageTableLevel];
    }
    break;

#endif

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

#if COS_GPUVA_SUPPORT

    pGetNodeMetadata->GpuMmuSupported = 1;

#endif

    return STATUS_SUCCESS;
}

#if COS_GPUVA_SUPPORT

NTSTATUS
CosKmAdapter::SubmitCommandVirtual(
    IN_CONST_PDXGKARG_SUBMITCOMMANDVIRTUAL  pSubmitCommandVirtual)
{
    COSDMABUFINFO * pDmaBufInfo = (COSDMABUFINFO *)pSubmitCommandVirtual->pDmaBufferPrivateData;

    //
    // m_pDmaBuffer from UMD is for debugging purpose only
    //

    if (!pSubmitCommandVirtual->Flags.Resubmission)
    {
        pDmaBufInfo->m_DmaBufferGpuVa = pSubmitCommandVirtual->DmaBufferVirtualAddress;
        pDmaBufInfo->m_DmaBufferSize = pSubmitCommandVirtual->DmaBufferSize;

        pDmaBufInfo->m_DmaBufState.m_Value = 0;
        pDmaBufInfo->m_DmaBufState.m_bGpuVaCommandBuffer = 1;

        pDmaBufInfo->m_DmaBufState.m_bPaging = pSubmitCommandVirtual->Flags.Paging;

        pDmaBufInfo->m_DmaBufStallDuration = 0;
    }

    NT_ASSERT(pDmaBufInfo->m_DmaBufState.m_bPreempted == pSubmitCommandVirtual->Flags.Resubmission);

    //
    // TODO : Using flattened parameters for QueueDmaBuffer()
    //

    DXGKARG_SUBMITCOMMAND   submitCommand = { 0 };

    submitCommand.hContext = pSubmitCommandVirtual->hContext;

    submitCommand.DmaBufferPhysicalAddress.QuadPart = pSubmitCommandVirtual->DmaBufferVirtualAddress;
    submitCommand.DmaBufferSize = pSubmitCommandVirtual->DmaBufferSize;
    
    submitCommand.DmaBufferSubmissionStartOffset = 0;
    submitCommand.DmaBufferSubmissionEndOffset = pSubmitCommandVirtual->DmaBufferSize;

    submitCommand.pDmaBufferPrivateData = pDmaBufInfo;
    submitCommand.DmaBufferPrivateDataSize = pSubmitCommandVirtual->DmaBufferPrivateDataSize;

    submitCommand.SubmissionFenceId = pSubmitCommandVirtual->SubmissionFenceId;

    submitCommand.Flags = pSubmitCommandVirtual->Flags;

    submitCommand.EngineOrdinal = pSubmitCommandVirtual->EngineOrdinal;
    submitCommand.NodeOrdinal = pSubmitCommandVirtual->NodeOrdinal;

    QueueDmaBuffer(&submitCommand);

    //
    // Wake up the worker thread for the GPU node
    //
    KeSetEvent(&m_workerThreadEvent, 0, FALSE);

    return S_OK;
}

#endif

NTSTATUS
CosKmAdapter::PreemptCommand(
    IN_CONST_PDXGKARG_PREEMPTCOMMAND    pPreemptCommand)
{
    //
    // Preemption request is non-blocking
    // It is guaranteed that there is only 1 pending preemption request
    //
    // HW driver should issue the request to HW and return
    //
    // Completion of the preemption request should be notified by interrupt
    //
    // Progression state of the preempted DMA buffer should be saved in
    // DMA buffer private data or inside DMA buffer itself
    //

    m_preemptionRequest = *pPreemptCommand;

    KeSetEvent(&m_preemptionEvent, 0, FALSE);

    return STATUS_SUCCESS;
}


NTSTATUS
CosKmAdapter::CancelCommand(
    IN_CONST_PDXGKARG_CANCELCOMMAND /*pCancelCommand*/)
{
    //
    // DMA buffer to be cancelled is guaranteed to be NOT queued in HW
    //
    // If needed, HW driver can take this opportunity to clean up DMA buffer private data
    //

    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::ResetFromTimeout(void)
{
    //
    // DdiResetFromTimeout is blocking, 
    // KMD should finish resetting the adapter before returning.
    //

    KeSetEvent(&m_resetRequestEvent, 0, FALSE);

    KeWaitForSingleObject(
        &m_resetCompletionEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL);

    m_bInHangState = false;

    //
    // Implicitly sync up : Graphics runtime considers all submitted Fence Id as completed.
    //

    m_lastCompletetdFenceId = m_lastSubmittedFenceId;

    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::RestartFromTimeout(void)
{
    //
    // Driver and HW need to get ready to accept and run new DMA buffers
    //

    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::CollectDbgInfo(
    IN_CONST_PDXGKARG_COLLECTDBGINFO    pCollectDbgInfo)
{
    //
    // Driver should collect enough (SW and HW) info for debugging the 
    // lightweight reset (Engine Reset with Reason VIDEO_ENGINE_TIMEOUT_DETECTED) or
    // heavyweight reset (TDR/Full Adapter Reset with Reason VIDEO_TDR_TIMEOUT_DETECTED)
    //

    switch (pCollectDbgInfo->Reason)
    {
    case VIDEO_ENGINE_TIMEOUT_DETECTED:
    case VIDEO_TDR_TIMEOUT_DETECTED:
        if (sizeof(*this) < pCollectDbgInfo->BufferSize)
        {
            memcpy(pCollectDbgInfo->pBuffer, this, sizeof(*this));
        }
        break;
    default:
        NT_ASSERT(false);
        break;
    }

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS CosKmAdapter::QueryDependentEngineGroup(
    DXGKARG_QUERYDEPENDENTENGINEGROUP* ArgsPtr
)
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    NT_ASSERT(ArgsPtr->NodeOrdinal == 0);
    NT_ASSERT(ArgsPtr->EngineOrdinal == 0);

    //
    // COSD supports only a single node (please see C_COS_GPU_ENGINE_COUNT)
    //

    ArgsPtr->DependentNodeOrdinalMask = 1;

    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::QueryEngineStatus(
    DXGKARG_QUERYENGINESTATUS  *pQueryEngineStatus)
{
    COS_LOG_TRACE("QueryEngineStatus was called.");

    //
    // DdiQueryEngineStatus is called when app has requested to disable TDR
    // with D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT on a per D3D12 Command
    // Queue basis or by UMD with DisableGpuTimeout for context creation.
    //
    // A long running DMA buffer should be preemptable to yield to other DMA buffer
    // and driver should be able to return its progression here.
    //

    pQueryEngineStatus->EngineStatus.Responsive = 1;

    return STATUS_SUCCESS;
}

NTSTATUS
CosKmAdapter::ResetEngine(
    INOUT_PDXGKARG_RESETENGINE  pResetEngine)
{
    NT_ASSERT(pResetEngine->NodeOrdinal == 0);
    NT_ASSERT(pResetEngine->EngineOrdinal == 0);

    //
    // DdiResetEngine is blocking, 
    // KMD should finish resetting the engine before returning.
    //

    KeSetEvent(&m_resetRequestEvent, 0, FALSE);

    KeWaitForSingleObject(
        &m_resetCompletionEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL);

    m_bInHangState = false;

    //
    // Use the Fence Id for the last Submited but un-Completed DMA buffer
    //

    pResetEngine->LastAbortedFenceId = m_lastSubmittedFenceId;

    //
    // Implicitly sync up : Graphics runtime considers all submitted Fence Id as completed.
    //

    m_lastCompletetdFenceId = m_lastSubmittedFenceId;

    //
    // Except for paging node, TDR (heavyweight reset) is attempted to recover
    // from ResetEngine (lightweight reset) failure
    //

    return STATUS_SUCCESS;
}

// TODO: Should this only be defined if COS_GPUVA_SUPPORT?

NTSTATUS
CosKmAdapter::CreateProcess(
    IN DXGKARG_CREATEPROCESS   *pArgs)
{
#if COS_GPUVA_SUPPORT

    CosKmdProcess *  pCosKmdProcess;

    pCosKmdProcess = (CosKmdProcess *)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(CosKmdProcess), 'COSD');
    if (NULL == pCosKmdProcess)
    {
        return STATUS_NO_MEMORY;
    }

    pCosKmdProcess->m_dummy = 0;

    pArgs->hKmdProcess = pCosKmdProcess;

    return STATUS_SUCCESS;

#else

    pArgs->hKmdProcess = 0;
    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;

#endif
}

NTSTATUS
CosKmAdapter::DestroyProcess(
    IN HANDLE KmdProcessHandle)
{
    ExFreePoolWithTag(KmdProcessHandle, 'COSD');

    return STATUS_SUCCESS;
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

NTSTATUS
CosKmAdapter::SetVirtualMachineData(
    IN_CONST_PDXGKARG_SETVIRTUALMACHINEDATA pArgs)
{
    pArgs;

    COS_ASSERTION("Not implemented");
    return STATUS_NOT_IMPLEMENTED;
}

#if COS_PHYSICAL_SUPPORT
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
            NT_ASSERT(allocation->SegmentId == COS_SEGMENT_VIDEO_MEMORY);
            if (pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer)
            {
                PHYSICAL_ADDRESS    allocAddress;

                allocAddress.QuadPart = allocation->PhysicalAddress.QuadPart + (LONGLONG)patch->AllocationOffset;
                *((PHYSICAL_ADDRESS *)(pDmaBuf + patch->PatchOffset)) = allocAddress;
            }
            else
            {
				UINT64 physicalOffset = (UINT64) allocation->PhysicalAddress.QuadPart + patch->AllocationOffset;

				NT_ASSERT(physicalOffset < CosKmdGlobal::s_videoMemorySize);

                // Patch HW command buffer
				UINT64    physicalAddress = (UINT64) CosKmdGlobal::s_videoMemoryPhysicalAddress.QuadPart + physicalOffset;

                *((UINT64 *)(pDmaBuf + patch->PatchOffset)) = physicalAddress;
            }
        }
    }
}
#endif

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
        }
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

    //
    // Allowing only one trigger for simplification
    //

    if (g_bTriggerTDR)
    {
        g_bTriggerEngineReset = false;
        g_bTriggerPreemption = false;
    }
    if (g_bTriggerEngineReset)
    {
        g_bTriggerPreemption = false;
    }

    if (!pDmaBufInfo->m_DmaBufState.m_bSubmittedOnce)
    {
        pDmaBufInfo->m_DmaBufState.m_bSubmittedOnce = 1;

        //
        // Trigger Preemption by emulating long running DMA buffer with stall
        //

        if ((!pDmaBufInfo->m_DmaBufState.m_bPaging) &&
            g_bTriggerPreemption)
        {
            pDmaBufInfo->m_DmaBufStallDuration = COS_DMA_BUF_STALL_DURATION;

            //
            // Reset the trigger for Preemption which was set using kernel debugger
            //

            g_bTriggerPreemption = false;
        }

        //
        // Timeout of regular DMA buffer triggers Engine Reset (lightweight reset)
        //

        if (g_bTriggerEngineReset &&
            (!pDmaBufInfo->m_DmaBufState.m_bPaging))
        {
            g_bTriggerEngineReset = false;

            m_bInHangState = true;
        }
    }

    //
    // Timeout of paging buffer or failure from DdiResetEngine triggers TDR (heavyweight reset)
    //

    if (g_bTriggerTDR &&
        pDmaBufInfo->m_DmaBufState.m_bPaging)
    {
        g_bTriggerTDR = false;

        m_bInHangState = true;
    }

    NT_ASSERT(!IsListEmpty(&m_dmaBufSubmissionFree));

    pDmaBufSubmission = CONTAINING_RECORD(RemoveHeadList(&m_dmaBufSubmissionFree), COSDMABUFSUBMISSION, m_QueueEntry);

    pDmaBufSubmission->m_pDmaBufInfo = pDmaBufInfo;

    pDmaBufSubmission->m_StartOffset = pSubmitCommand->DmaBufferSubmissionStartOffset;
    pDmaBufSubmission->m_EndOffset = pSubmitCommand->DmaBufferSubmissionEndOffset;
    pDmaBufSubmission->m_SubmissionFenceId = pSubmitCommand->SubmissionFenceId;

    //
    // Adapter remains in Hang state until reset (ResetEngine or ResetFromTimeout)
    //

    pDmaBufSubmission->m_bSimulateHang = m_bInHangState;

    InsertTailList(&m_dmaBufQueue, &pDmaBufSubmission->m_QueueEntry);

    m_lastSubmittedFenceId = pSubmitCommand->SubmissionFenceId;

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
