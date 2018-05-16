#pragma once

class RosUmdElementLayout
{
friend RosUmdDevice;

public:

    RosUmdElementLayout(const D3D10DDIARG_CREATEELEMENTLAYOUT * create, D3D10DDI_HRTELEMENTLAYOUT & hRT) :
        m_hRTElementLayout(hRT)
    {
        // TODO(bhouse) Perhaps we should use vector?

        m_numElements = create->NumElements;
        m_pElementDesc = new D3D10DDIARG_INPUT_ELEMENT_DESC[m_numElements];

        memcpy(m_pElementDesc, create->pVertexElements, sizeof(D3D10DDIARG_INPUT_ELEMENT_DESC) * m_numElements);
    }

    ~RosUmdElementLayout()
    {
        delete[] m_pElementDesc;
    }

    static RosUmdElementLayout* CastFrom(D3D10DDI_HELEMENTLAYOUT);
    D3D10DDI_HELEMENTLAYOUT CastTo() const;

private:

    UINT                                m_numElements;
    D3D10DDIARG_INPUT_ELEMENT_DESC *    m_pElementDesc;
    D3D10DDI_HRTELEMENTLAYOUT           m_hRTElementLayout;
};

inline RosUmdElementLayout* RosUmdElementLayout::CastFrom(D3D10DDI_HELEMENTLAYOUT hRasterizerState)
{
    return static_cast< RosUmdElementLayout* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HELEMENTLAYOUT RosUmdElementLayout::CastTo() const
{
    return MAKE_D3D10DDI_HELEMENTLAYOUT(const_cast< RosUmdElementLayout* >(this));
}
