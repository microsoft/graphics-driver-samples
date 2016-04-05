////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Device implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "RosUmdLogging.h"
#include "RosUmdUtil.tmh"

#include "RosUmdUtil.h"

#if VC4

#include "Vc4Hw.h"

#endif

UINT
ConvertFloatColor(
    DXGI_FORMAT format,
    FLOAT * pColor)
{
    BYTE    red, green, blue, alpha;

    red   = (BYTE)round(pColor[0] * 255.0);
    green = (BYTE)round(pColor[1] * 255.0);
    blue  = (BYTE)round(pColor[2] * 255.0);
    alpha = (BYTE)round(pColor[3] * 255.0);

    // TODO: define channel mapping for each supported DXGI_FORMAT.

    switch (format)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return (alpha << 24) | (blue << 16) | (green << 8) | red;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return (alpha << 24) | (red << 16)  | (green << 8) | blue;
    default:
        __debugbreak();
        return 0;
    }
}

#if VC4

VC4PrimitiveMode
ConvertD3D11Topology(
    D3D10_DDI_PRIMITIVE_TOPOLOGY    topology)
{
    static VC4PrimitiveMode vc4PrimitiveModes[6] =
    {
        VC4PrimitiveMode(0xFF),
        VC4_POINTS,
        VC4_LINES,
        VC4_LINE_STRIP,
        VC4_TRIANGLES,
        VC4_TRIANGLE_STRIP
    };

    if (topology > D3D10_DDI_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP)
    {
        topology = D3D10_DDI_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    return vc4PrimitiveModes[topology];
}

VC4DepthTestFunc
ConvertD3D11DepthComparisonFunc(
    D3D10_DDI_COMPARISON_FUNC   comparisonFunc)
{
    return (VC4DepthTestFunc)(comparisonFunc - 1);
}

VC4TextureWrap
ConvertD3D11TextureAddressMode(
    D3D10_DDI_TEXTURE_ADDRESS_MODE  texAddressMode)
{
    static VC4TextureWrap vc4TextureWraps[6] =
    {
        VC4TextureWrap(0xFF),
        VC4_TEX_REPEAT,
        VC4_TEX_MIRROR,
        VC4_TEX_CLAMP,
        VC4_TEX_BORDER,
        VC4_TEX_MIRROR
    };

    return vc4TextureWraps[texAddressMode];
}

VC4TextureMagFilter
ConvertD3D11TextureMagFilter(
    D3D10_DDI_FILTER    texFilter)
{
    return (texFilter & 0x4) ? VC4_TEX_MAG_LINEAR : VC4_TEX_MAG_NEAREST;
}

VC4TextureMinFilter
ConvertD3D11TextureMinFilter(
    D3D10_DDI_FILTER    texFilter,
    BOOLEAN             bClampToLOD0)
{
    if (bClampToLOD0)
    {
        return (texFilter & 0x10) ? VC4_TEX_MIN_LINEAR : VC4_TEX_MIN_NEAREST;
    }
    else
    {
        static VC4TextureMinFilter vc4TextureMinFilters[4] =
        {
            VC4_TEX_MIN_NEAR_MIP_NEAR,
            VC4_TEX_MIN_NEAR_MIP_LIN,
            VC4_TEX_MIN_LIN_MIP_NEAR,
            VC4_TEX_MIN_LIN_MIP_LIN,
        };

        return vc4TextureMinFilters[((texFilter & 0x10) >> 3) | (texFilter & 0x1)];
    }
}

#endif

