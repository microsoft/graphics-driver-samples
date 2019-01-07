#include "VpuShaderLib.h"

#ifdef _WINDLL
#include <stdio.h>
#include <assert.h>
#endif

#ifdef _WINDLL
extern __declspec(dllexport) VpuThreadLocalStorage g_tls;
#endif

VpuThreadLocalStorage g_tls;

void *memset(void *src, int c, size_t n)
{
	int8_t * ptr = (int8_t *) src;
	while(n-- > 0) *ptr++ = c;
	return src;
}

dx_types_Handle dx_op_createHandle(
    uint8_t resource_class,
    uint32_t range_id,
    uint32_t index,
    uint8_t non_uniform)
{
#ifdef _WINDLL
    assert((int32_t)resource_class == kVpuResourceClassUAV);
    assert(index < kVpuMaxUAVs);
#endif

    
	return (dx_types_Handle) &g_tls.m_uavs[index];
}

void dx_op_bufferLoad_f32(
	dx_types_ResRet_f32  * result,
    dx_types_Handle dxHandle,
    uint32_t index,
    uint32_t offset)
{
	VpuResourceHandle handle = (VpuResourceHandle) dxHandle;

	result->v1 = result->v2 = result->v3 = 0.0f;
	result->mask = 0;

    result->v0 = *((float *)(handle->m_base + (index * handle->m_elementSize) + offset));
}

void dx_op_bufferLoad_i32(
	dx_types_ResRet_i32  * result,
    dx_types_Handle dxHandle,
    uint32_t index,
    uint32_t offset)
{
	VpuResourceHandle handle = (VpuResourceHandle) dxHandle;

	result->v1 = result->v2 = result->v3 = 0;
	result->mask = 0;
	
    result->v0 = *((int32_t *)(handle->m_base + (index * handle->m_elementSize) + offset));
}

void dx_op_bufferStore_f32(
    dx_types_Handle dxHandle,
    uint32_t index,
    uint32_t offset,
    float v0,
    float v1,
    float v2,
    float v3,
    uint8_t mask)
{
	VpuResourceHandle handle = (VpuResourceHandle) dxHandle;

#ifdef _WINDLL
    assert(mask == 1);
#endif

    *((float *)(handle->m_base + (index * handle->m_elementSize) + offset)) = v0;
}

void dx_op_bufferStore_i32(
    dx_types_Handle dxHandle,
    uint32_t index,
    uint32_t offset,
    int32_t v0,
    int32_t v1,
    int32_t v2,
    int32_t v3,
    uint8_t mask)
{
	VpuResourceHandle handle = (VpuResourceHandle) dxHandle;

#ifdef _WINDLL
    assert(mask == 1);
#endif

    *((int32_t *)(handle->m_base + (index * handle->m_elementSize) + offset)) = v0;
}

uint32_t dx_op_threadId_i32(void)
{
    return g_tls.m_id;
}
