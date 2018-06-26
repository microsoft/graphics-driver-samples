////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Command Allocator implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

//
// TODO: Support Reset and proper Command Buffer reuse behavior
//

CosUmd12CommandBuffer *
CosUmd12CommandAllocator::AcquireCommandBuffer(
    D3D12DDI_COMMAND_QUEUE_FLAGS queueFlags)
{
    return CosUmd12CommandBuffer::Create();
}

void
CosUmd12CommandAllocator::ReleaseCommandBuffer(CosUmd12CommandBuffer * pCommandBuffer)
{
    pCommandBuffer->~CosUmd12CommandBuffer();
}
