////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Root Signature implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

#if !(COS_GPUVA_SUPPORT || COS_RS_2LEVEL_SUPPORT)

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
    memset(&m_hwRootSignature, 0, sizeof(m_hwRootSignature));

    m_numRegistersToPatch = 0;

    const D3D12DDI_ROOT_PARAMETER_0013 * pRootParameter = m_rootSignature.pRootParameters;

    for (UINT i = 0; i < m_rootSignature.NumParameters; i++, pRootParameter++)
    {
        switch (pRootParameter->ParameterType)
        {
        case D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            for (UINT j = 0; j < pRootParameter->DescriptorTable.NumDescriptorRanges; j++)
            {
                const D3D12DDI_DESCRIPTOR_RANGE_0013 * pDescriptorRange = pRootParameter->DescriptorTable.pDescriptorRanges + j;

                switch (pDescriptorRange->RangeType)
                {
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_SRV:
                    if ((pDescriptorRange->BaseShaderRegister + pDescriptorRange->NumDescriptors) > m_hwRootSignature.m_numSrvRegisters)
                    {
                        m_hwRootSignature.m_numSrvRegisters = pDescriptorRange->BaseShaderRegister + pDescriptorRange->NumDescriptors;
                    }
                    m_numRegistersToPatch += pDescriptorRange->NumDescriptors;
                    break;
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_UAV:
                    //
                    // TODO : Support Counter resource for Append structured buffer
                    //
                    if ((pDescriptorRange->BaseShaderRegister + pDescriptorRange->NumDescriptors) > m_hwRootSignature.m_numUavRegisters)
                    {
                        m_hwRootSignature.m_numUavRegisters = pDescriptorRange->BaseShaderRegister + pDescriptorRange->NumDescriptors;
                    }
                    m_numRegistersToPatch += pDescriptorRange->NumDescriptors;
                    break;
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_CBV:
                    if ((pDescriptorRange->BaseShaderRegister + pDescriptorRange->NumDescriptors) > m_hwRootSignature.m_numCbvRegisters)
                    {
                        m_hwRootSignature.m_numCbvRegisters = pDescriptorRange->BaseShaderRegister + pDescriptorRange->NumDescriptors;
                    }
                    m_numRegistersToPatch += pDescriptorRange->NumDescriptors;
                    break;
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                    // TODO :
                    ASSERT(false);
                    break;
                }
            }
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            {
                UINT numConstantRegisters = (pRootParameter->Constants.Num32BitValues + 3) & (~3);

                if ((pRootParameter->Constants.ShaderRegister +  numConstantRegisters) > m_hwRootSignature.m_numCbvRegisters)
                {
                    m_hwRootSignature.m_numCbvRegisters = pRootParameter->Constants.ShaderRegister + numConstantRegisters;
                }
            }
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_CBV:
            if ((pRootParameter->Descriptor.ShaderRegister + 1) > m_hwRootSignature.m_numCbvRegisters)
            {
                m_hwRootSignature.m_numCbvRegisters = pRootParameter->Descriptor.ShaderRegister + 1;
            }
            m_numRegistersToPatch++;
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_SRV:
            if ((pRootParameter->Descriptor.ShaderRegister + 1) > m_hwRootSignature.m_numSrvRegisters)
            {
                m_hwRootSignature.m_numSrvRegisters = pRootParameter->Descriptor.ShaderRegister + 1;
            }
            m_numRegistersToPatch++;
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_UAV:
            if ((pRootParameter->Descriptor.ShaderRegister + 1) > m_hwRootSignature.m_numUavRegisters)
            {
                m_hwRootSignature.m_numUavRegisters = pRootParameter->Descriptor.ShaderRegister + 1;
            }
            m_numRegistersToPatch++;
            break;
        }
    }

    m_hwRootSignature.m_commandId = RootSignatureSet;
    m_hwRootSignature.m_commandSize = GetHwRootSignatureSize(nullptr);
}

UINT CosUmd12RootSignature::GetHwRootSignatureSize(
    UINT * pNnumRegistersToPatch)
{
	if (pNnumRegistersToPatch)
		*pNnumRegistersToPatch = m_numRegistersToPatch;

    return (sizeof(m_hwRootSignature) + 
            sizeof(GpuHWConstantDescriptor)*m_hwRootSignature.m_numCbvRegisters +
            sizeof(GpuHWDescriptor)*(m_hwRootSignature.m_numSrvRegisters + m_hwRootSignature.m_numUavRegisters));
}

void CosUmd12RootSignature::WriteHWRootDescriptor(
    CosUmd12CommandBuffer * pCurCommandBuffer,
    D3D12DDI_GPU_VIRTUAL_ADDRESS resourceGpuVA,
    UINT hwDescriptorOffset,
    D3DDDI_PATCHLOCATIONLIST * &pPatchLocations)
{
#if COS_GPUVA_SUPPORT
#else

    D3DKMT_HANDLE hAllocation = (D3DKMT_HANDLE)(resourceGpuVA >> 32);
    UINT allocationOffset = (UINT)(resourceGpuVA & 0xFFFFFFFF);

    UINT allocIndex = pCurCommandBuffer->UseAllocation(hAllocation, true);

    pCurCommandBuffer->SetPatchLocation(
                        pPatchLocations,
                        allocIndex,
                        hwDescriptorOffset,
                        0,
                        allocationOffset);

#endif
}

void CosUmd12RootSignature::WriteHWRootSignature(
    BYTE * pRootValues,
    CosUmd12DescriptorHeap * pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
    CosUmd12CommandBuffer * pCurCommandBuffer,
    BYTE * pCommandBuf,
    UINT curCommandOffset,
    D3DDDI_PATCHLOCATIONLIST * pPatchLocations)
{
    GpuHWRootSignatureSet * pHwRootSignatureSet = (GpuHWRootSignatureSet *)pCommandBuf;

    *pHwRootSignatureSet = m_hwRootSignature;

    memset(pHwRootSignatureSet + 1, 0, pHwRootSignatureSet->m_commandSize - sizeof(GpuHWRootSignatureSet));

    UINT hwCbvTableOffset, hwSrvTableOffset, hwUavTableOffset;
    UINT hwViewTableOffset, hwDescriptorSize;

    hwCbvTableOffset = curCommandOffset + sizeof(GpuHWRootSignatureSet);
    hwSrvTableOffset = hwCbvTableOffset + sizeof(GpuHWConstantDescriptor)*m_hwRootSignature.m_numCbvRegisters;
    hwUavTableOffset = hwSrvTableOffset + sizeof(GpuHWDescriptor)*m_hwRootSignature.m_numSrvRegisters;

    const D3D12DDI_ROOT_PARAMETER_0013 * pRootParameter = m_rootSignature.pRootParameters;

    for (UINT i = 0; i < m_rootSignature.NumParameters; i++, pRootParameter++)
    {
        switch (pRootParameter->ParameterType)
        {
        case D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            for (UINT j = 0; j < pRootParameter->DescriptorTable.NumDescriptorRanges; j++)
            {
                const D3D12DDI_DESCRIPTOR_RANGE_0013 * pDescriptorRange = pRootParameter->DescriptorTable.pDescriptorRanges + j;
                const CosUmd12DescriptorHeap * pDescriptorHeap = NULL;

                switch (pDescriptorRange->RangeType)
                {
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_SRV:
                    hwViewTableOffset = hwSrvTableOffset;
                    hwDescriptorSize = sizeof(GpuHWDescriptor);
                    pDescriptorHeap = pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
                    break;
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_UAV:
                    //
                    // TODO : Support Counter resource for Append structured buffer
                    //
                    hwViewTableOffset = hwUavTableOffset;
                    hwDescriptorSize = sizeof(GpuHWDescriptor);
                    pDescriptorHeap = pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
                    break;
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_CBV:
                    hwViewTableOffset = hwCbvTableOffset;
                    hwDescriptorSize = sizeof(GpuHWConstantDescriptor);
                    pDescriptorHeap = pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
                    break;
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                    // TODO :
                    ASSERT(false);
                    break;
                }

                switch (pDescriptorRange->RangeType)
                {
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_SRV:
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_UAV:
                case D3D12DDI_DESCRIPTOR_RANGE_TYPE_CBV:
                    {
                        UINT tableOffset = *((UINT *)(pRootValues + m_pRootValueOffsets[i]));

                        UINT descriptorIndex = (tableOffset / sizeof(CosUmd12Descriptor)) + pDescriptorRange->OffsetInDescriptorsFromTableStart;

                        for (UINT k = 0; k < pDescriptorRange->NumDescriptors; k++, descriptorIndex++)
                        {
                            const CosUmd12Descriptor * pDescriptor = pDescriptorHeap->m_pDescriptors + descriptorIndex;

                            pDescriptor->WriteHWDescriptor(
                                            pCurCommandBuffer,
                                            hwViewTableOffset + (pDescriptorRange->BaseShaderRegister + k) * hwDescriptorSize,
                                            pPatchLocations);
                        }
                    }
                    break;
                }
            }
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            {
                FLOAT * pRootConstant = (FLOAT *)(pCommandBuf +
                                                  (hwCbvTableOffset - curCommandOffset) + 
                                                  pRootParameter->Constants.ShaderRegister * sizeof(GpuHWConstantDescriptor));

                memcpy(
                    pRootConstant,
                    pRootValues + m_pRootValueOffsets[i],
                    pRootParameter->Constants.Num32BitValues * sizeof(FLOAT));
            }
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_CBV:
            hwViewTableOffset = hwCbvTableOffset;
            hwDescriptorSize = sizeof(GpuHWConstantDescriptor);
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_SRV:
            hwViewTableOffset = hwSrvTableOffset;
            hwDescriptorSize = sizeof(GpuHWDescriptor);
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_UAV:
            hwViewTableOffset = hwUavTableOffset;
            hwDescriptorSize = sizeof(GpuHWDescriptor);
            break;
        }

        switch (pRootParameter->ParameterType)
        {
        case D3D12DDI_ROOT_PARAMETER_TYPE_CBV:
        case D3D12DDI_ROOT_PARAMETER_TYPE_SRV:
        case D3D12DDI_ROOT_PARAMETER_TYPE_UAV:
            {
                D3D12DDI_GPU_VIRTUAL_ADDRESS resourceGpuVA = *((D3D12DDI_GPU_VIRTUAL_ADDRESS *)(pRootValues + m_pRootValueOffsets[i]));

                WriteHWRootDescriptor(
                    pCurCommandBuffer,
                    resourceGpuVA,
                    hwViewTableOffset + pRootParameter->Descriptor.ShaderRegister * hwDescriptorSize,
                    pPatchLocations);
            }
            break;
        }
    }
}

#endif  // !(COS_GPUVA_SUPPORT || COS_RS_2LEVEL_SUPPORT)

