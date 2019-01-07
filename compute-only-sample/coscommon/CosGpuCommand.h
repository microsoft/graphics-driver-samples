#pragma once

#if COS_MLMC_RS5_SUPPORT

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
    QwordWrite = 'QWWT',
    DescriptorHeapSet = 'DHST',
    RootSignature2LevelSet = 'RS2S',
    MetaCommandExecute = 'MCEX'
};

struct GpuCommandBufferHeader
{
    union
    {
        struct
        {
            UINT    m_swCommandBuffer       : 1;
#if COS_GPUVA_SUPPORT

            UINT    m_gpuVaCommandBuffer    : 1;

#endif
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

#if COS_GPUVA_SUPPORT || COS_RS_2LEVEL_SUPPORT

struct GpuHwQwordWrite
{
    GpuCommandId        m_commandId;

    PHYSICAL_ADDRESS    m_gpuAddress;
    ULONGLONG           m_data;
};

enum GpuHwDescriptorType
{
    COS_CBV = 1,
    COS_SRV = 2,
    COS_UAV = 3,
};

struct GpuHwConstantBufferView
{
    UINT m_sizeInBytes;
    UINT m_padding;
};

struct GpuHwUnorderAccessView
{
    DXGI_FORMAT                                 m_format;
    D3D12DDI_RESOURCE_DIMENSION                 m_resourceDimension;
    D3D12DDIARG_BUFFER_UNORDERED_ACCESS_VIEW    m_buffer;
};

struct GpuHWDescriptor
{
    GpuHwDescriptorType         m_type;
    union
    {
        GpuHwConstantBufferView m_cbv;
        GpuHwUnorderAccessView  m_uav;
    };
    PHYSICAL_ADDRESS            m_resourceGpuAddress;
};

struct GpuHwDescriptorHeapSet
{
    GpuCommandId        m_commandId;

    PHYSICAL_ADDRESS    m_descriptorHeapGpuAddress;
};

struct GpuHWRootSignature2LSet
{
    GpuCommandId    m_commandId;
    UINT            m_commandSize;

    BYTE            m_rootValues[1];
};

#else

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

#endif  // COS_RS_2LEVEL_SUPPORT

struct GpuHwComputeShaderDisptch
{
    GpuCommandId    m_commandId;
    UINT            m_commandSize;

    UINT            m_threadCountX;
    UINT            m_threadCountY;
    UINT            m_threadCountZ;
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
#if COS_MLMC_RS5_SUPPORT
    MetaCommandNormalization    = 100,
    MetaCommandConvolution      = 101,
    MetaCommandGEMM             = 102,
    MetaCommandGRU              = 103,
    MetaCommandLSTM             = 104,
    MetaCommandMVN              = 105,
    MetaCommandPooling          = 106,
    MetaCommandReduction        = 107,
    MetaCommandRNN              = 108,
    MetaCommandRoiPooling       = 109,
    MetaCommandCopyTensor       = 110
#endif
};

#if COS_MLMC_RS5_SUPPORT

struct HW_META_COMMAND_COPY_TENSOR
{
    META_COMMAND_TENSOR_DESC DstDesc;
    META_COMMAND_TENSOR_DESC SrcDesc;
    META_COMMAND_BIND_FLAGS BindFlags;
};

struct HW_IO_TABLE_COPY_TENSOR
{
    D3D12_GPU_DESCRIPTOR_HANDLE DstResource;
    D3D12_GPU_DESCRIPTOR_HANDLE SrcResource;
    D3D12_GPU_DESCRIPTOR_HANDLE TemporaryResource;
};

#endif

struct GpuHwMetaCommand
{
    GpuCommandId    m_commandId;
    UINT            m_commandSize;

    MetaCommandId   m_metaCommandId;

#if COS_MLMC_RS5_SUPPORT
    //
    // Followed by THwMetaCommand structure (per type of meta command)
    //
    // COSD commonly reuses META_COMMAND_CREATE_*_DESC
    //
    // For HW driver it may contain compiled code for meta command and
    // other info for its execution
    //

    //
    // Followed by THwIoTable structure (per type of meta command)
    //
    // COSD commonly reuses META_COMMAND_EXECUTE_*_DESC
    // So it commonly has the same number of PHYSICAL_ADDRESS "slots" as
    // D3D12_GPU_DESCRIPTOR_HANDLE in META_COMMAND_EXECUTE_*_DESC
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
