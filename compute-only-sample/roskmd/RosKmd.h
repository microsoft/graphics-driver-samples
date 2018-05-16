#pragma once

//
//    Render Only Sample (Ros) Kernel Mode Driver (Kmd) Header
//
//    Copyright (C) Microsoft.  All rights reserved.
//

#include "RosAllocation.h"
#include "RosAdapter.h"

#define ROSD_SEGMENT_APERTURE 1
#define ROSD_SEGMENT_VIDEO_MEMORY 2
#define ROSD_SEGMENT_APERTURE_BASE_ADDRESS  0xC0000000

#define ROSD_COMMAND_BUFFER_SIZE    PAGE_SIZE

#define ROSD_PAGING_BUFFER_SIZE     PAGE_SIZE

typedef struct _ROSPRIVATEINFO2 : public _ROSADAPTERINFO
{
    UINT             m_Dummy;
} ROSPRIVATEINFO2;

const UINT C_ROSD_DMAPRIVATEDATASIZE_KMD = 0x100;

typedef struct _ROSUMDDMAPRIVATEDATA
{
    BYTE              m_KmdPrivateData[C_ROSD_DMAPRIVATEDATASIZE_KMD];
    UINT              m_DmaPacketIndex;
    UINT              m_Dummy;
    UINT              m_DmaBufferSize;
} ROSUMDDMAPRIVATEDATA;

typedef struct _ROSDUMDMAPRIVATEDATA2 : public ROSUMDDMAPRIVATEDATA
{
    UINT  m_Dummy;
} ROSUMDDMAPRIVATEDATA2;

#define ROSD_VERSION 2

const int C_ROSD_ALLOCATION_LIST_SIZE = 64;
const int C_ROSD_PATCH_LOCATION_LIST_SIZE = 128;

const int C_ROSD_GPU_ENGINE_COUNT = 1;

#if DBG

// #define BINNER_DBG  1

#endif

// #define GPU_CACHE_WORKAROUND 1

