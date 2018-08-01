#pragma once

#pragma warning(disable : 4201)

#include <wingdi.h>
#include <d3d10umddi.h>
#include <d3d12umddi.h>

enum CosHwLayout
{
    Linear,
    Tiled
};

enum CosHwFormat
{
    X565d,
    X8888,
    X565,
    X32,
    X16,
    X8,
    D24S8       // For depth stencil
};

struct CosAllocationExchange
{
    static const int kMagic = 'caex';

    UINT                    m_magic;

    // Input from UMD CreateResource DDI
    D3D10DDIRESOURCE_TYPE   m_resourceDimension;

    D3D10DDI_MIPINFO        m_mip0Info;
    UINT                    m_usage;        // D3D10_DDI_RESOURCE_USAGE
    UINT                    m_bindFlags;    // D3D10_DDI_RESOURCE_BIND_FLAG
    UINT                    m_mapFlags;     // D3D10_DDI_MAP
    UINT                    m_miscFlags;    // D3D10_DDI_RESOURCE_MISC_FLAG
    DXGI_FORMAT             m_format;
    DXGI_SAMPLE_DESC        m_sampleDesc;
    UINT                    m_mipLevels;
    UINT                    m_arraySize;

    bool                    m_isPrimary;
    DXGI_DDI_PRIMARY_DESC   m_primaryDesc;

    CosHwLayout             m_hwLayout;
    UINT                    m_hwWidthPixels;
    UINT                    m_hwHeightPixels;
    UINT                    m_hwSizeBytes;

    bool                    m_shared;
    bool                    m_cpuVisible;
    UINT64                  m_dataSize;
    D3D12DDI_TEXTURE_LAYOUT m_textureLayout;
};

struct CosAllocationGroupExchange
{
    int     m_dummy;
};

