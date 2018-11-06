////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Command Buffer implementation using Gpu Va
//
// Command Buffer using Gpu Va can be submitted to kernel runtime individually or chained together
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

#if COS_GPUVA_SUPPORT

CosUmd12CommandBuffer *
CosUmd12CommandBuffer::Create(
    CosUmd12Device *    pDevice)
{
    D3D12DDIARG_CREATEHEAP_0001 heapDesc;

    heapDesc.ByteSize = COS_COMMAND_BUFFER_SIZE;
    heapDesc.Alignment = PAGE_SIZE;
    heapDesc.MemoryPool = D3D12DDI_MEMORY_POOL_L0;
    heapDesc.CPUPageProperty = D3D12DDI_CPU_PAGE_PROPERTY_WRITE_COMBINE;
    heapDesc.Flags = D3D12DDI_HEAP_FLAG_BUFFERS;
    heapDesc.CreationNodeMask = 1;
    heapDesc.VisibleNodeMask = 1;

    CosUmd12CommandBuffer * pCommandBuffer = new CosUmd12CommandBuffer(pDevice, MAKE_D3D10DDI_HRTRESOURCE(NULL), &heapDesc);
    if (nullptr == pCommandBuffer)
    {
        return nullptr;
    }

    HRESULT hr;

    hr = pCommandBuffer->Standup();
    if (FAILED(hr))
    {
        delete pCommandBuffer;
        pCommandBuffer = nullptr;
    }

    return pCommandBuffer;
}

CosUmd12CommandBuffer::CosUmd12CommandBuffer(
    CosUmd12Device *    pDevice,
    D3D12DDI_HRTRESOURCE    hRTHeap,
    const D3D12DDIARG_CREATEHEAP_0001 * pHeapDesc) :
    m_commandHeap(pDevice, hRTHeap, pHeapDesc)
{
    m_pCommandBuffer = NULL;
    m_commandBufferSize = (UINT)pHeapDesc->ByteSize;

    m_commandBufferPos = 0;
}

HRESULT CosUmd12CommandBuffer::Standup()
{
    HRESULT hr;

    hr = m_commandHeap.Standup();
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_commandHeap.Map((void **)&m_pCommandBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    //
    // Write header into command buffer (for KMD)
    //

    m_pCmdBufHeader = (GpuCommand *)m_pCommandBuffer;
    m_pCmdBufHeader->m_commandId = Header;
    m_pCmdBufHeader->m_commandBufferHeader.m_swCommandBuffer = 0;
    m_pCmdBufHeader->m_commandBufferHeader.m_gpuVaCommandBuffer = 1;

    m_commandBufferPos += sizeof(GpuCommand);

    return hr;
}

void CosUmd12CommandBuffer::Teardown()
{
    m_commandHeap.Teardown();
}

CosUmd12CommandBuffer::~CosUmd12CommandBuffer()
{
}

bool CosUmd12CommandBuffer::IsCommandBufferEmpty()
{
    return (m_commandBufferPos <= sizeof(GpuCommand));
}

void
CosUmd12CommandBuffer::ReserveCommandBufferSpace(
    UINT                        commandSize,
    BYTE **                     ppCommandBuffer)
{
    *ppCommandBuffer = NULL;

    if ((m_commandBufferPos + commandSize + COMMAND_BUFFER_FLUSH_THRESHOLD) > m_commandBufferSize)
    {
        return;
    }

    *ppCommandBuffer = m_pCommandBuffer + m_commandBufferPos;
}

void
CosUmd12CommandBuffer::CommitCommandBufferSpace(
    UINT    commandSize)
{
    // Update the command buffer position
    m_commandBufferPos += commandSize;
    assert((m_commandBufferPos + COMMAND_BUFFER_FLUSH_THRESHOLD) < m_commandBufferSize);
}

HRESULT
CosUmd12CommandBuffer::Execute(CosUmd12CommandQueue * pCommandQueue)
{
    if (IsCommandBufferEmpty())
    {
        return S_OK;
    }

    return pCommandQueue->ExecuteCommandBuffer(
                            m_commandHeap.GetGpuVa(),
                            m_commandBufferPos,
                            m_pCommandBuffer);

    return S_OK;
}

#endif  // COS_GPUVA_SUPPORT

