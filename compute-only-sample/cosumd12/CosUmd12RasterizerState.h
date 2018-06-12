#pragma once

#include "CosUmd12.h"

class CosUmd12Device;

class CosUmd12RasterizerState
{
public:
    explicit CosUmd12RasterizerState(CosUmd12Device* pDevice, const D3D12DDI_RASTERIZER_DESC_0010* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
    }

    ~CosUmd12RasterizerState()
    {
    }

    static int CalculateSize(const D3D12DDI_RASTERIZER_DESC_0010 * pArgs)
    {
        return sizeof(CosUmd12RasterizerState);
    }

    static CosUmd12RasterizerState* CastFrom(D3D12DDI_HRASTERIZERSTATE);
    D3D12DDI_HRASTERIZERSTATE CastTo() const;

private:

    CosUmd12Device * m_pDevice;
    D3D12DDI_RASTERIZER_DESC_0010 m_args;

};

inline CosUmd12RasterizerState* CosUmd12RasterizerState::CastFrom(D3D12DDI_HRASTERIZERSTATE hRootSignature)
{
    return static_cast< CosUmd12RasterizerState* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HRASTERIZERSTATE CosUmd12RasterizerState::CastTo() const
{
    // TODO: Why does MAKE_D3D10DDI_HDEPTHSTENCILSTATE not exist?
    return MAKE_D3D10DDI_HRASTERIZERSTATE(const_cast< CosUmd12RasterizerState* >(this));
}


