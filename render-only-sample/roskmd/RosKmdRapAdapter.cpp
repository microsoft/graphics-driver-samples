#include "RosKmdRapAdapter.h"
#include "RosGpuCommand.h"

#include "Vc4Mailbox.h"

RosKmdRapAdapter::RosKmdRapAdapter(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext) :
    RosKmAdapter(PhysicalDeviceObject, MiniportDeviceContext)
{
    m_pVC4RegFile = NULL;
}

RosKmdRapAdapter::~RosKmdRapAdapter()
{
    // do nothing
}

void * RosKmdRapAdapter::operator new(size_t size)
{
    return ExAllocatePoolWithTag(NonPagedPoolNx, size, 'ROSD');
}

void RosKmdRapAdapter::operator delete(void * ptr)
{
    ExFreePool(ptr);
}

NTSTATUS
RosKmdRapAdapter::Start(
    IN_PDXGK_START_INFO     DxgkStartInfo,
    IN_PDXGKRNL_INTERFACE   DxgkInterface,
    OUT_PULONG              NumberOfVideoPresentSources,
    OUT_PULONG              NumberOfChildren)
{
    NTSTATUS status = RosKmAdapter::Start(DxgkStartInfo, DxgkInterface, NumberOfVideoPresentSources, NumberOfChildren);

    if (status != STATUS_SUCCESS) return status;

#if VC4
    //
    // m_deviceInfo.TranslatedResourceList has the HW resource information
    // (MMIO register file, interrupt, etc)
    //
#endif

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

        PFILE_OBJECT    fileObj;

        DECLARE_CONST_UNICODE_STRING(rpiqDeviceName, L"\\DosDevices\\RPIQ");

        status = IoGetDeviceObjectPointer(
            (PUNICODE_STRING)&rpiqDeviceName,
            FILE_ALL_ACCESS,
            &fileObj,
            &m_pRpiqDevice);

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        ObReferenceObject(m_pRpiqDevice);
        ObDereferenceObject(fileObj);

        status = SetVC4Power(true);

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        //
        // Highest 2 bits of VC4 GPU address controls path through cache
        //
        // Use the "C" alias so that VC4 doesn't use the L2 cache
        //
        // See http://www.farnell.com/datasheets/1521578.pdf section 1.2
        //

        m_busAddressOffset = VC4_BUS_ADDRESS_ALIAS_UNCACHED;

#if DBG

        //
        // Enable performance counter
        //

        volatile UINT *  pRegPerfCountSrc = &m_pVC4RegFile->V3D_PCTRS0;

        for (UINT i = 0; i < V3D_NUM_PERF_COUNTERS; i++)
        {
            pRegPerfCountSrc[i << 1] = i;
        }

        m_pVC4RegFile->V3D_PCTRE = ((1 << V3D_NUM_PERF_COUNTERS) - 1);

#endif
    }

    m_localVidMemSegmentSize = ((UINT)RosKmdGlobal::s_videoMemorySize) -
        (VC4_RENDERING_CTRL_LIST_POOL_SIZE +
            VC4_TILE_ALLOCATION_MEMORY_SIZE +
            VC4_TILE_STATE_DATA_ARRAY_SIZE);

    m_pControlListPool = ((PBYTE)RosKmdGlobal::s_pVideoMemory) + m_localVidMemSegmentSize;

    NT_ASSERT(0 == RosKmdGlobal::s_videoMemoryPhysicalAddress.HighPart);
    m_controlListPoolPhysicalAddress = RosKmdGlobal::s_videoMemoryPhysicalAddress.LowPart + m_localVidMemSegmentSize;
    m_tileAllocPoolPhysicalAddress = m_controlListPoolPhysicalAddress + VC4_RENDERING_CTRL_LIST_POOL_SIZE;
    m_tileStatePoolPhysicalAddress = m_tileAllocPoolPhysicalAddress + VC4_TILE_ALLOCATION_MEMORY_SIZE;

    m_pRenderingControlList = m_pControlListPool;
    m_renderingControlListPhysicalAddress = m_controlListPoolPhysicalAddress;

    m_tileAllocationMemoryPhysicalAddress = m_tileAllocPoolPhysicalAddress;
    m_tileStateDataArrayPhysicalAddress = m_tileStatePoolPhysicalAddress;

#endif

#if VC4_TODO

    //
    // Enable interrupt handling when device is ready
    //

    m_bReadyToHandleInterrupt = TRUE;

#endif

    return STATUS_SUCCESS;

}


void
RosKmdRapAdapter::ProcessRenderBuffer(
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

            regSLCACTL.ICCS0123 = 0xF;
            regSLCACTL.UCCS0123 = 0xF;
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
            dmaBufBaseAddress += m_busAddressOffset;

#if DBG

            m_pVC4RegFile->V3D_PCTRC = ((1 << V3D_NUM_PERF_COUNTERS) - 1);

#endif

            // Skip the command buffer header at the beginning
            SubmitControlList(
                true,
                dmaBufBaseAddress + pDmaBufSubmission->m_StartOffset + sizeof(GpuCommand),
                dmaBufBaseAddress + pDmaBufSubmission->m_EndOffset);

#if DBG

            m_pVC4RegFile->V3D_PCTRC = ((1 << V3D_NUM_PERF_COUNTERS) - 1);

#endif

            //
            // Submit the Rendering Control List to the GPU
            //
            SubmitControlList(
                false,
                m_renderingControlListPhysicalAddress + m_busAddressOffset,
                m_renderingControlListPhysicalAddress + m_busAddressOffset + renderingControlListLength);

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
RosKmdRapAdapter::SubmitControlList(
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
RosKmdRapAdapter::GenerateRenderingControlList(
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

    tileRenderingModeConfig.MemoryAddress = pDmaBufInfo->m_RenderTargetPhysicalAddress + m_busAddressOffset;

    tileRenderingModeConfig.WidthInPixels = (USHORT)pRenderTarget->m_mip0Info.TexelWidth;
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
    UINT    widthInTiles = pRenderTarget->m_hwWidthPixels / VC4_BINNING_TILE_PIXELS;
    UINT    heightInTiles = pRenderTarget->m_hwHeightPixels / VC4_BINNING_TILE_PIXELS;

    VC4TileCoordinates  tileCoordinates = vc4TileCoordinates;
    VC4BranchToSubList  branchToSubList = vc4BranchToSubList;
    UINT    tileAllocationPhysicalAddress = m_tileAllocationMemoryPhysicalAddress + m_busAddressOffset;
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
            tileCoordinates.TileRowNumber = (BYTE)y;

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

NTSTATUS
RosKmdRapAdapter::SetVC4Power(
        bool    bOn)
{
    PIRP pIrp = NULL;
    KEVENT ioCompleted;
    IO_STATUS_BLOCK statusBlock;
    MAILBOX_SET_POWER_VC4 setPowerVC4;

    KeInitializeEvent(&ioCompleted, NotificationEvent, FALSE);

    INIT_MAILBOX_SET_POWER_VC4(&setPowerVC4, bOn);

    pIrp = IoBuildDeviceIoControlRequest(
        IOCTL_MAILBOX_PROPERTY,
        m_pRpiqDevice,
        &setPowerVC4,
        sizeof(setPowerVC4),
        &setPowerVC4,
        sizeof(setPowerVC4),
        false,
        &ioCompleted,
        &statusBlock);
    if (NULL == pIrp)
    {
        return STATUS_NO_MEMORY;
    }

    NTSTATUS status;

    status = IoCallDriver(m_pRpiqDevice, pIrp);

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&ioCompleted, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }

    if (STATUS_SUCCESS != status)
    {
        return status;
    }

    if (setPowerVC4.Header.RequestResponse == RESPONSE_SUCCESS)
    {
        return STATUS_SUCCESS;
    }
    else
    {
        return STATUS_INVALID_PARAMETER;
    }
}

