#pragma once

#pragma warning(disable:4201)

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
        };

        UINT        m_value;
    };
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
