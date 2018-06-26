#pragma once

#pragma once

#pragma once

#pragma once

#include "CosUmd12.h"

class CosUmd12Device;

class CosUmd12DepthStencilState
{
public:
    explicit CosUmd12DepthStencilState(CosUmd12Device* pDevice, const D3D12DDI_DEPTH_STENCIL_DESC_0025* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
    }

    ~CosUmd12DepthStencilState()
    {
    }

    static int CalculateSize(const D3D12DDI_DEPTH_STENCIL_DESC_0025 * pArgs)
    {
        return sizeof(CosUmd12DepthStencilState);
    }

    static CosUmd12DepthStencilState* CastFrom(D3D12DDI_HDEPTHSTENCILSTATE);
    D3D12DDI_HDEPTHSTENCILSTATE CastTo() const;

private:

    CosUmd12Device * m_pDevice;
    D3D12DDI_DEPTH_STENCIL_DESC_0025 m_args;

};

inline CosUmd12DepthStencilState* CosUmd12DepthStencilState::CastFrom(D3D12DDI_HDEPTHSTENCILSTATE hRootSignature)
{
    return static_cast< CosUmd12DepthStencilState* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HDEPTHSTENCILSTATE CosUmd12DepthStencilState::CastTo() const
{
    // TODO: Why does MAKE_D3D10DDI_HDEPTHSTENCILSTATE not exist?
    return MAKE_D3D10DDI_HDEPTHSTENCILSTATE(const_cast< CosUmd12DepthStencilState* >(this));
}


