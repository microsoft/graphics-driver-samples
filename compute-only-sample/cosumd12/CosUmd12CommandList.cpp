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

void
CosUmd12CommandList::SetComputeRootUnorderedAccessView(
    UINT RootParameterIndex,
    D3D12DDI_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    CosUmd12RootSignature * pRootSignature = CosUmd12RootSignature::CastFrom(m_pPipelineState->m_args.hRootSignature);
    const D3D12DDI_ROOT_PARAMETER_0013 * pRootParameter = pRootSignature->m_rootSignature.pRootParameters + RootParameterIndex;

    m_rootDescriptorTableUav[pRootParameter->Descriptor.ShaderRegister] = BufferLocation;
}

void
CosUmd12CommandList::Dispatch(
    UINT ThreadGroupCountX,
    UINT ThreadGroupCountY,
    UINT ThreadGroupCountZ)
{
    //
    // Write the Root Signature into the command list
    //
    CosUmd12RootSignature * pRootSignature = CosUmd12RootSignature::CastFrom(m_pPipelineState->m_args.hRootSignature);
    GpuHWRootSignatureSet hwRootSignature;

    memset(&hwRootSignature, 0, sizeof(hwRootSignature));

    const D3D12DDI_ROOT_PARAMETER_0013 * pRootParameter = pRootSignature->m_rootSignature.pRootParameters;

    for (UINT i = 0; i < pRootSignature->m_rootSignature.NumParameters; i++, pRootParameter++)
    {
        switch (pRootParameter->ParameterType)
        {
        case D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            // TODO : 
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            // TODO :
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_CBV:
            // TODO : 
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_SRV:
            // TODO : 
            break;
        case D3D12DDI_ROOT_PARAMETER_TYPE_UAV:
            if ((pRootParameter->Descriptor.ShaderRegister + 1) > hwRootSignature.m_numRootDescriptorUav)
            {
                hwRootSignature.m_numRootDescriptorUav = pRootParameter->Descriptor.ShaderRegister + 1;
            }
            break;
        }
    }

    UINT numRootDescriptors = hwRootSignature.m_numRootDescriptorCbv +
                              hwRootSignature.m_numRootDescriptorSrv +
                              hwRootSignature.m_numRootDescriptorUav;

    UINT commandSize = sizeof(GpuHWRootSignatureSet) +
        sizeof(FLOAT)*hwRootSignature.m_numRootConstants +
        sizeof(PHYSICAL_ADDRESS)*numRootDescriptors;

    GpuHWRootSignatureSet * pRootSignatureSet = NULL;
    UINT curCommandOffset;
    D3DDDI_PATCHLOCATIONLIST * pPatchLocationList;

    ReserveCommandBufferSpace(
        false,                          // HW command
        commandSize,
        (BYTE **)&pRootSignatureSet,
        numRootDescriptors,
        numRootDescriptors,
        &curCommandOffset,
        &pPatchLocationList);
    if (NULL == pRootSignatureSet)
    {
        return;
    }

    *pRootSignatureSet = hwRootSignature;
    pRootSignatureSet->m_commandId = RootSignatureSet;
    pRootSignatureSet->m_commandSize = commandSize;

    memset(pRootSignatureSet + 1, 0, commandSize - sizeof(GpuHWRootSignatureSet));

    curCommandOffset += sizeof(GpuHWRootSignatureSet);
    UINT numPatchLocations = 0;

    for (UINT i = 0; i < hwRootSignature.m_numRootDescriptorUav; i++, curCommandOffset += sizeof(PHYSICAL_ADDRESS))
    {
        if (0 == m_rootDescriptorTableUav[i])
        {
            continue;
        }

        D3DKMT_HANDLE hUvaAllocation = (D3DKMT_HANDLE)(m_rootDescriptorTableUav[i] >> 32);
        UINT uvaAllocationOffset = (UINT)(m_rootDescriptorTableUav[i] & 0xFFFFFFFF);

        UINT allocIndex = m_pCurCommandBuffer->UseAllocation(hUvaAllocation, true);

        m_pCurCommandBuffer->SetPatchLocation(
            pPatchLocationList,
            allocIndex,
            curCommandOffset,
            uvaAllocationOffset);

        numPatchLocations++;
    }

    m_pCurCommandBuffer->CommitCommandBufferSpace(commandSize, numPatchLocations);

    //
    // Write Dispatch command into the Command List
    //

    CosUmd12Shader * pComputeShader = CosUmd12Shader::CastFrom(m_pPipelineState->m_args.hComputeShader);
    commandSize = sizeof(GpuHwComputeShaderDisptch) + pComputeShader->m_args.pShaderCode[1]*sizeof(UINT);

    GpuHwComputeShaderDisptch * pCSDispath;

    ReserveCommandBufferSpace(
        false,                          // HW command
        commandSize,
        (BYTE **)&pCSDispath);
    if (NULL == pCSDispath)
    {
        return;
    }

    pCSDispath->m_commandId = ComputeShaderDispatch;
    pCSDispath->m_commandSize = commandSize;

    //
    // TODO: Retrieve num threads per group from shader
    //
    pCSDispath->m_numThreadPerGroup = 4;
    pCSDispath->m_threadGroupCountX = ThreadGroupCountX;
    pCSDispath->m_threadGroupCountY = ThreadGroupCountY;
    pCSDispath->m_threadGroupCountZ = ThreadGroupCountZ;

    memcpy(pCSDispath->m_ShaderHash, pComputeShader->m_args.ShaderCodeHash.Hash, sizeof(pCSDispath->m_ShaderHash));
    memcpy(pCSDispath + 1, pComputeShader->m_args.pShaderCode, pComputeShader->m_args.pShaderCode[1] * sizeof(UINT));

    m_pCurCommandBuffer->CommitCommandBufferSpace(commandSize);
}

