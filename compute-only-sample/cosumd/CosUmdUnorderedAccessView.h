#pragma once

#include "CosUmd.h"

class CosUmdUnorderedAccessView
{
friend class CosUmdDevice;

public:

    CosUmdUnorderedAccessView(const D3D11DDIARG_CREATEUNORDEREDACCESSVIEW * pCreate, D3D11DDI_HRTUNORDEREDACCESSVIEW & hRT) :
        m_create(*pCreate), m_hRTUnorderedAccessView(hRT)
    {
        // do nothing
    }

    ~CosUmdUnorderedAccessView()
    {
        // do nothing
    }

    static CosUmdUnorderedAccessView* CastFrom(D3D11DDI_HUNORDEREDACCESSVIEW);
    D3D11DDI_HUNORDEREDACCESSVIEW CastTo() const;

private:

    D3D11DDIARG_CREATEUNORDEREDACCESSVIEW m_create;
    D3D11DDI_HRTUNORDEREDACCESSVIEW m_hRTUnorderedAccessView;
};

inline CosUmdUnorderedAccessView* CosUmdUnorderedAccessView::CastFrom(D3D11DDI_HUNORDEREDACCESSVIEW hRasterizerState)
{
    return static_cast< CosUmdUnorderedAccessView* >(hRasterizerState.pDrvPrivate);
}

inline D3D11DDI_HUNORDEREDACCESSVIEW CosUmdUnorderedAccessView::CastTo() const
{
    return MAKE_D3D11DDI_HUNORDEREDACCESSVIEW(const_cast< CosUmdUnorderedAccessView* >(this));
}

