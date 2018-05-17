#pragma once

#include "CosGpuCommand.h"

class CosUmdDevice;
class CosUmdResource;

#if VC4

//
// UMD only writes the Binning Control List
//

#endif

class CosUmdCommandBuffer
{
public:
    CosUmdCommandBuffer();
    ~CosUmdCommandBuffer();

    void Standup(CosUmdDevice * pCosUmdDevice, D3DDDICB_CREATECONTEXT * pCreateContext);

    void CopyResource(CosUmdResource * pDstResource, CosUmdResource * pSrcResource);
    void WriteResource(CosUmdResource * pResource, void * pData);

    void Flush(UINT flushFlags);

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
    UseResource(
        CosUmdResource *    pResource,
        BOOL                bWriteOperation);

    bool
    IsResourceUsed(
        CosUmdResource *    pResource);

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

        // Equivalent of D3D11 pipeline bind point - only informational for now
        pPatchLocation->SlotId = slotId;

        // UMD can use driverID to pass additional information to KMD - this will always be
        // zero and is unused
        pPatchLocation->DriverId = driverId;

        // With driver support, kernel runtime can run DMA buffer in multiple pieces
        // We will not support splitting of dma buffers and will always provide zero
        pPatchLocation->SplitOffset = dmaBufferSplitOffset;

        pPatchLocation++;
    }

    // FlushIfMatching will block flushing the current command buffer if the given fence
    // value matches the current command buffers fence value.
    void FlushIfMatching(ULONGLONG fence);

    bool IsCommandBufferEmpty();
    bool IsSwCommandBuffer();

#if VC4

    void UpdateClearColor(UINT clearColor);
    void UpdateClearDepthStencil(FLOAT depthValue, UINT8 stencilValue);

#endif

    //
    // Fence constants
    //

    static const ULONGLONG s_nullFence = 0ULL;
    static const ULONGLONG s_firstFence = 1ULL;

private:

    CosUmdDevice *                      m_pCosUmdDevice;

    BYTE *                              m_pCommandBuffer;
    UINT                                m_commandBufferSize;
    UINT                                m_commandBufferPos;

    D3DDDI_ALLOCATIONLIST *             m_pAllocationList;
    UINT                                m_allocationListSize;
    UINT                                m_allocationListPos;

    D3DDDI_PATCHLOCATIONLIST *          m_pPatchLocationList;
    UINT                                m_patchLocationListSize;
    UINT                                m_patchLocationListPos;

    // Submission fence value is unique for each command buffer submission.  The first
    // submitted command buffer has a fence value of s_firstFence.  With every submission, this
    // fence value is incremented by one.  We do not handle fence wrap around given that
    // it is a 64 bit value and do not expect the value to ever wrap.

    ULONGLONG                           m_submissionFence;

    GpuCommand *                        m_pCmdBufHeader;

    CONST UINT  COMMAND_BUFFER_FLUSH_THRESHOLD = 512;
    CONST UINT  ALLOCATION_LIST_FLUSH_THRESHOLD = 2;
    CONST UINT  PACTH_LOCATION_LIST_FLUSH_THRESHOLD = 2;
};

template<typename TypeCur, typename TypeNext>
void MoveToNextCommand(TypeCur pCurCommand, TypeNext &pNextCommand, UINT &curComamndOffset)
{
    curComamndOffset += sizeof(*pCurCommand);

    pNextCommand = (TypeNext)(pCurCommand + 1);
}

template<typename TypeCur, typename TypeNext>
void MoveToNextCommand(TypeCur pCurCommand, UINT variableSize, TypeNext &pNextCommand, UINT &curComamndOffset)
{
    curComamndOffset += (sizeof(*pCurCommand) + variableSize);

    pNextCommand = (TypeNext)(pCurCommand + 1);
}
