////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Descriptor Heap implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

#if COS_GPUVA_SUPPORT

int CosUmd12DescriptorHeap::CalculateSize(const D3D12DDIARG_CREATE_DESCRIPTOR_HEAP_0001 * pDesc)
{
    return sizeof(CosUmd12DescriptorHeap);
}

HRESULT CosUmd12DescriptorHeap::Create(
    CosUmd12Device *    pDevice,
    _In_ const D3D12DDIARG_CREATE_DESCRIPTOR_HEAP_0001 *    pDesc,
    D3D12DDI_HDESCRIPTORHEAP    DescriptorHeap)
{
    D3D12DDIARG_CREATEHEAP_0001 heapDesc = { 0 };
    UINT heapSize;

    heapSize = pDesc->NumDescriptors*sizeof(GpuHWDescriptor);
    heapSize = (heapSize + PAGE_SIZE - 1) & (~(PAGE_SIZE - 1));

    heapDesc.ByteSize = heapSize;
    heapDesc.Alignment = PAGE_SIZE;
    heapDesc.MemoryPool = D3D12DDI_MEMORY_POOL_L0;
    heapDesc.CPUPageProperty = D3D12DDI_CPU_PAGE_PROPERTY_WRITE_COMBINE;
    heapDesc.Flags = D3D12DDI_HEAP_FLAG_BUFFERS;
    heapDesc.CreationNodeMask = 1;
    heapDesc.VisibleNodeMask = 1;

    CosUmd12DescriptorHeap * pDescriptorHeap = new(DescriptorHeap.pDrvPrivate) CosUmd12DescriptorHeap(pDevice, pDesc, &heapDesc);

    HRESULT hr;

    hr = pDescriptorHeap->Standup();
    if (FAILED(hr))
    {
        pDescriptorHeap->Teardown();
    }

    return hr;
}

HRESULT CosUmd12DescriptorHeap::Standup()
{
    HRESULT hr;

    hr = m_hwDescriptorHeap.Standup();
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_hwDescriptorHeap.Map((void **)&m_pHwDescriptors);
    if (FAILED(hr))
    {
        return hr;
    }

    memset(m_pHwDescriptors, 0, m_desc.NumDescriptors*sizeof(CosUmd12Descriptor));

    return S_OK;
}

void CosUmd12DescriptorHeap::Teardown()
{
    m_hwDescriptorHeap.Teardown();
}

#endif  // COS_GPUVA_SUPPORT

