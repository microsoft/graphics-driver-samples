#pragma once

#include "CosUmd12.h"

class CosUmd12Device;

class CosUmd12BlendState
{
public:
    explicit CosUmd12BlendState(CosUmd12Device* pDevice, const D3D12DDI_BLEND_DESC_0010* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
    }

    ~CosUmd12BlendState()
    {
    }

    static int CalculateSize(const D3D12DDI_BLEND_DESC_0010 * pArgs)
    {
        return sizeof(CosUmd12BlendState);
    }

    static CosUmd12BlendState* CastFrom(D3D12DDI_HBLENDSTATE);
    D3D12DDI_HBLENDSTATE CastTo() const;

private:

    CosUmd12Device * m_pDevice;
    D3D12DDI_BLEND_DESC_0010 m_args;

};

inline CosUmd12BlendState* CosUmd12BlendState::CastFrom(D3D12DDI_HBLENDSTATE hRootSignature)
{
    return static_cast< CosUmd12BlendState* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HBLENDSTATE CosUmd12BlendState::CastTo() const
{
    // TODO: Why does MAKE_D3D10DDI_HBLENDSTATE not exist?
    return MAKE_D3D10DDI_HBLENDSTATE(const_cast< CosUmd12BlendState* >(this));
}


