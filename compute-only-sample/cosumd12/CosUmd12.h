#pragma once

#include <windows.h>

//
// Must define NTSTATUS due to d3dkmddi.h which is included by d3d10umddi.h
//

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;

#include <d3d12.h>
#include <d3d12umddi.h>
#include <new>
#include <assert.h>

#include <list>

void TraceFunction(const char * function, const char * file, int line);

#define TRACE_FUNCTION() TraceFunction(__FUNCTION__, __FILE__, __LINE__);

void AssertFunction(const char * function, const char * file, int line);

#define ASSERT_FUNCTION() AssertFunction(__FUNCTION__, __FILE__, __LINE__);

#define ASSERT(cond) if (!(cond)) ASSERT_FUNCTION()

void UnexpectedDdi(const char * function, const char * file, int line);

#define UNEXPECTED_DDI() UnexpectedDdi(__FUNCTION__, __FILE__, __LINE__);

#include "CosAllocation.h"
#include "CosContext.h"
#include "CosGpuCommand.h"

#include "CosUmd12Adapter.h"
#include "CosUmd12Device.h"

#if GPUVA
#include "CosUmd12CommandQueueGpuVa.h"
#else
#include "CosUmd12CommandQueue.h"
#endif

#include "CosUmd12Heap.h"
#include "CosUmd12Resource.h"

#if GPUVA
#include "CosUmd12CommandBufferGpuVa.h"
#else
#include "CosUmd12CommandBuffer.h"
#endif

#include "CosUmd12RootSignature.h"
#include "CosUmd12Shader.h"
#include "CosUmd12PipelineState.h"
#include "CosUmd12Descriptor.h"
#include "CosUmd12CommandPool.h"
#include "CosUmd12CommandRecorder.h"

#if GPUVA
#include "CosUmd12CommandListGpuVa.h"
#else
#include "CosUmd12CommandList.h"
#endif

#include "CosUmd12Fence.h"
#include "CosUmd12DescriptorHeap.h"

#include "CosMetaCommand.h"
#include "CosUmd12MetaCommand.h"

