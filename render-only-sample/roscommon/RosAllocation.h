#pragma once

#pragma warning(disable : 4201)

#include <wingdi.h>
#include <d3d10umddi.h>

#include <Vc4Hw.h>

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

    RosHwLayout             m_hwLayout;
    UINT                    m_hwWidthPixels;
    UINT                    m_hwHeightPixels;
    UINT                    m_hwSizeBytes;
};

struct RosAllocationGroupExchange
{
    int     m_dummy;
};

inline
VC4_NON_HDR_FRAME_BUFFER_COLOR_FORMAT
Vc4FrameBufferColorFormatFromDxgiFormat (
    DXGI_FORMAT Format
    )
{
    switch (Format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return VC4_NON_HDR_FRAME_BUFFER_COLOR_FORMAT::RGBA8888;
    default:
        __debugbreak();
        return VC4_NON_HDR_FRAME_BUFFER_COLOR_FORMAT::RGBA8888;
    }
}

inline VC4TileBufferPixelFormat Vc4TileBufferPixelFormatFromDxgiFormat (
    DXGI_FORMAT Format
    )
{
    switch (Format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return VC4_TILE_BUFFER_PIXEL_FORMAT_RGBA8888;
    default:
        __debugbreak();
        return VC4_TILE_BUFFER_PIXEL_FORMAT_RGBA8888;
    }
}

inline VC4_MEMORY_FORMAT Vc4MemoryFormatFromRosHwLayout (RosHwLayout Layout)
{
    switch (Layout) {
    case RosHwLayout::Linear: return VC4_MEMORY_FORMAT::LINEAR;
    case RosHwLayout::Tiled: return VC4_MEMORY_FORMAT::T_FORMAT;
    default:
        __debugbreak();
        return VC4_MEMORY_FORMAT::LINEAR;
    }
}

inline VC4TextureDataType Vc4TextureTypeFromDxgiFormat (
    RosHwLayout Layout,
    DXGI_FORMAT Format
    )
{
    switch (Layout) {
    case RosHwLayout::Linear:
        switch (Format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return VC4_TEX_RGBA32R;
        default:
            __debugbreak();
            return VC4_TEX_RGBA32R;
        }
        break;
    case RosHwLayout::Tiled:
        switch (Format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return VC4_TEX_RGBX8888; // XXX: shouldn't this be VC4_TEX_RGBA8888?
            break;
        default:
            __debugbreak();
            return VC4_TEX_RGBX8888;
        }
    default:
        __debugbreak();
        return VC4_TEX_RGBA32R;
    }
}