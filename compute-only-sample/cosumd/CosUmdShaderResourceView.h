#pragma once

class CosUmdShaderResourceView
{
friend class CosUmdDevice;
friend class CosCompiler;

public:

    CosUmdShaderResourceView(const D3D11DDIARG_CREATESHADERRESOURCEVIEW * pCreate, D3D10DDI_HRTSHADERRESOURCEVIEW & hRT) :
        m_create(*pCreate), m_hRTShaderResourceView(hRT)
    {
        // do nothing
    }

    ~CosUmdShaderResourceView()
    {
        // do nothing
    }

    static CosUmdShaderResourceView* CastFrom(D3D10DDI_HSHADERRESOURCEVIEW);
    D3D10DDI_HSHADERRESOURCEVIEW CastTo() const;

private:

    D3D11DDIARG_CREATESHADERRESOURCEVIEW m_create;
    D3D10DDI_HRTSHADERRESOURCEVIEW m_hRTShaderResourceView;
};

inline CosUmdShaderResourceView* CosUmdShaderResourceView::CastFrom(D3D10DDI_HSHADERRESOURCEVIEW hRasterizerState)
{
    return static_cast< CosUmdShaderResourceView* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HSHADERRESOURCEVIEW CosUmdShaderResourceView::CastTo() const
{
    return MAKE_D3D10DDI_HSHADERRESOURCEVIEW(const_cast< CosUmdShaderResourceView* >(this));
}
