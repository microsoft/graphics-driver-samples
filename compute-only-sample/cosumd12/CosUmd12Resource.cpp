////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Resource implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

HRESULT
CosUmd12Resource::Standup(CosUmd12Heap * pHeap)
{
    m_pHeap = pHeap;

    ASSERT(m_desc.ResourceType == D3D12DDI_RT_BUFFER);
    m_dataSize = m_desc.Width*m_desc.Height;

    return S_OK;
}

#if COS_GPUVA_SUPPORT

D3D12DDI_GPU_VIRTUAL_ADDRESS CosUmd12Resource::GetGpuVa()
{
    return m_pHeap->GetGpuVa() + m_desc.ReuseBufferGPUVA.BaseAddress.UMD.Offset;
}

#endif

D3D12DDI_GPU_VIRTUAL_ADDRESS CosUmd12Resource::GetUniqueAddress()
{
    return (((D3D12DDI_GPU_VIRTUAL_ADDRESS)m_pHeap->GetAllocationHandle()) << 32) | m_desc.ReuseBufferGPUVA.BaseAddress.UMD.Offset;
}

