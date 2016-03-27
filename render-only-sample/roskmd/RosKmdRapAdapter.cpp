#include "precomp.h"

#include "RosKmdLogging.h"
#include "RosKmdRapAdapter.tmh"

#include "RosKmdRapAdapter.h"
#include "RosGpuCommand.h"
#include "RosKmdUtil.h"
#include "Vc4Mailbox.h"


#if USE_SIMPENROSE

#include "simpenrose.h"
bool g_bUseSimPenrose = false;

#endif

bool g_bUseInterrupt = true;

RosKmdRapAdapter::RosKmdRapAdapter(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext) :
    RosKmAdapter(PhysicalDeviceObject, MiniportDeviceContext)
{
    m_pVC4RegFile = NULL;
    m_flags.m_isVC4 = TRUE;
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
    NTSTATUS status = RosKmAdapter::Start(
            DxgkStartInfo,
            DxgkInterface,
            NumberOfVideoPresentSources,
            NumberOfChildren);
    if (!NT_SUCCESS(status))
    {
        ROS_LOG_ERROR(
            "RosKmAdapter::Start(...) failed. (status=%!STATUS!)",
            status);
        return status;
    }
    auto stopKmAdapter = ROS_FINALLY::DoUnless([&]
    {
        PAGED_CODE();
        NTSTATUS tempStatus = RosKmAdapter::Stop();
        UNREFERENCED_PARAMETER(tempStatus);
        NT_ASSERT(NT_SUCCESS(tempStatus));
    });

#if VC4
    //
    // m_deviceInfo.TranslatedResourceList has the HW resource information
    // (MMIO register file, interrupt, etc)
    //
#endif

    CM_PARTIAL_RESOURCE_LIST       *pResourceList = &m_deviceInfo.TranslatedResourceList->List->PartialResourceList;
    CM_PARTIAL_RESOURCE_DESCRIPTOR *pResource = &pResourceList->PartialDescriptors[0];

#if VC4

    auto unmapVc4Registers = ROS_FINALLY::DoUnless([&] {
        PAGED_CODE();
        NTSTATUS tempStatus = m_DxgkInterface.DxgkCbUnmapMemory(
                m_DxgkInterface.DeviceHandle,
                m_pVC4RegFile);
        UNREFERENCED_PARAMETER(tempStatus);
        NT_ASSERT(NT_SUCCESS(tempStatus));
    }, true);   // DoNot by default

    auto closeRpiq = ROS_FINALLY::DoUnless([&]
    {
        PAGED_CODE();
        ObDereferenceObjectWithTag(m_pRpiqDevice, ROS_ALLOC_TAG::DEVICE);
    }, true);   // DoNot by default

    auto powerDownVc4 = ROS_FINALLY::DoUnless([&]
    {
        PAGED_CODE();
        NTSTATUS tempStatus = SetVC4Power(false);
        UNREFERENCED_PARAMETER(tempStatus);
        NT_ASSERT(NT_SUCCESS(tempStatus));
    }, true);   // DoNot by default

    if (m_flags.m_isVC4)
    {
        void* voidPtr;
        status = m_DxgkInterface.DxgkCbMapMemory(
            m_DxgkInterface.DeviceHandle,
            pResource->u.Memory.Start,
            pResource->u.Memory.Length,
            FALSE,
            FALSE,
            MmNonCached,
            &voidPtr);
        if (!NT_SUCCESS(status))
        {
            ROS_LOG_ERROR(
                "Failed to map memory for VC4 renderer registers. (status=%!STATUS!, pResource=0x%p)",
                status,
                pResource);
            return status;
        }
        m_pVC4RegFile = static_cast<VC4_REGISTER_FILE*>(voidPtr);
        unmapVc4Registers.DoNot(false); // arm the cleanup action

        // TODO[jordanrh]: get device path from rpiq.h
        DECLARE_CONST_UNICODE_STRING(rpiqDeviceName, L"\\DosDevices\\RPIQ");

        status = RosOpenDevice(
                const_cast<UNICODE_STRING*>(&rpiqDeviceName),
                FILE_ALL_ACCESS,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                &m_pRpiqDevice,
                ROS_ALLOC_TAG::DEVICE);
        if (!NT_SUCCESS(status))
        {
            ROS_LOG_ERROR(
                "Failed to open handle to rpiq device. (status=%!STATUS!, rpiqDeviceName=%wZ)",
                status,
                &rpiqDeviceName);
            return status;
        }
        closeRpiq.DoNot(false);

        status = SetVC4Power(true);
        if (!NT_SUCCESS(status))
        {
            ROS_LOG_ERROR(
                "Failed to power on VC4. (status=%!STATUS!)",
                status);
            return status;
        }
        powerDownVc4.DoNot(false);

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

#endif // DBG
    }

#if USE_SIMPENROSE

    if (g_bUseSimPenrose)
    {
        simpenrose_init_hardware_supply_mem(RosKmdGlobal::s_pVideoMemory, RosKmdGlobal::s_videoMemorySize);
        simpenrose_set_mem_base(RosKmdGlobal::s_videoMemoryPhysicalAddress.LowPart + m_busAddressOffset);
    }

#endif // USE_SIMPENROSE

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

#endif // VC4

    auto disableInterrupt = ROS_FINALLY::DoUnless([&] {
        PAGED_CODE();
        m_bReadyToHandleInterrupt = FALSE;
        WRITE_REGISTER_ULONG(reinterpret_cast<volatile ULONG*>(
            &m_pVC4RegFile->V3D_INTENA),
            0);
    }, true);

    if (g_bUseInterrupt)
    {
        //
        // Enable End of Frame interrupt when Render Control List completes
        //

        V3D_REG_INTENA  regIntEna = { 0 };

        regIntEna.EI_FRDONE = 1;

        // TODO[jordanrh]: register operations should use READ/WRITE_REGISTER_ULONG
        WRITE_REGISTER_ULONG(reinterpret_cast<volatile ULONG*>(
            &m_pVC4RegFile->V3D_INTENA),
            regIntEna.Value);

        m_bReadyToHandleInterrupt = TRUE;
        disableInterrupt.DoNot(false);
    }

    // Initialize display subsystem
    auto stopDisplay = ROS_FINALLY::DoUnless([&]
    {
        PAGED_CODE();
        m_display.StopDevice();
    }, true);   // cleanup action disabled by default

    if (!RosKmdGlobal::IsRenderOnly())
    {
        // initialize display components
        status = m_display.StartDevice(
                _RENDERER_CM_RESOURCE_COUNT,    // FirstResourceIndex
                NumberOfVideoPresentSources,
                NumberOfChildren);
        if (!NT_SUCCESS(status))
        {
            ROS_LOG_ERROR(
                "Failed to initialize display subsystem. (status=%!STATUS!)",
                status);
            return status;
        }
        stopDisplay.DoNot(false);   // arm the cleanup action
    }
    else
    {
        //
        // Render only device has no VidPn source and target
        //
        *NumberOfVideoPresentSources = 0;
        *NumberOfChildren = 0;
    }

    stopKmAdapter.DoNot();
    unmapVc4Registers.DoNot();
    closeRpiq.DoNot();
    powerDownVc4.DoNot();
    disableInterrupt.DoNot();
    stopDisplay.DoNot();

    ROS_LOG_TRACE("RosKmdRapAdapter successfully started.");
    return STATUS_SUCCESS;
}

NTSTATUS RosKmdRapAdapter::Stop ()
{
    NTSTATUS status;
    UNREFERENCED_PARAMETER(status);
    
    ROS_LOG_TRACE("Stopping RosKmdRapAdapter");

    if (!RosKmdGlobal::IsRenderOnly())
    {
        m_display.StopDevice();
    }

    // disable interrupts
    if (g_bUseInterrupt)
    {
        NT_ASSERT(m_bReadyToHandleInterrupt);
        WRITE_REGISTER_ULONG(reinterpret_cast<volatile ULONG*>(&m_pVC4RegFile->V3D_INTENA), 0);
    }
    else
    {
        NT_ASSERT(!m_bReadyToHandleInterrupt);
    }

    if (m_flags.m_isVC4)
    {
        // power down VC4
        status = SetVC4Power(false);
        NT_ASSERT(NT_SUCCESS(status));

        // close RPIQ file handle
        NT_ASSERT(m_pRpiqDevice);
        ObDereferenceObjectWithTag(m_pRpiqDevice, ROS_ALLOC_TAG::DEVICE);
        m_pRpiqDevice = nullptr;

        // unmap memory
        NT_ASSERT(m_pVC4RegFile);
        status = m_DxgkInterface.DxgkCbUnmapMemory(
                m_DxgkInterface.DeviceHandle,
                m_pVC4RegFile);
        NT_ASSERT(NT_SUCCESS(status));
        m_pVC4RegFile = nullptr;
    }
    else
    {
        NT_ASSERT(!m_pRpiqDevice);
        NT_ASSERT(!m_pVC4RegFile);
    }

    return RosKmAdapter::Stop();
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

#if USE_SIMPENROSE

        if (g_bUseSimPenrose)
        {
            //
            // SimPenrose requires CL to be in the "local video memory segment"
            //

            //
            // Copy the Binning CL to the end of Render CL pool
            //
            BYTE           *pBinningCL = m_pControlListPool + VC4_RENDERING_CTRL_LIST_POOL_SIZE - ROSD_COMMAND_BUFFER_SIZE;
            UINT            binningCLPhysicalAddress = m_controlListPoolPhysicalAddress + VC4_RENDERING_CTRL_LIST_POOL_SIZE - ROSD_COMMAND_BUFFER_SIZE;

            memcpy(pBinningCL, pDmaBufInfo->m_pDmaBuffer, pDmaBufSubmission->m_EndOffset);

            //
            // Re-patch the Binning CL
            //
            for (UINT i = 0; i < pDmaBufInfo->m_DmaBufState.m_NumDmaBufSelfRef; i++)
            {
                D3DDDI_PATCHLOCATIONLIST   *pPatchLoc = &pDmaBufInfo->m_DmaBufSelfRef[i];

                *((UINT *)(pBinningCL + pPatchLoc->PatchOffset)) =
                    binningCLPhysicalAddress +
                    m_busAddressOffset +
                    pPatchLoc->AllocationOffset;
            }

            simpenrose_do_binning(
                binningCLPhysicalAddress + m_busAddressOffset + pDmaBufSubmission->m_StartOffset + sizeof(GpuCommand),
                binningCLPhysicalAddress + m_busAddressOffset + pDmaBufSubmission->m_EndOffset);

            //
            // Generate the Rendering Control List
            //
            UINT    renderingControlListLength;
            renderingControlListLength = GenerateRenderingControlList(pDmaBufInfo);

            simpenrose_do_rendering(
                m_renderingControlListPhysicalAddress + m_busAddressOffset,
                m_renderingControlListPhysicalAddress + m_busAddressOffset + renderingControlListLength);

            MoveToNextBinnerRenderMemChunk(renderingControlListLength);
        }
        else

#endif // USE_SIMPENROSE

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

            ROS_LOG_TRACE(
                "Completed rendering to 0x%p",
                pDmaBufInfo->m_RenderTargetVirtualAddress);
                
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

        //
        // No need to wait for binning to be done.
        // Render job waits on sempahore to be signaled by binning job.
        //
        return;
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
    LARGE_INTEGER timeOut;

    if (! g_bUseInterrupt)
    {
        //
        // Set time out to 64 millisecond
        //

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
    }
    else
    {
        //
        // Set time out to 2 seconds
        //

        timeOut.QuadPart = -2000 * 1000 * 1000 / 10;

        status = KeWaitForSingleObject(
            &m_hwDmaBufCompletionEvent,
            Executive,
            KernelMode,
            FALSE,
            &timeOut);

        NT_ASSERT(status == STATUS_SUCCESS);
    }
}

UINT
RosKmdRapAdapter::GenerateRenderingControlList(
    ROSDMABUFINFO *pDmaBufInfo)
{
    RosKmdAllocation *pRenderTarget = pDmaBufInfo->m_pRenderTarget;

    // Write Clear Colors command from UMD
    VC4ClearColors *pVC4ClearColors;
    VC4WaitOnSemaphore *pVC4WaitOnSempahore;

    if (pDmaBufInfo->m_DmaBufState.m_HasVC4ClearColors)
    {
        pVC4ClearColors = (VC4ClearColors *)m_pRenderingControlList;

        *pVC4ClearColors = pDmaBufInfo->m_VC4ClearColors;

        MoveToNextCommand(pVC4ClearColors, pVC4WaitOnSempahore);
    }
    else
    {
        pVC4WaitOnSempahore = (VC4WaitOnSemaphore *)m_pRenderingControlList;
    }

    // Wait binning to be done.

    VC4WaitOnSemaphore waitOnSemaphore = vc4WaitOnSemaphore;
    *pVC4WaitOnSempahore = waitOnSemaphore;

    VC4TileRenderingModeConfig *pVC4TileRenderingModeConfig;
    MoveToNextCommand(pVC4WaitOnSempahore, pVC4TileRenderingModeConfig);

    // Write Tile Rendering Mode Config command

    VC4TileRenderingModeConfig  tileRenderingModeConfig = vc4TileRenderingModeConfig;

    tileRenderingModeConfig.MemoryAddress = pDmaBufInfo->m_RenderTargetPhysicalAddress + m_busAddressOffset;

    tileRenderingModeConfig.WidthInPixels = (USHORT)pRenderTarget->m_mip0Info.TexelWidth;
    tileRenderingModeConfig.HeightInPixels = (USHORT)pRenderTarget->m_mip0Info.TexelHeight;

    tileRenderingModeConfig.NonHDRFrameBufferColorFormat = static_cast<USHORT>(
        Vc4FrameBufferColorFormatFromDxgiFormat(pRenderTarget->m_format));

    tileRenderingModeConfig.MemoryFormat = static_cast<USHORT>(
        Vc4MemoryFormatFromRosHwLayout(pRenderTarget->m_hwLayout));

    *pVC4TileRenderingModeConfig = tileRenderingModeConfig;

    // Clear the tile buffer by store the 1st tile
    VC4TileCoordinates *pVC4TileCoordinates = NULL;
    VC4StoreTileBufferGeneral  *pVC4StoreTileBufferGeneral = NULL;

    if (pDmaBufInfo->m_DmaBufState.m_HasVC4ClearColors)
    {
        MoveToNextCommand(pVC4TileRenderingModeConfig, pVC4TileCoordinates);

        *pVC4TileCoordinates = vc4TileCoordinates;

        MoveToNextCommand(pVC4TileCoordinates, pVC4StoreTileBufferGeneral);

        *pVC4StoreTileBufferGeneral = vc4StoreTileBufferGeneral;
    }

    //
    // Calling control list generated by the Binning Control List
    //
    UINT    widthInTiles = pRenderTarget->m_hwWidthPixels / VC4_BINNING_TILE_PIXELS;
    UINT    heightInTiles = pRenderTarget->m_hwHeightPixels / VC4_BINNING_TILE_PIXELS;

    VC4TileCoordinates  tileCoordinates = vc4TileCoordinates;
    VC4BranchToSubList  branchToSubList = vc4BranchToSubList;
    VC4LoadTileBufferGeneral    loadTileBufColor = vc4LoadTileBufferGeneral;
    UINT    tileAllocationPhysicalAddress = m_tileAllocationMemoryPhysicalAddress + m_busAddressOffset;
    VC4LoadTileBufferGeneral    *pVC4LoadTileBufGeneral = NULL;
    VC4BranchToSubList *pVC4BranchToSubList = NULL;
    VC4StoreMSResolvedTileColorBuf *pVC4StoreMSResolvedTileColorBuf = NULL;
    VC4StoreMSResolvedTileColorBufAndSignalEndOfFrame  *pVC4StoreMSResolvedTileColorBufAndSignalEndOfFrame = NULL;

    if (pDmaBufInfo->m_DmaBufState.m_HasVC4ClearColors)
    {
        MoveToNextCommand(pVC4StoreTileBufferGeneral, pVC4TileCoordinates);
        pVC4StoreMSResolvedTileColorBufAndSignalEndOfFrame = (VC4StoreMSResolvedTileColorBufAndSignalEndOfFrame *)pVC4TileCoordinates;
    }
    else
    {
        if (pDmaBufInfo->m_pRenderTarget)
        {
            loadTileBufColor.BufferToLoad = VC4_TILE_BUFFER_COLOR;

            loadTileBufColor.Fortmat = static_cast<USHORT>(
                Vc4MemoryFormatFromRosHwLayout(
                    pDmaBufInfo->m_pRenderTarget->m_hwLayout));

            loadTileBufColor.PixelColorFormat = static_cast<USHORT>(
                Vc4TileBufferPixelFormatFromDxgiFormat(
                    pDmaBufInfo->m_pRenderTarget->m_format));

            loadTileBufColor.MemoryBaseAddress = (pDmaBufInfo->m_RenderTargetPhysicalAddress + m_busAddressOffset) >> 4;

            MoveToNextCommand(pVC4TileRenderingModeConfig, pVC4LoadTileBufGeneral);
        }
    }

    for (UINT x = 0; x < widthInTiles; x++)
    {
        for (UINT y = 0; y < heightInTiles; y++)
        {
            if (! pDmaBufInfo->m_DmaBufState.m_HasVC4ClearColors)
            {
                if (pDmaBufInfo->m_pRenderTarget)
                {
                    *pVC4LoadTileBufGeneral = loadTileBufColor;

                    MoveToNextCommand(pVC4LoadTileBufGeneral, pVC4TileCoordinates);
                }
            }

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

                if (pDmaBufInfo->m_DmaBufState.m_HasVC4ClearColors)
                {
                    MoveToNextCommand(pVC4StoreMSResolvedTileColorBuf, pVC4TileCoordinates);
                }
                else
                {
                    MoveToNextCommand(pVC4StoreMSResolvedTileColorBuf, pVC4LoadTileBufGeneral);
                }
            }
        }
    }

    return ((UINT)(((PBYTE)pVC4StoreMSResolvedTileColorBufAndSignalEndOfFrame) - m_pRenderingControlList));
}

NTSTATUS
RosKmdRapAdapter::SetVC4Power(
        bool    bOn)
{
    MAILBOX_SET_POWER_VC4 setPowerVC4;
    INIT_MAILBOX_SET_POWER_VC4(&setPowerVC4, bOn);

    ULONG_PTR information;
    NTSTATUS status = RosSendIoctlSynchronously(
            m_pRpiqDevice,
            IOCTL_MAILBOX_PROPERTY,
            &setPowerVC4,
            sizeof(setPowerVC4),
            &setPowerVC4,
            sizeof(setPowerVC4),
            FALSE,                  // InternalDeviceIoControl
            &information);
    if (!NT_SUCCESS(status) || (information != sizeof(setPowerVC4)))
    {
        ROS_LOG_ERROR(
            "Failed to send IOCTL_MAILBOX_PROPERTY to rpiq. (status=%!STATUS!, information=%Id, sizeof(setPowerVC4)=%d, m_pRpiqDevice=0x%p)",
            status,
            information,
            sizeof(setPowerVC4),
            m_pRpiqDevice);
        return NT_SUCCESS(status) ? STATUS_UNSUCCESSFUL : status;
    }

    if (setPowerVC4.Header.RequestResponse == RESPONSE_SUCCESS)
    {
        return STATUS_SUCCESS;
    }
    else
    {
        return STATUS_UNSUCCESSFUL;
    }
}

ROS_NONPAGED_SEGMENT_BEGIN; //================================================

_Use_decl_annotations_
BOOLEAN RosKmdRapAdapter::InterruptRoutine(ULONG MessageNumber)
{
    if (this->RendererInterruptRoutine(MessageNumber))
        return TRUE;
    
    if (RosKmdGlobal::IsRenderOnly())
        return FALSE;
    
    return m_display.InterruptRoutine(MessageNumber);
}

_Use_decl_annotations_
BOOLEAN RosKmdRapAdapter::RendererInterruptRoutine (ULONG /*MessageNumber*/)
{
    if (!m_bReadyToHandleInterrupt)
    {
        return FALSE;
    }

    V3D_REG_INTCTL  regIntCtl;

    regIntCtl.Value = m_pVC4RegFile->V3D_INTCTL;

    if (regIntCtl.INT_FRDONE)
    {
        regIntCtl.Value = 0;
        regIntCtl.INT_FRDONE = 1;

        // Acknowledge the interrupt
        m_pVC4RegFile->V3D_INTCTL = regIntCtl.Value;

        // If the interrupt is for DMA buffer completion,
        // queue the DPC to wake up the worker thread
        KeInsertQueueDpc(&m_hwDmaBufCompletionDpc, NULL, NULL);

        return TRUE;
    }

    return FALSE;
}

ROS_NONPAGED_SEGMENT_END; //==================================================

