#pragma once

class CosUmdRenderTargetView
{
friend class CosUmdDevice;
friend class CosCompiler;

public:

    CosUmdRenderTargetView(const D3D10DDIARG_CREATERENDERTARGETVIEW * pCreate, D3D10DDI_HRTRENDERTARGETVIEW & hRT) :
        m_create(*pCreate), m_hRTRenderTargetView(hRT)
    {
        // do nothing
    }

    ~CosUmdRenderTargetView()
    {
        // do nothing
    }

    static CosUmdRenderTargetView* CastFrom(D3D10DDI_HRENDERTARGETVIEW);
    D3D10DDI_HRENDERTARGETVIEW CastTo() const;

private:

    D3D10DDIARG_CREATERENDERTARGETVIEW m_create;
    D3D10DDI_HRTRENDERTARGETVIEW m_hRTRenderTargetView;
};

inline CosUmdRenderTargetView* CosUmdRenderTargetView::CastFrom(D3D10DDI_HRENDERTARGETVIEW hRasterizerState)
{
    return static_cast< CosUmdRenderTargetView* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HRENDERTARGETVIEW CosUmdRenderTargetView::CastTo() const
{
    return MAKE_D3D10DDI_HRENDERTARGETVIEW(const_cast< CosUmdRenderTargetView* >(this));
}
