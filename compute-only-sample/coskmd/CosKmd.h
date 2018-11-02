#pragma once

//
//    Compute Only Sample (Cos) Kernel Mode Driver (Kmd) Header
//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "Cos.h"

#include <initguid.h>
#include <ntddk.h>
#include <ntintsafe.h>
#include <ntstrsafe.h>

#define ENABLE_DXGK_SAL

extern "C" {
#include <dispmprt.h>
#include <dxgiformat.h>
#include <WppRecorder.h>
} // extern "C"

#include "CosAllocation.h"
#include "CosContext.h"
#include "CosAdapter.h"

#define COS_SEGMENT_VIDEO_MEMORY 1

// TODO: Clean up later
#define COS_SEGMENT_APERTURE 2
#define COS_SEGMENT_APERTURE_BASE_ADDRESS  0xC0000000

#define COS_PAGING_BUFFER_SIZE     PAGE_SIZE

typedef struct _COSPRIVATEINFO2 : public _COSADAPTERINFO
{
    UINT             m_Dummy;
} COSPRIVATEINFO2;

const UINT C_COS_DMAPRIVATEDATASIZE_KMD = 0x100;

typedef struct _COSUMDDMAPRIVATEDATA
{
    BYTE              m_KmdPrivateData[C_COS_DMAPRIVATEDATASIZE_KMD];
    UINT              m_DmaPacketIndex;
    UINT              m_Dummy;
    UINT              m_DmaBufferSize;
} COSUMDDMAPRIVATEDATA;

typedef struct _COSDUMDMAPRIVATEDATA2 : public COSUMDDMAPRIVATEDATA
{
    UINT  m_Dummy;
} COSUMDDMAPRIVATEDATA2;

#define COS_VERSION 2

const int C_COS_GPU_ENGINE_COUNT = 1;

#if DBG

// #define BINNER_DBG  1

#endif

#if COS_GPUVA_SUPPORT
//
// The current GpuVA implementation declares 1 aperture segment
// so the implicit system memory segment (created by OS) is 2.
//
#define IMPLICIT_SYSTEM_MEMORY_SEGMENT_ID   2
#endif


// #define GPU_CACHE_WORKAROUND 1

