#pragma once

#include "CosUmd12.h"

class CosUmd12CommandBuffer
{
public:
    static CosUmd12CommandBuffer * Create();

    CosUmd12CommandBuffer(
        UINT CommandBufferSize,
        UINT AllocationListSize,
        UINT PatchLocationListSize);

    ~CosUmd12CommandBuffer();

    HRESULT Standup();
    void Teardown();

    void
    ReserveCommandBufferSpace(
        bool                        bSwCommand,
        UINT                        commandSize,
        BYTE **                     ppCommandBuffer,
        UINT                        allocationListSize = 0,
        UINT                        patchLocationSize = 0,
        UINT *                      pCurCommandOffset = NULL,
        D3DDDI_PATCHLOCATIONLIST ** ppPatchLocationList = NULL);

    void
    CommitCommandBufferSpace(
        UINT    commandSize,
        UINT    patchLocationSize = 0);

    UINT
    UseAllocation(
        D3DKMT_HANDLE   hAllocation,
        BOOL            bWriteOperation);

    UINT
    UseResource(
        CosUmd12Resource *  pResource,
        BOOL                bWriteOperation);

    void
    SetPatchLocation(
        D3DDDI_PATCHLOCATIONLIST *  &pPatchLocation,
        UINT                        allocationIndex,
        UINT                        dmaBufferPatchOffset,
        UINT                        slotId = 0,
        UINT                        allocationOffset = 0,
        UINT                        driverId = 0,
        UINT                        dmaBufferSplitOffset = 0)
    {
        pPatchLocation->AllocationIndex = allocationIndex;
        pPatchLocation->PatchOffset = dmaBufferPatchOffset;
        pPatchLocation->AllocationOffset = allocationOffset;

        // Equivalent of D3D12 pipeline bind point - only informational for now
        pPatchLocation->SlotId = slotId;

        // UMD can use driverID to pass additional information to KMD - this will always be
        // zero and is unused
        pPatchLocation->DriverId = driverId;

        // With driver support, kernel runtime can run DMA buffer in multiple pieces
        // We will not support splitting of dma buffers and will always provide zero
        pPatchLocation->SplitOffset = dmaBufferSplitOffset;

        pPatchLocation++;
    }

    bool IsCommandBufferEmpty();

    void
    RecordGpuAddressReference(
        D3D12DDI_GPU_VIRTUAL_ADDRESS resourceGpuVA,
        UINT commandBufferOffset,
        D3DDDI_PATCHLOCATIONLIST * &pPatchLocations);

    // Interface for Command Queue
    HRESULT Execute(CosUmd12CommandQueue * pCommandQueue);

private:

    BYTE *                              m_pCommandBuffer;
    UINT                                m_commandBufferSize;
    UINT                                m_commandBufferPos;

    D3DDDI_ALLOCATIONLIST *             m_pAllocationList;
    UINT                                m_allocationListSize;
    UINT                                m_allocationListPos;

    D3DDDI_PATCHLOCATIONLIST *          m_pPatchLocationList;
    UINT                                m_patchLocationListSize;
    UINT                                m_patchLocationListPos;

    GpuCommand *                        m_pCmdBufHeader;

    bool IsSwCommandBuffer();

    CONST UINT  COMMAND_BUFFER_FLUSH_THRESHOLD = 512;
    CONST UINT  ALLOCATION_LIST_FLUSH_THRESHOLD = 2;
    CONST UINT  PACTH_LOCATION_LIST_FLUSH_THRESHOLD = 2;
};
