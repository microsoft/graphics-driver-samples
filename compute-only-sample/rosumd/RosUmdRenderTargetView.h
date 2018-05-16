#pragma once

class RosUmdRenderTargetView
{
friend class RosUmdDevice;
friend class RosCompiler;

public:

    RosUmdRenderTargetView(const D3D10DDIARG_CREATERENDERTARGETVIEW * pCreate, D3D10DDI_HRTRENDERTARGETVIEW & hRT) :
        m_create(*pCreate), m_hRTRenderTargetView(hRT)
    {
        // do nothing
    }

    ~RosUmdRenderTargetView()
    {
        // do nothing
    }

    static RosUmdRenderTargetView* CastFrom(D3D10DDI_HRENDERTARGETVIEW);
    D3D10DDI_HRENDERTARGETVIEW CastTo() const;

private:

    D3D10DDIARG_CREATERENDERTARGETVIEW m_create;
    D3D10DDI_HRTRENDERTARGETVIEW m_hRTRenderTargetView;
};

inline RosUmdRenderTargetView* RosUmdRenderTargetView::CastFrom(D3D10DDI_HRENDERTARGETVIEW hRasterizerState)
{
    return static_cast< RosUmdRenderTargetView* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HRENDERTARGETVIEW RosUmdRenderTargetView::CastTo() const
{
    return MAKE_D3D10DDI_HRENDERTARGETVIEW(const_cast< RosUmdRenderTargetView* >(this));
}
