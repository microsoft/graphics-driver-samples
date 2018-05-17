#pragma once

#pragma once

class CosUmdSampler
{
    friend class CosUmdDevice;

public:

    CosUmdSampler(const D3D10_DDI_SAMPLER_DESC * desc, D3D10DDI_HRTSAMPLER & hRT) :
        m_desc(*desc), m_hRTSampler(hRT)
    {
        // do nothing
    }

    static CosUmdSampler* CastFrom(D3D10DDI_HSAMPLER hSampler);
    D3D10DDI_HSAMPLER CastTo() const;

private:

    D3D10_DDI_SAMPLER_DESC m_desc;
    D3D10DDI_HRTSAMPLER m_hRTSampler;
};

inline CosUmdSampler* CosUmdSampler::CastFrom(D3D10DDI_HSAMPLER hSampler)
{
    return static_cast< CosUmdSampler* >(hSampler.pDrvPrivate);
}

inline D3D10DDI_HSAMPLER CosUmdSampler::CastTo() const
{
    return MAKE_D3D10DDI_HSAMPLER(const_cast< CosUmdSampler* >(this));
}
