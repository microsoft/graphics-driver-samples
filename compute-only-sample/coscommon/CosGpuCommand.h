#pragma once

#pragma warning(disable:4201)

#if VC4
#include "Vc4Hw.h"
#endif

enum GpuCommandId
{
    Nop,
    ResourceCopy,
    Header = 'CSCB',
    RootSignatureSet = 'RTSS',
    ComputeShaderDispatch = 'CSDP'
};

struct GpuCommandBufferHeader
{
    union
    {
        struct
        {
            UINT    m_swCommandBuffer   : 1;
#if VC4

            UINT    m_hasVC4ClearColors : 1;

#endif
        };

        UINT        m_value;
    };

#if VC4

    VC4ClearColors  m_vc4ClearColors;

#endif
};

struct GpuResourceCopy
{
    PHYSICAL_ADDRESS    m_dstGpuAddress;
    PHYSICAL_ADDRESS    m_srcGpuAddress;
    size_t              m_sizeBytes;
};

struct GpuHWRootSignatureSet
{
    GpuCommandId    m_commandId;
    UINT            m_commandSize;

    UINT            m_numRootConstants;
    UINT            m_numRootDescriptorCbv;
    UINT            m_numRootDescriptorSrv;
    UINT            m_numRootDescriptorUav;
    //
    // Followed by m_numRootConstants of FLOAT
    //
    //
    // Followed by number Cbv + Srv + Uav of PHYSICAL_ADDRESS
    //
};

struct GpuHwComputeShaderDisptch
{
    GpuCommandId    m_commandId;
    UINT            m_commandSize;

    UINT            m_numThreadPerGroup;
    UINT            m_threadGroupCountX;
    UINT            m_threadGroupCountY;
    UINT            m_threadGroupCountZ;
    BYTE            m_ShaderHash[16];
    //
    // Followed by shader binary code
    //
};

struct GpuCommand
{
    GpuCommandId    m_commandId;

    union
    {
        GpuCommandBufferHeader  m_commandBufferHeader;
        GpuResourceCopy         m_resourceCopy;
    };
};
