////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Root Signature implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

#if COS_GPUVA_SUPPORT

void 
CosUmd12RootSignature::SetRootDescriptorTable(
    BYTE * pRootValues,
    CosUmd12DescriptorHeap * pDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
    UINT rootParameterIndex,
    D3D12DDI_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
    TRACE_FUNCTION();

    CosUmd12DescriptorHeap * pDescriptorHeap = NULL;

    switch (m_rootSignature.pRootParameters[rootParameterIndex].DescriptorTable.pDescriptorRanges[0].RangeType)
    {
    case D3D12DDI_DESCRIPTOR_RANGE_TYPE_SRV:
    case D3D12DDI_DESCRIPTOR_RANGE_TYPE_UAV:
    case D3D12DDI_DESCRIPTOR_RANGE_TYPE_CBV:
        pDescriptorHeap = pDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
        break;
    case D3D12DDI_DESCRIPTOR_RANGE_TYPE_SAMPLER:
        //
        // TODO : 
        //
        ASSERT(false);
        break;
    }

    D3D12DDI_GPU_VIRTUAL_ADDRESS heapGpuAddress = pDescriptorHeap->GetGpuAddress();

    ASSERT(baseDescriptor.ptr >= heapGpuAddress);
    ASSERT(0 == ((baseDescriptor.ptr - heapGpuAddress) % sizeof(CosUmd12Descriptor)));

    UINT gpuVaOffset = (UINT)(baseDescriptor.ptr - pDescriptorHeap->GetGpuAddress());

    *((UINT *)(pRootValues + m_pRootValueOffsets[rootParameterIndex])) = gpuVaOffset;
}

void
CosUmd12RootSignature::SetRoot32BitConstants(
    BYTE * pRootValues,
    UINT rootParameterIndex,
    UINT num32BitValuesToSet,
    const void* pSrcData,
    UINT destOffsetIn32BitValues)
{
    TRACE_FUNCTION();

    FLOAT * pRootConstant = (FLOAT *)(pRootValues + m_pRootValueOffsets[rootParameterIndex]);

    memcpy(pRootConstant + destOffsetIn32BitValues, pSrcData, sizeof(FLOAT)*num32BitValuesToSet);
}

void
CosUmd12RootSignature::SetRootView(
    BYTE * pRootValues,
    UINT rootParameterIndex,
    D3D12DDI_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    TRACE_FUNCTION();

    *((D3D12DDI_GPU_VIRTUAL_ADDRESS *)(pRootValues + m_pRootValueOffsets[rootParameterIndex])) = bufferLocation;
}

UINT CosUmd12RootSignature::GetHwRootSignatureSize()
{
    UINT commandSize = FIELD_OFFSET(GpuHWRootSignature2LSet, m_rootValues) +
                       m_sizeOfRootValues +
                       sizeof(GpuHwDescriptorHeapSet);

    return commandSize;
}

void CosUmd12RootSignature::WriteHWRootSignature(
    BYTE * pRootValues,
    CosUmd12DescriptorHeap * pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
    BYTE * pCommandBuf)
{
    const CosUmd12DescriptorHeap * pDescriptorHeap = pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
    GpuHwDescriptorHeapSet * pHwDescriptorHeapSet = nullptr;
    GpuHWRootSignature2LSet * pHwRootSignatureSet = nullptr;

    //
    // Write GpuHwDescriptorHeapSet command
    //

    pHwDescriptorHeapSet = (GpuHwDescriptorHeapSet *)pCommandBuf;

    pHwDescriptorHeapSet->m_commandId = DescriptorHeapSet;
    if (pDescriptorHeap)
    {
        pHwDescriptorHeapSet->m_descriptorHeapGpuAddress.QuadPart = pDescriptorHeap->GetGpuAddress();
    }
    else
    {
        pHwDescriptorHeapSet->m_descriptorHeapGpuAddress.QuadPart = 0;
    }

    //
    // Write GpuHWRootSignature2LSet command
    //

    pHwRootSignatureSet = (GpuHWRootSignature2LSet *)(pHwDescriptorHeapSet + 1);

    pHwRootSignatureSet->m_commandId = RootSignature2LevelSet;
    pHwRootSignatureSet->m_commandSize = FIELD_OFFSET(GpuHWRootSignature2LSet, m_rootValues) + m_sizeOfRootValues;

    //
    // Copy root value into the command buffer
    //

    memcpy(pHwRootSignatureSet->m_rootValues, pRootValues, m_sizeOfRootValues);
}

#endif  // COS_GPUVA_SUPPORT

