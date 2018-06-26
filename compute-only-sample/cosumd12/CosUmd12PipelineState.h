#pragma once

#pragma once

#pragma once

#pragma once

#include "CosUmd12.h"

class CosUmd12Device;

class CosUmd12PipelineState
{
public:
    explicit CosUmd12PipelineState(CosUmd12Device* pDevice, D3D12DDI_HRTPIPELINESTATE hRTPipelineState, const D3D12DDIARG_CREATE_PIPELINE_STATE_0033* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
        m_hRTPipelineState = hRTPipelineState;
    }

    ~CosUmd12PipelineState()
    {
    }

    static int CalculateSize(const D3D12DDIARG_CREATE_PIPELINE_STATE_0033 * pArgs)
    {
        return sizeof(CosUmd12PipelineState);
    }

    static CosUmd12PipelineState* CastFrom(D3D12DDI_HPIPELINESTATE);
    D3D12DDI_HPIPELINESTATE CastTo() const;

private:

    friend class CosUmd12CommandList;

    CosUmd12Device * m_pDevice;
    D3D12DDI_HRTPIPELINESTATE m_hRTPipelineState;
    D3D12DDIARG_CREATE_PIPELINE_STATE_0033 m_args;

};

inline CosUmd12PipelineState* CosUmd12PipelineState::CastFrom(D3D12DDI_HPIPELINESTATE hPipelineState)
{
    return static_cast< CosUmd12PipelineState* >(hPipelineState.pDrvPrivate);
}

inline D3D12DDI_HPIPELINESTATE CosUmd12PipelineState::CastTo() const
{
    return MAKE_D3D12DDI_HPIPELINESTATE(const_cast< CosUmd12PipelineState* >(this));
}


