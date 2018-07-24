#pragma once

#include "CosUmd12.h"

class CosUmd12CommandBuffer;

class CosUmd12CommandPool
{
public:
    explicit CosUmd12CommandPool(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_COMMAND_POOL_0040* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
    }

    ~CosUmd12CommandPool()
    {
    }

    void Reset()
    {
        // do nothing
    }

    static int CalculateSize(const D3D12DDIARG_CREATE_COMMAND_POOL_0040 * pArgs)
    {
        return sizeof(CosUmd12CommandPool);
    }

    static CosUmd12CommandPool* CastFrom(D3D12DDI_HCOMMANDPOOL_0040);
    D3D12DDI_HCOMMANDALLOCATOR CastTo() const;

    //
    // Interface between Command Allocator and Command List
    //
    CosUmd12CommandBuffer * AcquireCommandBuffer(D3D12DDI_COMMAND_QUEUE_FLAGS queueFlags);
    void ReleaseCommandBuffer(CosUmd12CommandBuffer * pCommandBuffer);

private:

    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATE_COMMAND_POOL_0040 m_args;

};

inline CosUmd12CommandPool* CosUmd12CommandPool::CastFrom(D3D12DDI_HCOMMANDPOOL_0040 hCommandPool)
{
    return static_cast< CosUmd12CommandPool* >(hCommandPool.pDrvPrivate);
}

inline D3D12DDI_HCOMMANDALLOCATOR CosUmd12CommandPool::CastTo() const
{
    // TODO: Why does MAKE_D3D10DDI_HDEPTHSTENCILSTATE not exist?
    return MAKE_D3D12DDI_HCOMMANDALLOCATOR(const_cast< CosUmd12CommandPool* >(this));
}
