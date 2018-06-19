////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Compute Command List implementation
//
// Filled up Command Buffers are kept in a m_filledCommandBuffers and submitted to kernel runtime one by one
// 
// The maximal number of Command Buffers is limited by COS_MAX_NUM_COMMAND_BUFFERS
//
// SW and HW commands are kept in separate command buffers so that KMD can emulate or submit to HW directly
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"


HRESULT CosUmd12CommandList::StandUp()
{
    if (IsComputeType())
    {
        m_pDevice->m_pUMCallbacks->pfnSetCommandListDDITableCb(m_args.hRTCommandList, m_pDevice->m_pAdapter->m_hRTTable[CosUmd12Adapter::TableType::Compute]);
    }
    else
    {
        m_pDevice->m_pUMCallbacks->pfnSetCommandListDDITableCb(m_args.hRTCommandList, m_pDevice->m_pAdapter->m_hRTTable[CosUmd12Adapter::TableType::Render]);
    }

    m_pCommandAllocator = CosUmd12CommandAllocator::CastFrom(m_args.hDrvCommandAllocator);

    m_pCurCommandBuffer = m_pCommandAllocator->AcquireCommandBuffer(m_args.QueueFlags);

    if (NULL == m_pCurCommandBuffer)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

void
CosUmd12CommandList::Close()
{
    if (m_pCurCommandBuffer->IsCommandBufferEmpty())
    {
        return;
    }

    m_filledCommandBuffers[m_numFilledCommandBuffers++] = m_pCurCommandBuffer;

    m_pCurCommandBuffer = NULL;
}

void
CosUmd12CommandList::Execute(CosUmd12CommandQueue * pCommandQueue)
{
    HRESULT hr;

    for (UINT i = 0; i < m_numFilledCommandBuffers; i++)
    {
        hr = m_filledCommandBuffers[i]->Execute(pCommandQueue);
    }
}

void
CosUmd12CommandList::ResourceCopy(
    D3D12DDI_HRESOURCE DstResource,
    D3D12DDI_HRESOURCE SrcResource)
{
    CosUmd12Resource *  pDstResource = CosUmd12Resource::CastFrom(DstResource);
    CosUmd12Resource *  pSrcResource = CosUmd12Resource::CastFrom(SrcResource);

    BYTE *  pCommandBuffer;
    UINT    curCommandOffset;
    D3DDDI_PATCHLOCATIONLIST *  pPatchLocationList;

    GpuCommand * command;

    ReserveCommandBufferSpace(
        true,                           // SW command
        sizeof(*command),
        &pCommandBuffer,
        2,
        2,
        &curCommandOffset,
        &pPatchLocationList);
    if (NULL == pCommandBuffer)
    {
        return;
    }

    assert(pPatchLocationList != NULL);

    command = reinterpret_cast<GpuCommand *>(pCommandBuffer);

    command->m_commandId = GpuCommandId::ResourceCopy;
    command->m_resourceCopy.m_srcGpuAddress.QuadPart = 0;
    command->m_resourceCopy.m_dstGpuAddress.QuadPart = 0;
    command->m_resourceCopy.m_sizeBytes = pDstResource->GetDataSize();

    UINT dstAllocIndex = m_pCurCommandBuffer->UseResource(pDstResource, true);
    UINT srcAllocIndex = m_pCurCommandBuffer->UseResource(pSrcResource, false);

    m_pCurCommandBuffer->SetPatchLocation(
                            pPatchLocationList,
                            dstAllocIndex,
                            curCommandOffset + offsetof(GpuCommand, m_resourceCopy.m_dstGpuAddress),
                            pDstResource->GetHeapOffset());
    m_pCurCommandBuffer->SetPatchLocation(
                            pPatchLocationList,
                            srcAllocIndex,
                            curCommandOffset + offsetof(GpuCommand, m_resourceCopy.m_srcGpuAddress),
                            pSrcResource->GetHeapOffset());

    m_pCurCommandBuffer->CommitCommandBufferSpace(sizeof(*command), 2);
}

void
CosUmd12CommandList::ReserveCommandBufferSpace(
    bool                        bSwCommand,
    UINT                        commandSize,
    BYTE **                     ppCommandBuffer,
    UINT                        allocationListSize,
    UINT                        patchLocationSize,
    UINT *                      pCurCommandOffset,
    D3DDDI_PATCHLOCATIONLIST ** ppPatchLocationList)
{
    m_pCurCommandBuffer->ReserveCommandBufferSpace(
                            bSwCommand,
                            commandSize,
                            ppCommandBuffer,
                            allocationListSize,
                            patchLocationSize,
                            pCurCommandOffset,
                            ppPatchLocationList);

    //
    // The current command buffer has enough space for the new command
    //

    if (*ppCommandBuffer)
    {
        return;
    }

    //
    // New command buffer need to be allocated
    //

    if (m_numFilledCommandBuffers == (COS_MAX_NUM_COMMAND_BUFFERS - 1))
    {
        m_pDevice->m_pUMCallbacks->pfnSetErrorCb(m_pDevice->m_hRTDevice, D3DDDIERR_DEVICEREMOVED);

        return;
    }

    m_filledCommandBuffers[m_numFilledCommandBuffers++] = m_pCurCommandBuffer;

    m_pCurCommandBuffer = m_pCommandAllocator->AcquireCommandBuffer(m_args.QueueFlags);

    m_pCurCommandBuffer->ReserveCommandBufferSpace(
                            bSwCommand,
                            commandSize,
                            ppCommandBuffer,
                            allocationListSize,
                            patchLocationSize,
                            pCurCommandOffset,
                            ppPatchLocationList);
}

