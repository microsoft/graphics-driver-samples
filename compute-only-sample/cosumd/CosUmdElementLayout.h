#pragma once

class CosUmdElementLayout
{
friend CosUmdDevice;

public:

    CosUmdElementLayout(const D3D10DDIARG_CREATEELEMENTLAYOUT * create, D3D10DDI_HRTELEMENTLAYOUT & hRT) :
        m_hRTElementLayout(hRT)
    {
        // TODO(bhouse) Perhaps we should use vector?

        m_numElements = create->NumElements;
        m_pElementDesc = new D3D10DDIARG_INPUT_ELEMENT_DESC[m_numElements];

        memcpy(m_pElementDesc, create->pVertexElements, sizeof(D3D10DDIARG_INPUT_ELEMENT_DESC) * m_numElements);
    }

    ~CosUmdElementLayout()
    {
        delete[] m_pElementDesc;
    }

    static CosUmdElementLayout* CastFrom(D3D10DDI_HELEMENTLAYOUT);
    D3D10DDI_HELEMENTLAYOUT CastTo() const;

private:

    UINT                                m_numElements;
    D3D10DDIARG_INPUT_ELEMENT_DESC *    m_pElementDesc;
    D3D10DDI_HRTELEMENTLAYOUT           m_hRTElementLayout;
};

inline CosUmdElementLayout* CosUmdElementLayout::CastFrom(D3D10DDI_HELEMENTLAYOUT hRasterizerState)
{
    return static_cast< CosUmdElementLayout* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HELEMENTLAYOUT CosUmdElementLayout::CastTo() const
{
    return MAKE_D3D10DDI_HELEMENTLAYOUT(const_cast< CosUmdElementLayout* >(this));
}
