#pragma once

#include "CosUmd12.h"

class CosUmd12Device;
class CosUmd12CommandList;
class CosUmd12CommandBuffer;
class CosUmd12DescriptorHeap;

const UINT SIZE_ROOT_SIGNATURE = 64*sizeof(DWORD);

class CosUmd12RootSignature
{
public:
    explicit CosUmd12RootSignature(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_ROOT_SIGNATURE_0013* pArgs)
    {
        m_pDevice = pDevice;
        m_version = pArgs->Version;
        m_nodeMask = pArgs->NodeMask;
        m_rootSignature = *pArgs->pRootSignature_1_1;

        char * storage = (char *)this + sizeof(*this);
        UINT storageSize = CalculateSize(pArgs) - sizeof(*this);
        UINT valueOffset = 0;

        m_pRootValueOffsets = (UINT *)storage;
        UINT size = m_rootSignature.NumParameters * sizeof(UINT);

        storageSize -= size;
        storage += size;

        size = sizeof(D3D12DDI_ROOT_PARAMETER_0013) * m_rootSignature.NumParameters;
        ASSERT(storageSize >= size);

        D3D12DDI_ROOT_PARAMETER_0013 * dstRootParameters = (D3D12DDI_ROOT_PARAMETER_0013 *) storage;
        storageSize -= size;
        storage += size;

        memcpy(dstRootParameters, m_rootSignature.pRootParameters, size);
        m_rootSignature.pRootParameters = dstRootParameters;

        D3D12DDI_ROOT_PARAMETER_0013 * pRootParameter = dstRootParameters;
        for (UINT i = 0; i < m_rootSignature.NumParameters; i++, pRootParameter++)
        {
            m_pRootValueOffsets[i] = valueOffset;

            switch (pRootParameter->ParameterType)
            {
            case D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                {
                    D3D12DDI_ROOT_DESCRIPTOR_TABLE_0013 * pDescriptorTable = &pRootParameter->DescriptorTable;
                    size = sizeof(D3D12DDI_DESCRIPTOR_RANGE_0013) * pDescriptorTable->NumDescriptorRanges;
                    ASSERT(storageSize >= size);

                    D3D12DDI_DESCRIPTOR_RANGE_0013 * dstDescriptorRanges = (D3D12DDI_DESCRIPTOR_RANGE_0013 *)storage;
                    storageSize -= size;
                    storage += size;

                    memcpy(dstDescriptorRanges, pDescriptorTable->pDescriptorRanges, size);
                    pDescriptorTable->pDescriptorRanges = dstDescriptorRanges;
                }
                //
                // Offset from the start of Descriptor Heap need to be stored
                //
                valueOffset += sizeof(UINT);
                break;
            case D3D12DDI_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
                valueOffset += pRootParameter->Constants.Num32BitValues * sizeof(FLOAT);
                break;
            case D3D12DDI_ROOT_PARAMETER_TYPE_CBV:
            case D3D12DDI_ROOT_PARAMETER_TYPE_SRV:
            case D3D12DDI_ROOT_PARAMETER_TYPE_UAV:
                valueOffset += sizeof(D3D12DDI_GPU_VIRTUAL_ADDRESS);
                break;
            }
        }

        ASSERT(storageSize == 0);
        ASSERT(m_rootSignature.NumStaticSamplers == 0);

        m_sizeOfRootValues = valueOffset;
    }

    ~CosUmd12RootSignature()
    {
    }

    static int CalculateSize(const D3D12DDIARG_CREATE_ROOT_SIGNATURE_0013 * pArgs)
    {
        const D3D12DDI_ROOT_SIGNATURE_0013 * pRootSignature = pArgs->pRootSignature_1_1;
        int size = sizeof(CosUmd12RootSignature) + sizeof(D3D12DDI_ROOT_PARAMETER_0013) * pRootSignature->NumParameters;
        const D3D12DDI_ROOT_PARAMETER_0013 * pRootParameter = pRootSignature->pRootParameters;

        for (UINT i = 0; i < pArgs->pRootSignature_1_1->NumParameters; i++, pRootParameter++)
        {
            if (pRootParameter->ParameterType == D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                const D3D12DDI_ROOT_DESCRIPTOR_TABLE_0013 * pDescriptorTable = &pRootParameter->DescriptorTable;
                size += sizeof(D3D12DDI_DESCRIPTOR_RANGE_0013) * pDescriptorTable->NumDescriptorRanges;
            }
        }

        //
        // Offsets to root parameter values
        //

        size += pRootSignature->NumParameters * sizeof(UINT);

        return size;
    }

    static CosUmd12RootSignature* CastFrom(D3D12DDI_HROOTSIGNATURE);
    D3D12DDI_HROOTSIGNATURE CastTo() const;

    void SetRootDescriptorTable(
        BYTE * pRootValues,
        CosUmd12DescriptorHeap * pDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
        UINT rootParameterIndex,
        D3D12DDI_GPU_DESCRIPTOR_HANDLE baseDescriptor);
        
   void SetRoot32BitConstants(
        BYTE * pRootValues,
        UINT rootParameterIndex,
        UINT num32BitValuesToSet,
        const void* pSrcData,
        UINT destOffsetIn32BitValues);

    void SetRootView(
        BYTE * pRootValues,
        UINT rootParameterIndex,
        D3D12DDI_GPU_VIRTUAL_ADDRESS bufferLocation);

    UINT GetHwRootSignatureSize();

    void WriteHWRootSignature(
        BYTE * pRootValues,
        CosUmd12DescriptorHeap * pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
        BYTE * pCommandBuf);

private:

    friend class CosUmd12CommandList;

    CosUmd12Device * m_pDevice;
    D3D12DDI_ROOT_SIGNATURE_VERSION m_version;
    D3D12DDI_ROOT_SIGNATURE_0013 m_rootSignature;
    UINT m_nodeMask;

    UINT * m_pRootValueOffsets;
    UINT m_sizeOfRootValues;

    void PrepareHWRootSignature();
};

inline CosUmd12RootSignature* CosUmd12RootSignature::CastFrom(D3D12DDI_HROOTSIGNATURE hRootSignature)
{
    return static_cast< CosUmd12RootSignature* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HROOTSIGNATURE CosUmd12RootSignature::CastTo() const
{
    return MAKE_D3D12DDI_HROOTSIGNATURE(const_cast< CosUmd12RootSignature* >(this));
}

