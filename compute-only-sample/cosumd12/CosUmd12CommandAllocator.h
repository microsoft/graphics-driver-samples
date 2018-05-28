#pragma once

#include "CosUmd12.h"

class CosUmd12CommandAllocator
{
public:
    explicit CosUmd12CommandAllocator(CosUmd12Device* pDevice, const D3D12DDIARG_CREATECOMMANDALLOCATOR* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
    }

    ~CosUmd12CommandAllocator()
    {
    }

    void Reset()
    {
        // do nothing
    }

    static int CalculateSize(const D3D12DDIARG_CREATECOMMANDALLOCATOR * pArgs)
    {
        return sizeof(CosUmd12CommandAllocator);
    }

    static CosUmd12CommandAllocator* CastFrom(D3D12DDI_HCOMMANDALLOCATOR);
    D3D12DDI_HCOMMANDALLOCATOR CastTo() const;

private:

    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATECOMMANDALLOCATOR m_args;

};

inline CosUmd12CommandAllocator* CosUmd12CommandAllocator::CastFrom(D3D12DDI_HCOMMANDALLOCATOR hRootSignature)
{
    return static_cast< CosUmd12CommandAllocator* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HCOMMANDALLOCATOR CosUmd12CommandAllocator::CastTo() const
{
    // TODO: Why does MAKE_D3D10DDI_HDEPTHSTENCILSTATE not exist?
    return MAKE_D3D12DDI_HCOMMANDALLOCATOR(const_cast< CosUmd12CommandAllocator* >(this));
}
