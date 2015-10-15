#pragma once

#pragma warning(disable:4201)

#include "Vc4Hw.h"

enum GpuCommandId
{
    Nop,
    ResourceCopy,
    Header = 'RSCB'
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

struct GpuCommand
{
    GpuCommandId    m_commandId;

    union
    {
        GpuCommandBufferHeader  m_commandBufferHeader;
        GpuResourceCopy         m_resourceCopy;
    };
};
