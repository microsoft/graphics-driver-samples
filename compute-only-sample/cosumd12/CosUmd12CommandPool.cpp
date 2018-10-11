////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Command Pool implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

//
// TODO: Support Reset and proper Command Buffer reuse behavior
//

CosUmd12CommandBuffer *
CosUmd12CommandPool::AcquireCommandBuffer(
    D3D12DDI_COMMAND_QUEUE_FLAGS queueFlags)
{
#if GPUVA

    return CosUmd12CommandBuffer::Create(m_pDevice);

#else

    return CosUmd12CommandBuffer::Create();

#endif
}

void
CosUmd12CommandPool::ReleaseCommandBuffer(CosUmd12CommandBuffer * pCommandBuffer)
{
#if GPUVA

    delete pCommandBuffer;

#else

    pCommandBuffer->~CosUmd12CommandBuffer();

#endif
}
