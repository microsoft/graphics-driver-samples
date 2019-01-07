#pragma once

#include <windows.h>

#include "Cos.h"

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

#if COS_GPUVA_SUPPORT
#include "CosUmd12CommandQueueGpuVa.h"
#else
#include "CosUmd12CommandQueue.h"
#endif

#include "CosUmd12Heap.h"
#include "CosUmd12Resource.h"

#if COS_GPUVA_SUPPORT
#include "CosUmd12CommandBufferGpuVa.h"
#else
#include "CosUmd12CommandBuffer.h"
#endif

#if COS_GPUVA_SUPPORT
#include "CosUmd12RootSignatureGpuVa.h"
#elif COS_RS_2LEVEL_SUPPORT
#include "CosUmd12RootSignature2L.h"
#else
#include "CosUmd12RootSignature.h"
#endif

#include "CosUmd12Shader.h"
#include "CosUmd12PipelineState.h"

#include "CosUmd12DescriptorGpuVa.h"
#include "CosUmd12Descriptor2L.h"
#include "CosUmd12Descriptor.h"

#include "CosUmd12CommandPool.h"
#include "CosUmd12CommandRecorder.h"

#if COS_GPUVA_SUPPORT
#include "CosUmd12CommandListGpuVa.h"
#else
#include "CosUmd12CommandList.h"
#endif

#include "CosUmd12Fence.h"

#if COS_GPUVA_SUPPORT
#include "CosUmd12DescriptorHeapGpuVa.h"
#elif COS_RS_2LEVEL_SUPPORT
#include "CosUmd12DescriptorHeap2L.h"
#else
#include "CosUmd12DescriptorHeap.h"
#endif

#include "CosMetaCommand.h"
#include "CosUmd12MetaCommand.h"

