#pragma once

class CosUmdDepthStencilState
{
    friend class CosUmdDevice;

public:

    CosUmdDepthStencilState(const D3D10_DDI_DEPTH_STENCIL_DESC * desc, D3D10DDI_HRTDEPTHSTENCILSTATE & hRT) :
        m_desc(*desc), m_hRTDepthStencilState(hRT)
    {
        // do nothing
    }

    static CosUmdDepthStencilState* CastFrom(D3D10DDI_HDEPTHSTENCILSTATE);
    D3D10DDI_HDEPTHSTENCILSTATE CastTo() const;

    const D3D10_DDI_DEPTH_STENCIL_DESC* GetDesc()
    {
        return &m_desc;
    }

private:

    D3D10_DDI_DEPTH_STENCIL_DESC m_desc;
    D3D10DDI_HRTDEPTHSTENCILSTATE m_hRTDepthStencilState;
};

inline CosUmdDepthStencilState* CosUmdDepthStencilState::CastFrom(D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState)
{
    return static_cast< CosUmdDepthStencilState* >(hDepthStencilState.pDrvPrivate);
}

inline D3D10DDI_HDEPTHSTENCILSTATE CosUmdDepthStencilState::CastTo() const
{
    return MAKE_D3D10DDI_HDEPTHSTENCILSTATE(const_cast< CosUmdDepthStencilState* >(this));
}
