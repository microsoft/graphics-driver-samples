#pragma once

#pragma once

#include "CosUmd12.h"

class CosUmd12Fence
{
public:
    explicit CosUmd12Fence(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_FENCE* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
    }

    ~CosUmd12Fence()
    {
    }

    static int CalculateSize(const D3D12DDIARG_CREATE_FENCE * pArgs)
    {
        return sizeof(CosUmd12Fence);
    }

    static CosUmd12Fence* CastFrom(D3D12DDI_HFENCE);
    D3D12DDI_HFENCE CastTo() const;

private:

    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATE_FENCE m_args;

};

inline CosUmd12Fence* CosUmd12Fence::CastFrom(D3D12DDI_HFENCE hRootSignature)
{
    return static_cast< CosUmd12Fence* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HFENCE CosUmd12Fence::CastTo() const
{
    return MAKE_D3D12DDI_HFENCE(const_cast< CosUmd12Fence* >(this));
}
