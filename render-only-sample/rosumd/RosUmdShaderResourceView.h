#pragma once

class RosUmdShaderResourceView
{
friend class RosUmdDevice;
friend class RosCompiler;

public:

    RosUmdShaderResourceView(const D3D11DDIARG_CREATESHADERRESOURCEVIEW * pCreate, D3D10DDI_HRTSHADERRESOURCEVIEW & hRT) :
        m_create(*pCreate), m_hRTShaderResourceView(hRT)
    {
        // do nothing
    }

    ~RosUmdShaderResourceView()
    {
        // do nothing
    }

    static RosUmdShaderResourceView* CastFrom(D3D10DDI_HSHADERRESOURCEVIEW);
    D3D10DDI_HSHADERRESOURCEVIEW CastTo() const;

private:

    D3D11DDIARG_CREATESHADERRESOURCEVIEW m_create;
    D3D10DDI_HRTSHADERRESOURCEVIEW m_hRTShaderResourceView;
};

inline RosUmdShaderResourceView* RosUmdShaderResourceView::CastFrom(D3D10DDI_HSHADERRESOURCEVIEW hRasterizerState)
{
    return static_cast< RosUmdShaderResourceView* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HSHADERRESOURCEVIEW RosUmdShaderResourceView::CastTo() const
{
    return MAKE_D3D10DDI_HSHADERRESOURCEVIEW(const_cast< RosUmdShaderResourceView* >(this));
}
