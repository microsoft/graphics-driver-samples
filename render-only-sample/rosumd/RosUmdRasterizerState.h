#pragma once


class RosUmdRasterizerState
{
    friend class RosUmdDevice;

public:

    RosUmdRasterizerState(const D3D11_1_DDI_RASTERIZER_DESC * desc, D3D10DDI_HRTRASTERIZERSTATE & hRT) :
        m_desc(*desc), m_hRTRasterizerState(hRT)
    {
        // do nothing
    }

    static RosUmdRasterizerState* CastFrom(D3D10DDI_HRASTERIZERSTATE);
    D3D10DDI_HRASTERIZERSTATE CastTo() const;

    const D3D11_1_DDI_RASTERIZER_DESC *GetDesc()
    {
        return &m_desc;
    }

private:

    D3D11_1_DDI_RASTERIZER_DESC m_desc;
    D3D10DDI_HRTRASTERIZERSTATE m_hRTRasterizerState;
};

inline RosUmdRasterizerState* RosUmdRasterizerState::CastFrom(D3D10DDI_HRASTERIZERSTATE hRasterizerState)
{
    return static_cast< RosUmdRasterizerState* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HRASTERIZERSTATE RosUmdRasterizerState::CastTo() const
{
    return MAKE_D3D10DDI_HRASTERIZERSTATE(const_cast< RosUmdRasterizerState* >(this));
}
