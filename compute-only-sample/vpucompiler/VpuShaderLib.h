#pragma once

#include "Vpu.h"

typedef struct { float v0; float v1; float v2; float v3; int32_t mask; } dx_types_ResRet_f32;
typedef struct { int32_t v0; int32_t v1; int32_t v2; int32_t v3; int32_t mask; } dx_types_ResRet_i32;

typedef void * dx_types_Handle;

dx_types_Handle dx_op_createHandle(
    uint8_t resource_class,
    uint32_t range_id,
    uint32_t index,
    uint8_t non_uniform);

void dx_op_bufferLoad_f32(
	dx_types_ResRet_f32 * result,
	dx_types_Handle handle,
    uint32_t index,
    uint32_t offset);

void dx_op_bufferLoad_i32(
	dx_types_ResRet_i32 * result,
	dx_types_Handle handle,
    uint32_t index,
    uint32_t offset);

void dx_op_bufferStore_f32(
    dx_types_Handle handle,
    uint32_t index,
    uint32_t offset,
    float v0,
    float v1,
    float v2,
    float v3,
    uint8_t mask);

void dx_op_bufferStore_i32(
    dx_types_Handle handle,
    uint32_t index,
    uint32_t offset,
    int32_t v0,
    int32_t v1,
    int32_t v2,
    int32_t v3,
	uint8_t mask);

uint32_t dx_op_threadId_i32(void);
