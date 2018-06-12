#pragma once

#include "CosUmd12.h"

class CosUmd12Device;

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
        int storageSize = CalculateSize(pArgs) - sizeof(*this);

        size_t size = sizeof(D3D12DDI_ROOT_PARAMETER_0013) * m_rootSignature.NumParameters;
        ASSERT(storageSize >= size);

        D3D12DDI_ROOT_PARAMETER_0013 * dstRootParameters = (D3D12DDI_ROOT_PARAMETER_0013 *) storage;
        storageSize -= size;

        memcpy(dstRootParameters, m_rootSignature.pRootParameters, size);
        m_rootSignature.pRootParameters = dstRootParameters;

        D3D12DDI_ROOT_PARAMETER_0013 * pRootParameter = dstRootParameters;
        for (UINT i = 0; i < m_rootSignature.NumParameters; i++, pRootParameter++) {
            if (pRootParameter->ParameterType == D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
                D3D12DDI_ROOT_DESCRIPTOR_TABLE_0013 * pDescriptorTable = &pRootParameter->DescriptorTable;
                size = sizeof(D3D12DDI_DESCRIPTOR_RANGE_0013) * pDescriptorTable->NumDescriptorRanges;
                ASSERT(storageSize >= size);

                D3D12DDI_DESCRIPTOR_RANGE_0013 * dstDescriptorRanges = (D3D12DDI_DESCRIPTOR_RANGE_0013 *)storage;
                storageSize -= size;

                memcpy(dstDescriptorRanges, pDescriptorTable->pDescriptorRanges, size);
                pDescriptorTable->pDescriptorRanges = dstDescriptorRanges;
            }
        }

        ASSERT(storageSize == 0);
        ASSERT(m_rootSignature.NumStaticSamplers == 0);
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
            if (pRootParameter->ParameterType == D3D12DDI_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
                const D3D12DDI_ROOT_DESCRIPTOR_TABLE_0013 * pDescriptorTable = &pRootParameter->DescriptorTable;
                size += sizeof(D3D12DDI_DESCRIPTOR_RANGE_0013) * pDescriptorTable->NumDescriptorRanges;
            }
        return size;
    }

    static CosUmd12RootSignature* CastFrom(D3D12DDI_HROOTSIGNATURE);
    D3D12DDI_HROOTSIGNATURE CastTo() const;

private:

    CosUmd12Device * m_pDevice;
    D3D12DDI_ROOT_SIGNATURE_VERSION m_version;
    D3D12DDI_ROOT_SIGNATURE_0013 m_rootSignature;        
    UINT m_nodeMask;

};

inline CosUmd12RootSignature* CosUmd12RootSignature::CastFrom(D3D12DDI_HROOTSIGNATURE hRootSignature)
{
    return static_cast< CosUmd12RootSignature* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HROOTSIGNATURE CosUmd12RootSignature::CastTo() const
{
    return MAKE_D3D12DDI_HROOTSIGNATURE(const_cast< CosUmd12RootSignature* >(this));
}
