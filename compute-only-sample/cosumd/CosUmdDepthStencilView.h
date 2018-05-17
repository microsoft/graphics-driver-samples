#pragma once

class CosUmdDepthStencilView
{
friend class CosUmdDevice;

public:

    CosUmdDepthStencilView(const D3D11DDIARG_CREATEDEPTHSTENCILVIEW * pCreate, D3D10DDI_HRTDEPTHSTENCILVIEW & hRT) :
        m_create(*pCreate), m_hRTDepthStencilView(hRT)
    {
        // do nothing
    }

    ~CosUmdDepthStencilView()
    {
        // do nothing
    }

    static CosUmdDepthStencilView* CastFrom(D3D10DDI_HDEPTHSTENCILVIEW);
    D3D10DDI_HDEPTHSTENCILVIEW CastTo() const;

private:

    D3D11DDIARG_CREATEDEPTHSTENCILVIEW m_create;
    D3D10DDI_HRTDEPTHSTENCILVIEW m_hRTDepthStencilView;
};

inline CosUmdDepthStencilView* CosUmdDepthStencilView::CastFrom(D3D10DDI_HDEPTHSTENCILVIEW hRasterizerState)
{
    return static_cast< CosUmdDepthStencilView* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HDEPTHSTENCILVIEW CosUmdDepthStencilView::CastTo() const
{
    return MAKE_D3D10DDI_HDEPTHSTENCILVIEW(const_cast< CosUmdDepthStencilView* >(this));
}
