#pragma once

class RosUmdBlendState
{
public:

    RosUmdBlendState(const D3D11_1_DDI_BLEND_DESC * desc, D3D10DDI_HRTBLENDSTATE & hRT) :
        m_desc(*desc), m_hRTBlendState(hRT)
    {
        // do nothing
    }

    static RosUmdBlendState* CastFrom(D3D10DDI_HBLENDSTATE);
    D3D10DDI_HBLENDSTATE CastTo() const;

    const D3D11_1_DDI_BLEND_DESC *GetDesc()
    {
        return &m_desc;
    }

private:

    D3D11_1_DDI_BLEND_DESC m_desc;
    D3D10DDI_HRTBLENDSTATE m_hRTBlendState;
};

inline RosUmdBlendState* RosUmdBlendState::CastFrom(D3D10DDI_HBLENDSTATE hRasterizerState)
{
    return static_cast< RosUmdBlendState* >(hRasterizerState.pDrvPrivate);
}

inline D3D10DDI_HBLENDSTATE RosUmdBlendState::CastTo() const
{
    return MAKE_D3D10DDI_HBLENDSTATE(const_cast< RosUmdBlendState* >(this));
}
