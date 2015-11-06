#pragma once

#include "d3dumddi_.h"

class RosUmdDepthStencilState
{
    friend class RosUmdDevice;

public:

    RosUmdDepthStencilState(const D3D10_DDI_DEPTH_STENCIL_DESC * desc, D3D10DDI_HRTDEPTHSTENCILSTATE & hRT) :
        m_desc(*desc), m_hRTDepthStencilState(hRT)
    {
        // do nothing
    }

    static RosUmdDepthStencilState* CastFrom(D3D10DDI_HDEPTHSTENCILSTATE);
    D3D10DDI_HDEPTHSTENCILSTATE CastTo() const;

private:

    D3D10_DDI_DEPTH_STENCIL_DESC m_desc;
    D3D10DDI_HRTDEPTHSTENCILSTATE m_hRTDepthStencilState;
};

inline RosUmdDepthStencilState* RosUmdDepthStencilState::CastFrom(D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState)
{
    return static_cast< RosUmdDepthStencilState* >(hDepthStencilState.pDrvPrivate);
}

inline D3D10DDI_HDEPTHSTENCILSTATE RosUmdDepthStencilState::CastTo() const
{
    return MAKE_D3D10DDI_HDEPTHSTENCILSTATE(const_cast< RosUmdDepthStencilState* >(this));
}
