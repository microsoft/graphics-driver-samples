#pragma once

#if MLMC

#ifndef __d3d12_h__

typedef struct D3D12_GPU_DESCRIPTOR_HANDLE
{
    UINT64 ptr;
} D3D12_GPU_DESCRIPTOR_HANDLE;

#endif

#include "MetaCommandDefinitions.h"

#endif

#pragma warning(disable:4201)

enum GpuCommandId
{
    Nop,
    ResourceCopy,
    Header = 'CSCB',
    RootSignatureSet = 'RTSS',
    ComputeShaderDispatch = 'CSDP',
    MetaCommandExecute = 'MCEX'
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
    UINT                m_sizeBytes;
};

//
// A constant register is loaded directly with Root Constant value or through a Constant Buffer
//

union GpuHWConstantDescriptor
{
    FLOAT               m_constantValues[4];
    PHYSICAL_ADDRESS    m_resourceGpuAddress;
};

struct GpuHWDescriptor
{
    PHYSICAL_ADDRESS    m_resourceGpuAddress;
};

struct GpuHWRootSignatureSet
{
    GpuCommandId    m_commandId;
    UINT            m_commandSize;

    UINT            m_numCbvRegisters;
    UINT            m_numSrvRegisters;
    UINT            m_numUavRegisters;

    //
    // Followed by number Cbv of GpuHWConstantDescriptor
    //

    //
    // Followed by number Srv + Uav of GpuHWDescriptor
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

enum MetaCommandId
{
    MetaCommandIdentity         = 1,    // TODO: Switch identity meta command to the new code path
#if MLMC
    MetaCommandNormalization    = 100,
    MetaCommandConvolution      = 101,
#endif
};

struct GpuHwMetaCommand
{
    GpuCommandId    m_commandId;
    UINT            m_commandSize;

    MetaCommandId   m_metaCommandId;

#if MLMC
    //
    // Followed by META_COMMAND_CREATE_*_DESC
    //

    //
    // Followed by same number of PHYSICAL_ADDRESS "slots" as
    // D3D12_GPU_DESCRIPTOR_HANDLE in META_COMMAND_EXECUTE_*_DESC
    //

    //
    // For HW driver followed by the compiled code for meta command or other
    // info for its execution
    //
#endif
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
