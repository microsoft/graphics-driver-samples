////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Command Buffer implementation
//
// Command Buffer is the unit of submission to kernel runtime
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

#if !COS_GPUVA_SUPPORT

CosUmd12CommandBuffer * CosUmd12CommandBuffer::Create()
{
    UINT size;

    size = 
        sizeof(CosUmd12CommandBuffer) +
        sizeof(BYTE)*COS_COMMAND_BUFFER_SIZE +
        sizeof(D3DDDI_ALLOCATIONLIST)*C_COS_ALLOCATION_LIST_SIZE +
        sizeof(D3DDDI_PATCHLOCATIONLIST)*C_COS_PATCH_LOCATION_LIST_SIZE;

    VOID * pMem;
    
    pMem = malloc(size);
    if (NULL == pMem)
    {
        return NULL;
    }

    CosUmd12CommandBuffer * pCommandBuffer = new (pMem) CosUmd12CommandBuffer(COS_COMMAND_BUFFER_SIZE, C_COS_ALLOCATION_LIST_SIZE, C_COS_PATCH_LOCATION_LIST_SIZE);

    return pCommandBuffer;
}

CosUmd12CommandBuffer::CosUmd12CommandBuffer(
    UINT CommandBufferSize,
    UINT AllocationListSize,
    UINT PatchLocationListSize)
{
    m_pCommandBuffer = (BYTE *)(this + 1);
    m_commandBufferSize = CommandBufferSize;

    m_pAllocationList = (D3DDDI_ALLOCATIONLIST *)(m_pCommandBuffer + m_commandBufferSize);
    m_allocationListSize = AllocationListSize;

    m_pPatchLocationList = (D3DDDI_PATCHLOCATIONLIST *)(m_pAllocationList + m_allocationListSize);
    m_patchLocationListSize = PatchLocationListSize;

    m_commandBufferPos = 0;
    m_allocationListPos = 0;
    m_patchLocationListPos = 0;

    //
    // Write header into command buffer (for KMD)
    //

    m_pCmdBufHeader = (GpuCommand *)m_pCommandBuffer;
    m_pCmdBufHeader->m_commandId = Header;
    m_pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer = 1;

    m_commandBufferPos += sizeof(GpuCommand);
}

CosUmd12CommandBuffer::~CosUmd12CommandBuffer()
{
    free(this);
}

bool CosUmd12CommandBuffer::IsCommandBufferEmpty()
{
    return (m_commandBufferPos <= sizeof(GpuCommand));
}

bool CosUmd12CommandBuffer::IsSwCommandBuffer()
{
    ASSERT(m_pCmdBufHeader->m_commandId == Header);
    return m_pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer;
}

void
CosUmd12CommandBuffer::ReserveCommandBufferSpace(
    bool                        bSwCommand,
    UINT                        commandSize,
    BYTE **                     ppCommandBuffer,
    UINT                        allocationListSize,
    UINT                        patchLocationSize,
    UINT *                      pCurCommandOffset,
    D3DDDI_PATCHLOCATIONLIST ** ppPatchLocationList)
{
    *ppCommandBuffer = NULL;

    if (IsSwCommandBuffer() != bSwCommand)
    {
        if (IsCommandBufferEmpty() == false)
        {
            return;
        }

        // Command buffer contains either SW or HW commands
        m_pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer = bSwCommand;
    }

    if (((m_commandBufferPos + commandSize + COMMAND_BUFFER_FLUSH_THRESHOLD) > m_commandBufferSize) ||
        ((m_allocationListPos + allocationListSize + ALLOCATION_LIST_FLUSH_THRESHOLD) > m_allocationListSize) ||
        ((m_patchLocationListPos + patchLocationSize + PACTH_LOCATION_LIST_FLUSH_THRESHOLD) > m_patchLocationListSize))
    {
        return;
    }

    *ppCommandBuffer = m_pCommandBuffer + m_commandBufferPos;

    if (allocationListSize)
    {
        *pCurCommandOffset = m_commandBufferPos;
        *ppPatchLocationList = m_pPatchLocationList + m_patchLocationListPos;
    }
}

void
CosUmd12CommandBuffer::CommitCommandBufferSpace(
    UINT    commandSize,
    UINT    patchLocationSize)
{
    // Update the command buffer position
    m_commandBufferPos += commandSize;
    assert((m_commandBufferPos + COMMAND_BUFFER_FLUSH_THRESHOLD) < m_commandBufferSize);

    // Update the patch location list position
    m_patchLocationListPos += patchLocationSize;
    assert((m_patchLocationListPos + PACTH_LOCATION_LIST_FLUSH_THRESHOLD) < m_patchLocationListSize);

    assert((m_allocationListPos + ALLOCATION_LIST_FLUSH_THRESHOLD) < m_allocationListSize);
}

//
// D3D12 resource can be referenced by multiple Command List/Buffer,
// so a search of the Allocation List is necessary
//

UINT
CosUmd12CommandBuffer::UseResource(
    CosUmd12Resource *  pResource,
    BOOL                bWriteOperation)
{
    return UseAllocation(pResource->GetHeapAllocationHandle(), bWriteOperation);
}

UINT
CosUmd12CommandBuffer::UseAllocation(
    D3DKMT_HANDLE   hAllocation,
    BOOL            bWriteOperation)
{
    UINT i;

    for (i = 0; i < m_allocationListPos; i++)
    {
        if (hAllocation == m_pAllocationList[i].hAllocation)
        {
            m_pAllocationList[i].WriteOperation = bWriteOperation;

            return i;
        }
    }

    auto pAllocationEntry = m_pAllocationList + i;

    pAllocationEntry->hAllocation = hAllocation;
    pAllocationEntry->Value = 0;
    pAllocationEntry->WriteOperation = bWriteOperation;

    // Update the allocation list position
    m_allocationListPos++;
    assert((m_allocationListPos + ALLOCATION_LIST_FLUSH_THRESHOLD) < m_allocationListSize);

    return i;
}

void
CosUmd12CommandBuffer::RecordGpuAddressReference(
    D3D12DDI_GPU_VIRTUAL_ADDRESS resourceGpuVA,
    UINT commandBufferOffset,
    D3DDDI_PATCHLOCATIONLIST * &pPatchLocations)
{
    D3DKMT_HANDLE hAllocation = (D3DKMT_HANDLE)(resourceGpuVA >> 32);
    UINT allocationOffset = (UINT)(resourceGpuVA & 0xFFFFFFFF);

    UINT allocIndex = UseAllocation(hAllocation, true);

    SetPatchLocation(
        pPatchLocations,
        allocIndex,
        commandBufferOffset,
        0,
        allocationOffset);
}

HRESULT
CosUmd12CommandBuffer::Execute(CosUmd12CommandQueue * pCommandQueue)
{
    if (IsCommandBufferEmpty())
    {
        return S_OK;
    }

    return pCommandQueue->ExecuteCommandBuffer(
                            m_pCommandBuffer,
                            m_commandBufferPos,
                            m_pAllocationList,
                            m_allocationListPos,
                            m_pPatchLocationList,
                            m_patchLocationListPos);
}

#endif  // !COS_GPUVA_SUPPORT
