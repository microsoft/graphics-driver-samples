#pragma once


#include "CosUmd12.h"

class CosUmd12CommandRecorder
{
public:
    explicit CosUmd12CommandRecorder(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_COMMAND_RECORDER_0040* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
        m_pCommandPool = NULL;
    }

    ~CosUmd12CommandRecorder()
    {
    }

    static int CalculateSize(const D3D12DDIARG_CREATE_COMMAND_RECORDER_0040 * pArgs)
    {
        return sizeof(CosUmd12CommandRecorder);
    }

    static CosUmd12CommandRecorder* CastFrom(D3D12DDI_HCOMMANDRECORDER_0040);
    D3D12DDI_HCOMMANDRECORDER_0040 CastTo() const;

    CosUmd12CommandPool* GetCommandPool()
    {
        return m_pCommandPool;
    }

    void SetCommandPoolAsTarget(CosUmd12CommandPool * pCommandPool)
    {
        m_pCommandPool = pCommandPool;
    }

private:
    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATE_COMMAND_RECORDER_0040 m_args;

    CosUmd12CommandPool * m_pCommandPool;
};

inline CosUmd12CommandRecorder* CosUmd12CommandRecorder::CastFrom(D3D12DDI_HCOMMANDRECORDER_0040 hCommandRecorder)
{
    return static_cast< CosUmd12CommandRecorder* >(hCommandRecorder.pDrvPrivate);
}

inline D3D12DDI_HCOMMANDRECORDER_0040 CosUmd12CommandRecorder::CastTo() const
{
    return MAKE_D3D12DDI_HCOMMANDRECORDER_0040(const_cast< CosUmd12CommandRecorder* >(this));
}
