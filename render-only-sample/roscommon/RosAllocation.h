#pragma once

#pragma warning(disable : 4201)

#include <wingdi.h>
#include <d3d10umddi.h>

enum RosHwLayout
{
    Linear,
    Tiled
};

enum RosHwFormat
{
    X565d,
    X8888,
    X565,
    X32,
    X16,
    X8,
    D24S8       // For depth stencil
};

struct RosAllocationExchange
{
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

    // HW specific information calculated based on the fields above
    RosHwLayout             m_hwLayout;

    UINT                    m_hwWidthPixels;
    UINT                    m_hwHeightPixels;
    RosHwFormat             m_hwFormat;
    UINT                    m_hwPitchBytes;
    UINT                    m_hwSizeBytes;
};

struct RosAllocationGroupExchange
{
    int     m_dummy;
};