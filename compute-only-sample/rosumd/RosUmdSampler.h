#pragma once

#pragma once

class RosUmdSampler
{
    friend class RosUmdDevice;

public:

    RosUmdSampler(const D3D10_DDI_SAMPLER_DESC * desc, D3D10DDI_HRTSAMPLER & hRT) :
        m_desc(*desc), m_hRTSampler(hRT)
    {
        // do nothing
    }

    static RosUmdSampler* CastFrom(D3D10DDI_HSAMPLER hSampler);
    D3D10DDI_HSAMPLER CastTo() const;

private:

    D3D10_DDI_SAMPLER_DESC m_desc;
    D3D10DDI_HRTSAMPLER m_hRTSampler;
};

inline RosUmdSampler* RosUmdSampler::CastFrom(D3D10DDI_HSAMPLER hSampler)
{
    return static_cast< RosUmdSampler* >(hSampler.pDrvPrivate);
}

inline D3D10DDI_HSAMPLER RosUmdSampler::CastTo() const
{
    return MAKE_D3D10DDI_HSAMPLER(const_cast< RosUmdSampler* >(this));
}
