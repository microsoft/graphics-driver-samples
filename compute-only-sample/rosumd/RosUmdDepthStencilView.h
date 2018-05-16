#pragma once

class RosUmdDepthStencilView
{
friend class RosUmdDevice;

public:

    RosUmdDepthStencilView(const D3D11DDIARG_CREATEDEPTHSTENCILVIEW * pCreate, D3D10DDI_HRTDEPTHSTENCILVIEW & hRT) :
        m_create(*pCreate), m_hRTDepthStencilView(hRT)
    {
        // do nothing
    }

    ~RosUmdDepthStencilView()
    {
        // do nothing
    }

    static RosUmdDepthStencilView* CastFrom(D3D10DDI_HDEPTHSTENCILVIEW);
    D3D10DDI_HDEPTHSTENCILVIEW CastTo() const;

private:

    D3D11DDIARG_CREATEDEPTHSTENCILVIEW m_create;
    D3D10DDI_HRTDEPTHSTENCILVIEW m_hRTDepthStencilView;
};

inline RosUmdDepthStencilView* RosUmdDepthStencilView::CastFrom(D3D10DDI_HDEPTHSTENCILVIEW hRasterizerState)
{
    return static_cast< RosUmdDepthStencilView* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HDEPTHSTENCILVIEW RosUmdDepthStencilView::CastTo() const
{
    return MAKE_D3D10DDI_HDEPTHSTENCILVIEW(const_cast< RosUmdDepthStencilView* >(this));
}
