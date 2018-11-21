////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Root Signature implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

#if COS_RS_2LEVEL_SUPPORT

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

void
CosUmd12RootSignature::PrepareHWRootSignature()
{
    m_numRootViews = 0;
    m_numDescriptorsUsed = 0;

    const D3D12DDI_ROOT_PARAMETER_0013 * pRootParameter = m_rootSignature.pRootParameters;

    for (UINT i = 0; i < m_rootSignature.NumParameters; i++, pRootParameter++)
    {
        switch (pRootParameter->ParameterType)
        {
        case D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            for (UINT j = 0; j < pRootParameter->DescriptorTable.NumDescriptorRanges; j++)
            {
                const D3D12DDI_DESCRIPTOR_RANGE_0013 * pDescriptorRange = pRootParameter->DescriptorTable.pDescriptorRanges + j;

                m_numDescriptorsUsed += pDescriptorRange->NumDescriptors;
            }
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_CBV:
        case D3D12DDI_ROOT_PARAMETER_TYPE_SRV:
        case D3D12DDI_ROOT_PARAMETER_TYPE_UAV:
            m_numRootViews++;
            break;
        }
    }
}

UINT CosUmd12RootSignature::GetHwRootSignatureSize(
    UINT * pNumPatchLocation)
{
    //
    // Use GpuHwWriteQWord to patch the Descriptor
    //
    // Each requires 2 patch location entries:
    // 1. The 1st one patches GpuHwWriteQWord::m_gpuAddress so that it points
    //    to the Descriptor's GpuHWDescriptor::m_resourceGpuAddress field
    // 2. The 2nd one patches GpuHwWriteQWord::m_data so that it contains
    //    the resource's GPU address
    //
    // After execution of the GpuHwWriteQWord command, the Descriptor's
    // m_resourceGpuAddress has the resource's GPU addres
    //

    *pNumPatchLocation = m_numRootViews + m_numDescriptorsUsed*2;

    UINT commandSize = FIELD_OFFSET(GpuHWRootSignature2LSet, m_rootValues) +
                       m_sizeOfRootValues +
                       m_numDescriptorsUsed*sizeof(GpuHwQwordWrite);

    if (m_numDescriptorsUsed)
    {
        //
        // 1 additional patch location for GpuHwDescriptorHeapSet
        //

        (*pNumPatchLocation) += 1;
        commandSize += sizeof(GpuHwDescriptorHeapSet);
    }

    return commandSize;

}

void CosUmd12RootSignature::WriteHWRootSignature(
    BYTE * pRootValues,
    CosUmd12DescriptorHeap * pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
    CosUmd12CommandBuffer * pCurCommandBuffer,
    BYTE * pCommandBuf,
    UINT curCommandOffset,
    D3DDDI_PATCHLOCATIONLIST * pPatchLocations)
{
    //
    // Write GpuHwDescriptorHeapSet command
    //

    const CosUmd12DescriptorHeap * pDescriptorHeap = pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
    GpuHwDescriptorHeapSet * pHwDescriptorHeapSet = nullptr;
    GpuHWRootSignature2LSet * pHwRootSignatureSet = nullptr;

    if (m_numDescriptorsUsed)
    {
         pHwDescriptorHeapSet = (GpuHwDescriptorHeapSet *)pCommandBuf;

        pHwDescriptorHeapSet->m_commandId = DescriptorHeapSet;

        pCurCommandBuffer->RecordGpuAddressReference(
            pDescriptorHeap->GetGpuAddress(),
            curCommandOffset + (UINT)(((BYTE *)(&pHwDescriptorHeapSet->m_descriptorHeapGpuAddress)) - pCommandBuf),
            pPatchLocations);

        pHwRootSignatureSet = (GpuHWRootSignature2LSet *)(pHwDescriptorHeapSet + 1);
    }
    else
    {
        pHwRootSignatureSet = (GpuHWRootSignature2LSet *)pCommandBuf;
    }

    //
    // Write GpuHWRootSignature2LSet command
    //

    pHwRootSignatureSet->m_commandId = RootSignature2LevelSet;
    pHwRootSignatureSet->m_commandSize = FIELD_OFFSET(GpuHWRootSignature2LSet, m_rootValues) + m_sizeOfRootValues;

    //
    // Copy root value into the command buffer
    //

    memcpy(pHwRootSignatureSet->m_rootValues, pRootValues, m_sizeOfRootValues);

    //
    // Set up Patch Location Entry for all the Root Views
    //

    const D3D12DDI_ROOT_PARAMETER_0013 * pRootParameter = m_rootSignature.pRootParameters;

    for (UINT i = 0; i < m_rootSignature.NumParameters; i++, pRootParameter++)
    {
        switch (pRootParameter->ParameterType)
        {
        case D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
        case D3D12DDI_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_CBV:
        case D3D12DDI_ROOT_PARAMETER_TYPE_SRV:
        case D3D12DDI_ROOT_PARAMETER_TYPE_UAV:
            {
                UINT rootViewOffset = m_pRootValueOffsets[i];

                D3D12DDI_GPU_VIRTUAL_ADDRESS rootViewGpuAddress = *(D3D12DDI_GPU_VIRTUAL_ADDRESS *)(pRootValues + rootViewOffset);

                pCurCommandBuffer->RecordGpuAddressReference(
                    rootViewGpuAddress,
                    curCommandOffset + (UINT)(pHwRootSignatureSet->m_rootValues + rootViewOffset - pCommandBuf),
                    pPatchLocations);
            }
            break;
        }
    }

    //
    // Patch Descriptors using GpuHwQwordWrite
    //

    GpuHwQwordWrite * pGpuHwQwordWrite = (GpuHwQwordWrite *)((BYTE *)(pHwRootSignatureSet) + pHwRootSignatureSet->m_commandSize);

    pRootParameter = m_rootSignature.pRootParameters;

    for (UINT i = 0; i < m_rootSignature.NumParameters; i++, pRootParameter++)
    {
        if (D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE != pRootParameter->ParameterType)
        {
            continue;
        }

        UINT offsetForTable = *(UINT *)(pRootValues + m_pRootValueOffsets[i]);
        UINT offsetForTableInDescriptors = offsetForTable/sizeof(CosUmd12Descriptor);
        UINT offsetForHwTable = sizeof(GpuHWDescriptor)*offsetForTableInDescriptors;

        //
        // Fix up the HW descriptor table offset in root values
        //

        *(UINT *)(pHwRootSignatureSet->m_rootValues + m_pRootValueOffsets[i]) = offsetForHwTable;

        for (UINT j = 0; j < pRootParameter->DescriptorTable.NumDescriptorRanges; j++)
        {
            const D3D12DDI_DESCRIPTOR_RANGE_0013 * pDescriptorRange = pRootParameter->DescriptorTable.pDescriptorRanges + j;
            D3D12DDI_GPU_VIRTUAL_ADDRESS resourceAddressField = pDescriptorHeap->GetGpuAddress() + 
                                                                offsetForHwTable +
                                                                pDescriptorRange->OffsetInDescriptorsFromTableStart*sizeof(GpuHWDescriptor) +
                                                                FIELD_OFFSET(GpuHWDescriptor, m_resourceGpuAddress);
            CosUmd12Descriptor * pDescriptor = pDescriptorHeap->GetCpuAddress() +
                                               offsetForTableInDescriptors +
                                               pDescriptorRange->OffsetInDescriptorsFromTableStart;
            GpuHWDescriptor * pHwDescriptor = pDescriptorHeap->m_pHwDescriptors +
                                              offsetForTableInDescriptors +
                                              pDescriptorRange->OffsetInDescriptorsFromTableStart;

            for (UINT k = 0; k < pDescriptorRange->NumDescriptors; k++)
            {
                //
                // Set up HW descriptor from SW
                //

                pHwDescriptor->m_type = pDescriptor->m_type;

                switch (pDescriptorRange->RangeType)
                {
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_UAV:
                    pHwDescriptor->m_uav.m_format = pDescriptor->m_uav.Format;
                    ASSERT(D3D12DDI_RD_BUFFER == pDescriptor->m_uav.ResourceDimension);
                    pHwDescriptor->m_uav.m_resourceDimension = pDescriptor->m_uav.ResourceDimension;
                    pHwDescriptor->m_uav.m_buffer = pDescriptor->m_uav.Buffer;
                    break;
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_CBV:
                    pHwDescriptor->m_cbv.m_sizeInBytes = pDescriptor->m_cbv.SizeInBytes;
                    pHwDescriptor->m_cbv.m_padding = pDescriptor->m_cbv.Padding;
                    break;
                default:
                    ASSERT(false);
                    break;
                }

                pGpuHwQwordWrite->m_commandId = QwordWrite;

                pCurCommandBuffer->RecordGpuAddressReference(
                    resourceAddressField,
                    curCommandOffset + (UINT)(((PBYTE)(&pGpuHwQwordWrite->m_gpuAddress)) - pCommandBuf),
                    pPatchLocations);

                pDescriptor->WriteHWDescriptor(
                    pCurCommandBuffer,
                    curCommandOffset + (UINT)(((PBYTE)(&pGpuHwQwordWrite->m_data)) - pCommandBuf),
                    pPatchLocations);

                pGpuHwQwordWrite++;
                resourceAddressField += sizeof(GpuHWDescriptor);
                pDescriptor++;
                pHwDescriptor++;
            }
        }
    }
}

#endif  // COS_RS_2LEVEL_SUPPORT

