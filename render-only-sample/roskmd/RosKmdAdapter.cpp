
#define INITGUID
#include <ntifs.h>
#include "RosKmd.h"
#include "RosKmdAdapter.h"
#include "RosKmdAllocation.h"
#include "RosKmdContext.h"
#include "RosKmdResource.h"
#include "RosKmdGlobal.h"
#include "RosKmdUtil.h"
#include "RosGpuCommand.h"
#include "RosKmdAcpi.h"
#include "Vc4Hw.h"
#include "Vc4Ddi.h"

void * RosKmAdapter::operator new(size_t size)
{
    return ExAllocatePoolWithTag(NonPagedPoolNx, size, 'ROSD');
}

void RosKmAdapter::operator delete(void * ptr)
{
    ExFreePool(ptr);
}

RosKmAdapter::RosKmAdapter(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext)
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

    m_pVC4RegFile = NULL;

#endif

    *MiniportDeviceContext = this;
}

RosKmAdapter::~RosKmAdapter()
{
    // do nothing
}

NTSTATUS __stdcall
RosKmAdapter::AddAdapter(
    IN_CONST_PDEVICE_OBJECT     PhysicalDeviceObject,
    OUT_PPVOID                  MiniportDeviceContext)
{
    RosKmAdapter  *pRosKmAdapter = new RosKmAdapter(PhysicalDeviceObject, MiniportDeviceContext);

    if (!pRosKmAdapter)
    {
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::QueryEngineStatus(
    DXGKARG_QUERYENGINESTATUS  *pQueryEngineStatus)
{
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

            RtlFillMemoryUlong(
                ((BYTE *)RosKmdGlobal::s_pVideoMemory) + pPagingBuffer->Fill.Destination.SegmentAddress.QuadPart,
                pPagingBuffer->Fill.FillSize,
                pPagingBuffer->Fill.FillPattern);
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
RosKmAdapter::ProcessRenderBuffer(
    ROSDMABUFSUBMISSION * pDmaBufSubmission)
{
    ROSDMABUFINFO * pDmaBufInfo = pDmaBufSubmission->m_pDmaBufInfo;

    if (pDmaBufInfo->m_DmaBufState.m_bSwCommandBuffer)
    {
        NT_ASSERT(0 == (pDmaBufSubmission->m_EndOffset - pDmaBufSubmission->m_StartOffset) % sizeof(GpuCommand));

        GpuCommand * pGpuCommand = (GpuCommand *)(pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_StartOffset);
        GpuCommand * pEndofCommand = (GpuCommand *)(pDmaBufInfo->m_pDmaBuffer + pDmaBufSubmission->m_EndOffset);

        for (; pGpuCommand < pEndofCommand; pGpuCommand++)
        {
            switch (pGpuCommand->m_commandId)
            {
            case Header:
            case Nop:
                break;
            case ResourceCopy:
            {
                RtlCopyMemory(
                    ((BYTE *)RosKmdGlobal::s_pVideoMemory) + pGpuCommand->m_resourceCopy.m_dstGpuAddress.QuadPart,
                    ((BYTE *)RosKmdGlobal::s_pVideoMemory) + pGpuCommand->m_resourceCopy.m_srcGpuAddress.QuadPart,
                    pGpuCommand->m_resourceCopy.m_sizeBytes);
            }
            break;
            default:
                break;
            }
        }
    }
    else
    {
        //
        // Submit HW command buffer to the GPU
        //

#if VC4
        if (m_flags.m_isVC4)
        {
            //
            // TODO[indyz]:
            //
            // 1. Submit the Binning and Rendering Control list simultaneously
            //    and use semaphore for synchronization
            // 2. Enable interrupt to signal end of frame
            //

            //
            // Generate the Rendering Control List
            //
            UINT    renderingControlListLength;
            renderingControlListLength = GenerateRenderingControlList(pDmaBufInfo);

#if 1

            // TODO[indyz]: Decide the best way to handle the cache 
            //
            KeInvalidateAllCaches();

            //
            // Flush the VC4 GPU caches
            //

            V3D_REG_L2CACTL regL2CACTL = { 0 };

            regL2CACTL.L2CCLR = 1;
            
            m_pVC4RegFile->V3D_L2CACTL = regL2CACTL.Value;

            V3D_REG_SLCACTL regSLCACTL = { 0 };

            regSLCACTL.ICCS0123  = 0xF;
            regSLCACTL.UCCS0123  = 0xF;
            regSLCACTL.T0CCS0123 = 0xF;
            regSLCACTL.T1CCS0123 = 0xF;

            m_pVC4RegFile->V3D_SLCACTL = regSLCACTL.Value;

#endif

            //
            // Submit the Binning Control List from UMD to the GPU
            //
            NT_ASSERT(pDmaBufInfo->m_DmaBufferPhysicalAddress.HighPart == 0);
            NT_ASSERT(pDmaBufInfo->m_DmaBufferSize <= kPageSize);

            UINT dmaBufBaseAddress;
            
            dmaBufBaseAddress = GetAperturePhysicalAddress(pDmaBufInfo->m_DmaBufferPhysicalAddress.LowPart);

            // Skip the command buffer header at the beginning
            SubmitControlList(
                true,
                dmaBufBaseAddress + pDmaBufSubmission->m_StartOffset + sizeof(GpuCommand),
                dmaBufBaseAddress + pDmaBufSubmission->m_EndOffset);

            //
            // Submit the Rendering Control List to the GPU
            //
            SubmitControlList(
                false,
                m_renderingControlListPhysicalAddress,
                m_renderingControlListPhysicalAddress + renderingControlListLength);

            MoveToNextBinnerRenderMemChunk(renderingControlListLength);

            //
            // Flush the VC4 GPU caches
            //

            m_pVC4RegFile->V3D_L2CACTL = regL2CACTL.Value;
            m_pVC4RegFile->V3D_SLCACTL = regSLCACTL.Value;
        }
#endif  // VC4
    }
}

void
RosKmAdapter::SubmitControlList(
    bool bBinningControlList, 
    UINT startAddress,
    UINT endAddress)
{
    //
    // Setting End Address register kicks off execution of the Control List
    // Current Address register starts with CL start address and reaches
    // CL end address upon completion
    //

    V3D_REG_CT0CS regCTnCS = { 0 };

    regCTnCS.CTRUN = 1;

    if (bBinningControlList)
    {
        m_pVC4RegFile->V3D_CT0CS = regCTnCS.Value;
        KeMemoryBarrier();

        m_pVC4RegFile->V3D_CT0CA = startAddress;
        KeMemoryBarrier();

        m_pVC4RegFile->V3D_CT0EA = endAddress;
        KeMemoryBarrier();
    }
    else
    {
        m_pVC4RegFile->V3D_CT1CS = regCTnCS.Value;
        KeMemoryBarrier();

        m_pVC4RegFile->V3D_CT1CA = startAddress;
        KeMemoryBarrier();

        m_pVC4RegFile->V3D_CT1EA = endAddress;
        KeMemoryBarrier();
    }

    //
    // Completion of DMA buffer is acknowledged with interrupt and 
    // subsequent DPC signals m_hwDmaBufCompletionEvent
    //
    // TODO[indyz]: Enable interrupt and handle TDR
    //

    NTSTATUS status;

#if 1
    //
    // Set time out to 64 millisecond
    //

    LARGE_INTEGER timeOut;
    timeOut.QuadPart = -64 * 1000 * 1000 / 10;

    UINT i;
    for (i = 0; i < 32; i++)
    {
        status = KeWaitForSingleObject(
            &m_hwDmaBufCompletionEvent,
            Executive,
            KernelMode,
            FALSE,
            &timeOut);

        NT_ASSERT(status == STATUS_TIMEOUT);

        // Check Control List Executor Thread 0 or 1 Control and Status

        if (bBinningControlList)
        {
            regCTnCS.Value = m_pVC4RegFile->V3D_CT0CS;
        }
        else
        {
            regCTnCS.Value = m_pVC4RegFile->V3D_CT1CS;
        }

        if (regCTnCS.CTRUN == 0)
        {
            break;
        }
    }

    // Check for TDR condition
    NT_ASSERT(i < 32);

#else

    status = KeWaitForSingleObject(
        &m_hwDmaBufCompletionEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL);

    NT_ASSERT(status == STATUS_SUCCESS);

#endif

}

UINT
RosKmAdapter::GenerateRenderingControlList(
    ROSDMABUFINFO *pDmaBufInfo)
{
    RosKmdAllocation *pRenderTarget = pDmaBufInfo->m_pRenderTarget;

    // Write Clear Colors command from UMD
    VC4ClearColors *pVC4ClearColors;
    VC4TileRenderingModeConfig *pVC4TileRenderingModeConfig;

    if (pDmaBufInfo->m_DmaBufState.m_HasVC4ClearColors)
    {
        pVC4ClearColors = (VC4ClearColors *)m_pRenderingControlList;

        *pVC4ClearColors = pDmaBufInfo->m_VC4ClearColors;

        MoveToNextCommand(pVC4ClearColors, pVC4TileRenderingModeConfig);
    }
    else
    {
        pVC4TileRenderingModeConfig = (VC4TileRenderingModeConfig *)m_pRenderingControlList;
    }

    // Write Tile Rendering Mode Config command

    VC4TileRenderingModeConfig  tileRenderingModeConfig = vc4TileRenderingModeConfig;

    tileRenderingModeConfig.MemoryAddress = pDmaBufInfo->m_RenderTargetPhysicalAddress;

    tileRenderingModeConfig.WidthInPixels  = (USHORT)pRenderTarget->m_mip0Info.TexelWidth;
    tileRenderingModeConfig.HeightInPixels = (USHORT)pRenderTarget->m_mip0Info.TexelHeight;

    NT_ASSERT(pRenderTarget->m_hwFormat == X8888);
    tileRenderingModeConfig.NonHDRFrameBufferColorFormat = 1;

    *pVC4TileRenderingModeConfig = tileRenderingModeConfig;

    // Clear the tile buffer by store the 1st tile
    VC4TileCoordinates *pVC4TileCoordinates;
    MoveToNextCommand(pVC4TileRenderingModeConfig, pVC4TileCoordinates);

    *pVC4TileCoordinates = vc4TileCoordinates;

    VC4StoreTileBufferGeneral  *pVC4StoreTileBufferGeneral;
    MoveToNextCommand(pVC4TileCoordinates, pVC4StoreTileBufferGeneral);

    *pVC4StoreTileBufferGeneral = vc4StoreTileBufferGeneral;

    //
    // Calling control list generated by the Binning Control List
    //
    UINT    widthInTiles  = pRenderTarget->m_hwWidthPixels  / VC4_BINNING_TILE_PIXELS;
    UINT    heightInTiles = pRenderTarget->m_hwHeightPixels / VC4_BINNING_TILE_PIXELS;

    VC4TileCoordinates  tileCoordinates = vc4TileCoordinates;
    VC4BranchToSubList  branchToSubList = vc4BranchToSubList;
    UINT    tileAllocationPhysicalAddress = m_tileAllocationMemoryPhysicalAddress;
    VC4BranchToSubList *pVC4BranchToSubList;
    VC4StoreMSResolvedTileColorBuf *pVC4StoreMSResolvedTileColorBuf;
    VC4StoreMSResolvedTileColorBufAndSignalEndOfFrame  *pVC4StoreMSResolvedTileColorBufAndSignalEndOfFrame;

    MoveToNextCommand(pVC4StoreTileBufferGeneral, pVC4TileCoordinates);
    pVC4StoreMSResolvedTileColorBufAndSignalEndOfFrame = (VC4StoreMSResolvedTileColorBufAndSignalEndOfFrame *)pVC4TileCoordinates;

    for (UINT x = 0; x < widthInTiles; x++)
    {
        for (UINT y = 0; y < heightInTiles; y++)
        {
            tileCoordinates.TileColumnNumber = (BYTE)x;
            tileCoordinates.TileRowNumber    = (BYTE)y;

            *pVC4TileCoordinates = tileCoordinates;

            MoveToNextCommand(pVC4TileCoordinates, pVC4BranchToSubList);

            branchToSubList.BranchAddress = tileAllocationPhysicalAddress + (y*widthInTiles + x)*VC4_TILE_ALLOCATION_BLOCK_SIZE;

            *pVC4BranchToSubList = branchToSubList;

            if ((x == (widthInTiles - 1)) &&
                (y == (heightInTiles - 1)))
            {
                MoveToNextCommand(pVC4BranchToSubList, pVC4StoreMSResolvedTileColorBufAndSignalEndOfFrame);

                *pVC4StoreMSResolvedTileColorBufAndSignalEndOfFrame = vc4StoreMSResolvedTileColorBufAndSignalEndOfFrame;

                pVC4StoreMSResolvedTileColorBufAndSignalEndOfFrame++;
            }
            else
            {
                MoveToNextCommand(pVC4BranchToSubList, pVC4StoreMSResolvedTileColorBuf);

                *pVC4StoreMSResolvedTileColorBuf = vc4StoreMSResolvedTileColorBuf;

                MoveToNextCommand(pVC4StoreMSResolvedTileColorBuf, pVC4TileCoordinates);
            }
        }
    }

    return ((UINT)(((PBYTE)pVC4StoreMSResolvedTileColorBufAndSignalEndOfFrame) - m_pRenderingControlList));
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

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

#if HW_GPU
    //
    // m_deviceInfo.TranslatedResourceList has the HW resource information
    // (MMIO register file, interrupt, etc)
    //
#endif

    status = m_DxgkInterface.DxgkCbGetDeviceInformation(
        m_DxgkInterface.DeviceHandle,
        &m_deviceInfo);
    NT_ASSERT(status == STATUS_SUCCESS);

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
    CM_PARTIAL_RESOURCE_LIST       *pResourceList = &m_deviceInfo.TranslatedResourceList->List->PartialResourceList;
    CM_PARTIAL_RESOURCE_DESCRIPTOR *pResource = &pResourceList->PartialDescriptors[0];


    // Indicate if we are running on VC4 hardware
    if ((pResourceList->Count == 2) &&
        (pResource->Type == CmResourceTypeMemory))
    {
        m_flags.m_isVC4 = pResourceList->Count == 2;
    }

#if VC4

    if (m_flags.m_isVC4)
    {
        status = m_DxgkInterface.DxgkCbMapMemory(
            m_DxgkInterface.DeviceHandle,
            pResource->u.Memory.Start,
            pResource->u.Memory.Length,
            FALSE,
            FALSE,
            MmNonCached,
            (PVOID *)&m_pVC4RegFile);

        if (status != STATUS_SUCCESS)
        {
            return status;
        }
    }

    m_localVidMemSegmentSize = ((UINT)RosKmdGlobal::s_videoMemorySize) -
        (VC4_RENDERING_CTRL_LIST_POOL_SIZE +
         VC4_TILE_ALLOCATION_MEMORY_SIZE   +
         VC4_TILE_STATE_DATA_ARRAY_SIZE);

    m_pControlListPool = ((PBYTE)RosKmdGlobal::s_pVideoMemory) + m_localVidMemSegmentSize;

    NT_ASSERT(0 == RosKmdGlobal::s_videoMemoryPhysicalAddress.HighPart);
    m_controlListPoolPhysicalAddress      = RosKmdGlobal::s_videoMemoryPhysicalAddress.LowPart + m_localVidMemSegmentSize;
    m_tileAllocPoolPhysicalAddress        = m_controlListPoolPhysicalAddress + VC4_RENDERING_CTRL_LIST_POOL_SIZE;
    m_tileStatePoolPhysicalAddress        = m_tileAllocPoolPhysicalAddress + VC4_TILE_ALLOCATION_MEMORY_SIZE;

    m_pRenderingControlList               = m_pControlListPool;
    m_renderingControlListPhysicalAddress = m_controlListPoolPhysicalAddress;

    m_tileAllocationMemoryPhysicalAddress = m_tileAllocPoolPhysicalAddress;
    m_tileStateDataArrayPhysicalAddress   = m_tileStatePoolPhysicalAddress;

#endif

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

#if HW_GPU

    //
    // Enable interrupt handling when device is ready
    //

    m_bReadyToHandleInterrupt = TRUE;

#endif

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

        DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "Filling at %lx with %lx size %lx\n",
            pArgs->Fill.Destination.SegmentAddress,
            pArgs->Fill.FillPattern,
            pArgs->Fill.FillSize);

        if (pArgs->DmaSize < sizeof(DXGKARG_BUILDPAGINGBUFFER))
        {
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

NTSTATUS __stdcall 
RosKmAdapter::DdiSubmitCommand(
    IN_CONST_HANDLE                     hAdapter,
    IN_CONST_PDXGKARG_SUBMITCOMMAND     pSubmitCommand)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(hAdapter);
    pRosKmAdapter;

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

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

        dmaBufPhysicalAddress = pRosKmAdapter->GetAperturePhysicalAddress(
            pSubmitCommand->DmaBufferPhysicalAddress.LowPart);

        for (UINT i = 0; i < pDmaBufInfo->m_DmaBufState.m_NumDmaBufSelfRef; i++)
        {
            D3DDDI_PATCHLOCATIONLIST   *pPatchLoc = &pDmaBufInfo->m_DmaBufSelfRef[i];

            *((UINT *)(pDmaBuf + pPatchLoc->PatchOffset)) = dmaBufPhysicalAddress +
                pPatchLoc->AllocationOffset;
        }
    }

#endif

    // NOTE: pRosKmContext will be NULL for paging operations
    RosKmContext *pRosKmContext = (RosKmContext *)pSubmitCommand->hContext;
    pRosKmContext;

    pRosKmAdapter->QueueDmaBuffer(pSubmitCommand);

    //
    // Wake up the worker thread for the GPU node
    //
    KeSetEvent(&pRosKmAdapter->m_workerThreadEvent, 0, FALSE);

    return Status;
}

NTSTATUS __stdcall
RosKmAdapter::DdiPatch(
    IN_CONST_HANDLE             hAdapter,
    IN_CONST_PDXGKARG_PATCH     pPatch)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(hAdapter);
    ROSDMABUFINFO *pDmaBufInfo = (ROSDMABUFINFO *)pPatch->pDmaBufferPrivateData;

    pRosKmAdapter;

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    RosKmContext * pRosKmContext = (RosKmContext *)pPatch->hContext;
    pRosKmContext;

    pDmaBufInfo->m_DmaBufferPhysicalAddress = pPatch->DmaBufferPhysicalAddress;

    pRosKmAdapter->PatchDmaBuffer(
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
__stdcall
RosKmAdapter::DdiCreateAllocation(
    IN_CONST_HANDLE                     hAdapter,
    INOUT_PDXGKARG_CREATEALLOCATION     pCreateAllocation)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(hAdapter);
    pRosKmAdapter;

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
        return STATUS_NO_MEMORY;
    }

    *(RosAllocationExchange *)pRosKmdAllocation = *pRosAllocation;

    pAllocationInfo->hAllocation = pRosKmdAllocation;

    pAllocationInfo->Alignment = 64;
    pAllocationInfo->AllocationPriority = D3DDDI_ALLOCATIONPRIORITY_NORMAL;
    pAllocationInfo->EvictionSegmentSet = 0; // don't use apperture for eviction

    pAllocationInfo->Flags.Value = 0;
    // Always mark allocation as CPU visible
    pAllocationInfo->Flags.CpuVisible = 1;
    // TODO[indyz]: Look into if Cached should be used
    // pAllocationInfo->Flags.Cached = 1;

    pAllocationInfo->HintedBank.Value = 0;
    pAllocationInfo->MaximumRenamingListLength = 0;
    pAllocationInfo->pAllocationUsageHint = NULL;
    pAllocationInfo->PhysicalAdapterIndex = 0;
    pAllocationInfo->PitchAlignedSize = 0;
    pAllocationInfo->PreferredSegment.Value = 0;
    pAllocationInfo->PreferredSegment.SegmentId0 = ROSD_SEGMENT_VIDEO_MEMORY;
    pAllocationInfo->PreferredSegment.Direction0 = 0;
    pAllocationInfo->Size = pRosAllocation->m_hwSizeBytes;
    pAllocationInfo->SupportedReadSegmentSet = 1 << (ROSD_SEGMENT_VIDEO_MEMORY - 1);
    pAllocationInfo->SupportedWriteSegmentSet = 1 << (ROSD_SEGMENT_VIDEO_MEMORY - 1);

    if (pCreateAllocation->Flags.Resource && pCreateAllocation->hResource == NULL && pRosKmdResource != NULL)
    {
        pCreateAllocation->hResource = pRosKmdResource;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiDestroyAllocation(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_DESTROYALLOCATION     pDestroyAllocation)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(hAdapter);
    pRosKmAdapter;

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
__stdcall
RosKmAdapter::DdiQueryAdapterInfo(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_QUERYADAPTERINFO      pQueryAdapterInfo)
{
    RosKmAdapter   *pRosKmAdapter = RosKmAdapter::Cast(hAdapter);
    NTSTATUS        Status = STATUS_INVALID_PARAMETER;

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdQueryAdapterInfo hAdapter=%lx Type=%d\n",
        hAdapter, pQueryAdapterInfo->Type);

    switch (pQueryAdapterInfo->Type)
    {
    case DXGKQAITYPE_UMDRIVERPRIVATE:
    {
        if (pQueryAdapterInfo->OutputDataSize < sizeof(ROSADAPTERINFO))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        ROSADAPTERINFO* pRosAdapterInfo = (ROSADAPTERINFO*)pQueryAdapterInfo->pOutputData;
        
        pRosAdapterInfo->m_version = ROSD_VERSION;
        pRosAdapterInfo->m_wddmVersion = pRosKmAdapter->m_WDDMVersion;

        // Software APCI device only claims an interrupt resource
        pRosAdapterInfo->m_isSoftwareDevice = (pRosKmAdapter->m_flags.m_isVC4 != 1);

        RtlCopyMemory(
            pRosAdapterInfo->m_deviceId,
            pRosKmAdapter->m_deviceId,
            pRosKmAdapter->m_deviceIdLength);

        Status = STATUS_SUCCESS;
    }
    break;

    case DXGKQAITYPE_DRIVERCAPS:
    {
        if (pQueryAdapterInfo->OutputDataSize < sizeof(DXGK_DRIVERCAPS))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
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
        // TODO[bhouse] MaxQueueFlipOnVSync
        //

        //
        // TODO[bhouse] FlipCaps
        //

        //
        // Use SW flip queue for flip with interval of 1 or more
        //
#if 1
        pDriverCaps->FlipCaps.FlipOnVSyncMmIo = 1;
#endif

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

        pDriverCaps->GpuEngineTopology.NbAsymetricProcessingNodes = pRosKmAdapter->m_NumNodes;

        //
        // TODO[bhouse] WDDMVersion
        //              Documentation states that we should not set this value if WDDM 1.3
        //
        pDriverCaps->WDDMVersion = pRosKmAdapter->m_WDDMVersion;

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
        // TODO[bhouse] SupportNonVGA
        //

        //
        // TODO[bhouse] SupportSmoothRotation
        //

        //
        // TODO[bhouse] SupportPerEngineTDR
        //
#if 1
        pDriverCaps->SupportPerEngineTDR = 1;
#endif

        //
        // TODO[bhouse] SupportDirectFlip
        //
#if 1
        pDriverCaps->SupportDirectFlip = 1;
#endif

        //
        // TODO[bhouse] SupportMultiPlaneOverlay
        //

        //
        // Support SupportRuntimePowerManagement
        //
        pDriverCaps->SupportRuntimePowerManagement = 1;

        //
        // TODO[bhouse] SupportSurpriseRemovalInHibernation
        //

        //
        // TODO[bhouse] HybridDiscrete
        //

        //
        // TODO[bhouse] MaxOverlayPlanes
        //


        Status = STATUS_SUCCESS;
    }
    break;

    case DXGKQAITYPE_QUERYSEGMENT3:
    {
        if (pQueryAdapterInfo->OutputDataSize < sizeof(DXGK_QUERYSEGMENTOUT3))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        DXGK_QUERYSEGMENTOUT3   *pSegmentInfo = (DXGK_QUERYSEGMENTOUT3*)pQueryAdapterInfo->pOutputData;

        Status = STATUS_SUCCESS;

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
            // that will never be used.
            //
            // TODO[bhouse] Investigate why we have to lie.
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
            pSegmentDesc[1].CpuTranslatedAddress = RosKmdGlobal::s_videoMemoryPhysicalAddress; // cpu base physical address
            pSegmentDesc[1].Size = RosKmdGlobal::s_videoMemorySize;

        }
    }
    break;

    case DXGKQAITYPE_NUMPOWERCOMPONENTS:
    {
        if (pQueryAdapterInfo->OutputDataSize != sizeof(UINT))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        //
        // Support only one 3D engine(s).
        //
        *(reinterpret_cast<UINT*>(pQueryAdapterInfo->pOutputData)) = pRosKmAdapter->GetNumPowerComponents();

        Status = STATUS_SUCCESS;
    }
    break;

    case DXGKQAITYPE_POWERCOMPONENTINFO:
    {
        if (pQueryAdapterInfo->InputDataSize != sizeof(UINT) ||
            pQueryAdapterInfo->OutputDataSize < sizeof(DXGK_POWER_RUNTIME_COMPONENT))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        ULONG ComponentIndex = *(reinterpret_cast<UINT*>(pQueryAdapterInfo->pInputData));
        DXGK_POWER_RUNTIME_COMPONENT* pPowerComponent = reinterpret_cast<DXGK_POWER_RUNTIME_COMPONENT*>(pQueryAdapterInfo->pOutputData);

        Status = pRosKmAdapter->GetPowerComponentInfo(ComponentIndex, pPowerComponent); 
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

        Status = STATUS_SUCCESS;
    }
    break;

    default:
        // NT_ASSERT(FALSE);
        break;
    }

    return Status;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiDescribeAllocation(
    IN_CONST_HANDLE                         hAdapter,
    INOUT_PDXGKARG_DESCRIBEALLOCATION       pDescribeAllocation)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    RosKmdAllocation *pAllocation = (RosKmdAllocation *)pDescribeAllocation;

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
__stdcall
RosKmAdapter::DdiGetNodeMetadata(
    IN_CONST_HANDLE                 hAdapter,
    UINT                            NodeOrdinal,
    OUT_PDXGKARG_GETNODEMETADATA    pGetNodeMetadata
    )
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdGetNodeMetadata hAdapter=%lx Node=%d\n",
        hAdapter, NodeOrdinal);

    RtlZeroMemory(pGetNodeMetadata, sizeof(*pGetNodeMetadata));

    pGetNodeMetadata->EngineType = DXGK_ENGINE_TYPE_3D;

    RtlStringCbPrintfW(pGetNodeMetadata->FriendlyName,
        sizeof(pGetNodeMetadata->FriendlyName),
        L"3DNode%02X",
        NodeOrdinal);


    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiGetStandardAllocationDriverData(
    IN_CONST_HANDLE                                 hAdapter,
    INOUT_PDXGKARG_GETSTANDARDALLOCATIONDRIVERDATA  pGetStandardAllocationDriverData)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    pGetStandardAllocationDriverData;

    NTSTATUS    Status = STATUS_SUCCESS;

    return Status;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiSubmitCommandVirtual(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_SUBMITCOMMANDVIRTUAL  pSubmitCommandVirtual)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    hAdapter;
    pSubmitCommandVirtual;

    NTSTATUS        Status = STATUS_SUCCESS;

    return Status;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiPreemptCommand(
    IN_CONST_HANDLE                     hAdapter,
    IN_CONST_PDXGKARG_PREEMPTCOMMAND    pPreemptCommand)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    hAdapter;
    pPreemptCommand;

    NTSTATUS        Status = STATUS_SUCCESS;

    return Status;
}

NTSTATUS
__stdcall CALLBACK
RosKmAdapter::DdiRestartFromTimeout(
    IN_CONST_HANDLE     hAdapter)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiCancelCommand(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_PDXGKARG_CANCELCOMMAND pCancelCommand)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    hAdapter;
    pCancelCommand;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiQueryCurrentFence(
    IN_CONST_HANDLE                    hAdapter,
    INOUT_PDXGKARG_QUERYCURRENTFENCE   pCurrentFence)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    hAdapter;
    pCurrentFence;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiResetEngine(
    IN_CONST_HANDLE             hAdapter,
    INOUT_PDXGKARG_RESETENGINE  pResetEngine)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    hAdapter;
    pResetEngine;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiQueryEngineStatus(
    IN_CONST_HANDLE                     hAdapter,
    INOUT_PDXGKARG_QUERYENGINESTATUS    pQueryEngineStatus)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    hAdapter;
    pQueryEngineStatus;

    return STATUS_SUCCESS;
}


NTSTATUS
__stdcall
RosKmAdapter::DdiQueryDependentEngineGroup(
    IN_CONST_HANDLE                             hAdapter,
    INOUT_DXGKARG_QUERYDEPENDENTENGINEGROUP     pQueryDependentEngineGroup)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    hAdapter;
    pQueryDependentEngineGroup;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiGetScanLine(
    IN_CONST_HANDLE             hAdapter,
    INOUT_PDXGKARG_GETSCANLINE  /*pGetScanLine*/)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    NT_ASSERT(FALSE);
    return STATUS_SUCCESS;
}


NTSTATUS
__stdcall
RosKmAdapter::DdiControlInterrupt(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_DXGK_INTERRUPT_TYPE    InterruptType,
    IN_BOOLEAN                      EnableInterrupt)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    hAdapter;
    InterruptType;
    EnableInterrupt;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiCollectDbgInfo(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_COLLECTDBGINFO        /*pCollectDbgInfo*/)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiPresent(
    IN_CONST_HANDLE         hContext,
    INOUT_PDXGKARG_PRESENT  pPresent)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmAdapter::DdiPresent hAdapter=%lx\n", hContext);

    hContext;
    pPresent;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiCreateProcess(
    IN_PVOID  pMiniportDeviceContext,
    IN DXGKARG_CREATEPROCESS* pArgs)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdCreateProcess MiniportDeviceContext=%lx\n",
        pMiniportDeviceContext);

    UNREFERENCED_PARAMETER(pMiniportDeviceContext);

    pArgs->hKmdProcess = 0;
    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiDestroyProcess(
    IN_PVOID pMiniportDeviceContext,
    IN HANDLE KmdProcessHandle)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdDestroyProcess MiniportDeviceContext=%lx\n",
        pMiniportDeviceContext);

    UNREFERENCED_PARAMETER(pMiniportDeviceContext);
    UNREFERENCED_PARAMETER(KmdProcessHandle);
    return STATUS_SUCCESS;
}

void
__stdcall
RosKmAdapter::DdiSetStablePowerState(
    IN_CONST_HANDLE                        hAdapter,
    IN_CONST_PDXGKARG_SETSTABLEPOWERSTATE  pArgs)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdSetStablePowerState=%lx\n",
        hAdapter);

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(pArgs);
}

NTSTATUS
__stdcall
RosKmAdapter::DdiCalibrateGpuClock(
    IN_CONST_HANDLE                             hAdapter,
    IN UINT32                                   NodeOrdinal,
    IN UINT32                                   EngineOrdinal,
    OUT_PDXGKARG_CALIBRATEGPUCLOCK              pClockCalibration
    )
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    hAdapter;
    NodeOrdinal;
    EngineOrdinal;
    pClockCalibration;

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiRenderKm(
    IN_CONST_HANDLE         hContext,
    INOUT_PDXGKARG_RENDER   pRender)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "RosKmdRenderKm hAdapter=%lx\n", hContext);

    hContext;
    pRender;

    NT_ASSERT(FALSE);
    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiEscape(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_PDXGKARG_ESCAPE        pEscape)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s\n",
        __FUNCTION__);

    hAdapter;

    NTSTATUS        Status;

    if (pEscape->PrivateDriverDataSize < sizeof(UINT))
    {
        return STATUS_INVALID_PARAMETER;
    }

    UINT    EscapeId = *((UINT *)pEscape->pPrivateDriverData);

    switch (EscapeId)
    {

    default:

        NT_ASSERT(false);
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    return Status;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiSetPalette(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_PDXGKARG_SETPALETTE    /*pSetPalette*/)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    return STATUS_SUCCESS;
}

NTSTATUS
__stdcall
RosKmAdapter::DdiSetPointerPosition(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_SETPOINTERPOSITION    pSetPointerPosition)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    if (pSetPointerPosition->Flags.Visible)
    {
        return STATUS_NOT_IMPLEMENTED;
    }
    else
    {
        return STATUS_SUCCESS;
    }

}

NTSTATUS
__stdcall
RosKmAdapter::DdiSetPointerShape(
    IN_CONST_HANDLE                     hAdapter,
    IN_CONST_PDXGKARG_SETPOINTERSHAPE   pSetPointerShape)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    if (!pSetPointerShape->Flags.Value)
    {
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
__stdcall CALLBACK
RosKmAdapter::DdiResetFromTimeout(
    IN_CONST_HANDLE     hAdapter)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n",
        __FUNCTION__, hAdapter);

    RosKmAdapter  *pRosKmAdapter = (RosKmAdapter *)hAdapter;

    return pRosKmAdapter->ResetFromTdr();
}

NTSTATUS
RosKmAdapter::ResetFromTdr()
{
    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::DdiQueryInterface(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_PQUERY_INTERFACE     QueryInterface)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    QueryInterface;

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
RosKmAdapter::DdiQueryChildRelations(
    IN_CONST_PVOID                  MiniportDeviceContext,
    INOUT_PDXGK_CHILD_DESCRIPTOR    ChildRelations,
    IN_ULONG                        ChildRelationsSize)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    MiniportDeviceContext;
    ChildRelations;
    ChildRelationsSize;

    return STATUS_INVALID_PARAMETER;
}

NTSTATUS
RosKmAdapter::DdiQueryChildStatus(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_PDXGK_CHILD_STATUS   ChildStatus,
    IN_BOOLEAN              NonDestructiveOnly)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    MiniportDeviceContext;
    ChildStatus;
    NonDestructiveOnly;

    return STATUS_INVALID_PARAMETER;
}

NTSTATUS
RosKmAdapter::DdiQueryDeviceDescriptor(
    IN_CONST_PVOID                  MiniportDeviceContext,
    IN_ULONG                        ChildUid,
    INOUT_PDXGK_DEVICE_DESCRIPTOR   pDeviceDescriptor)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    MiniportDeviceContext;
    ChildUid;
    pDeviceDescriptor;

    return STATUS_INVALID_PARAMETER;
}

NTSTATUS
RosKmAdapter::DdiNotifyAcpiEvent(
    IN_CONST_PVOID      MiniportDeviceContext,
    IN_DXGK_EVENT_TYPE  EventType,
    IN_ULONG            Event,
    IN_PVOID            Argument,
    OUT_PULONG          AcpiFlags)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    MiniportDeviceContext;
    EventType;
    Event;
    Argument;
    AcpiFlags;

    NT_ASSERT(false);
    return STATUS_SUCCESS;
}

void
RosKmAdapter::DdiResetDevice(
    IN_CONST_PVOID  MiniportDeviceContext)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

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
                UINT    gpuAddress = RosKmdGlobal::s_videoMemoryPhysicalAddress.LowPart + 
                                     allocation->PhysicalAddress.LowPart +
                                     patch->AllocationOffset;
                
                switch (patch->SlotId)
                {
                case VC4_SLOT_RT_BINNING_CONFIG:
                    pDmaBufInfo->m_RenderTargetPhysicalAddress = gpuAddress;
                    break;
                case VC4_SLOT_TILE_ALLOCATION_MEMORY:
                    *((UINT *)(pDmaBuf + patch->PatchOffset)) = m_tileAllocationMemoryPhysicalAddress;
                    break;
                case VC4_SLOT_TILE_STATE_DATA_ARRAY:
                    *((UINT *)(pDmaBuf + patch->PatchOffset)) = m_tileStateDataArrayPhysicalAddress;
                    break;
                case VC4_SLOT_NV_SHADER_STATE:
                case VC4_SLOT_BRANCH:
                    // When PrePatch happens in DdiRender, DMA buffer physical
                    // address is not available, so DMA buffer self-reference
                    // patches are handled in DdiSubmitCommand
                    break;
                default:
                    *((UINT *)(pDmaBuf + patch->PatchOffset)) = gpuAddress;
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

