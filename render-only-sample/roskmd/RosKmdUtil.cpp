////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utility implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

#include "RosKmdLogging.h"
#include "RosKmdUtil.tmh"

#include "RosKmdAdapter.h"
#include "RosKmdContext.h"
#include "RosKmdAllocation.h"
#include "RosGpuCommand.h"
#include "RosKmdGlobal.h"
#include "RosKmdUtil.h"

#if VC4

#include "Vc4Hw.h"

#endif

D3DDDIFORMAT
TranslateDxgiFormat(
    DXGI_FORMAT dxgiFormat)
{
    switch (dxgiFormat)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return D3DDDIFMT_A8B8G8R8;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return D3DDDIFMT_A2B10G10R10;
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        return D3DDDIFMT_A2B10G10R10_XR_BIAS;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return D3DDDIFMT_A16B16G16R16F;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return D3DDDIFMT_A8R8G8B8;
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return D3DDDIFMT_X8R8G8B8;
    case DXGI_FORMAT_B5G6R5_UNORM:
        return D3DDDIFMT_R5G6B5;
    case DXGI_FORMAT_B5G5R5A1_UNORM:
        return D3DDDIFMT_A1R5G5B5;
    case DXGI_FORMAT_B4G4R4A4_UNORM:
        return D3DDDIFMT_A4R4G4B4;
    case DXGI_FORMAT_A8_UNORM:
        return D3DDDIFMT_A8;
    case DXGI_FORMAT_R16G16_UNORM:
        return D3DDDIFMT_G16R16;
    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return D3DDDIFMT_A16B16G16R16;
    case DXGI_FORMAT_R16_FLOAT:
        return D3DDDIFMT_R16F;
    case DXGI_FORMAT_R16G16_FLOAT:
        return D3DDDIFMT_G16R16F;
    case DXGI_FORMAT_R32_FLOAT:
        return D3DDDIFMT_R32F;
    case DXGI_FORMAT_R32G32_FLOAT:
        return D3DDDIFMT_G32R32F;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return D3DDDIFMT_A32B32G32R32F;
    case DXGI_FORMAT_YUY2:
        return D3DDDIFMT_YUY2;
    case DXGI_FORMAT_D16_UNORM:
        return D3DDDIFMT_D16_LOCKABLE;
    case DXGI_FORMAT_D32_FLOAT:
        return D3DDDIFMT_D32F_LOCKABLE;
    case DXGI_FORMAT_R8G8_SNORM:
        return D3DDDIFMT_V8U8;
    case DXGI_FORMAT_R8_UNORM:
        return D3DDDIFMT_R8;
    case DXGI_FORMAT_R8G8_UNORM:
        return D3DDDIFMT_G8R8;
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
        return D3DDDIFMT_DXT1;
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
        return D3DDDIFMT_DXT2;
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
        return D3DDDIFMT_DXT3;
    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return D3DDDIFMT_Q8W8V8U8;
    case DXGI_FORMAT_R16G16_SNORM:
        return D3DDDIFMT_V16U16;
    case DXGI_FORMAT_UNKNOWN:
        return D3DDDIFMT_UNKNOWN;
    default:
        NT_ASSERT(FALSE);
        return D3DDDIFMT_UNKNOWN;
    }
}

VC4_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
DXGI_FORMAT DxgiFormatFromD3dDdiFormat (D3DDDIFORMAT Format)
{
    switch (Format)
    {
    case D3DDDIFMT_A8:
        return DXGI_FORMAT_A8_UNORM;
    case D3DDDIFMT_L8:
        // Map D3D9 L8 format to D3D11 R8 - see shader converter EmitSamplerSwizzle()
        return DXGI_FORMAT_R8_UNORM;  
    case D3DDDIFMT_L16:
        // Map D3D9 L16 format to D3D11 R16 - see shader converter EmitSamplerSwizzle()
        return DXGI_FORMAT_R16_UNORM;  
    case D3DDDIFMT_R5G6B5:
        return DXGI_FORMAT_B5G6R5_UNORM;
    case D3DDDIFMT_A1R5G5B5:
        return DXGI_FORMAT_B5G5R5A1_UNORM;
    case D3DDDIFMT_A8R8G8B8:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case D3DDDIFMT_X8R8G8B8:
        ROS_LOG_WARNING("Forcing D3DDDIFMT_X8R8G8B8 to DXGI_FORMAT_B8G8R8A8_UNORM");
        return DXGI_FORMAT_B8G8R8A8_UNORM;//DXGI_FORMAT_R8G8B8A8_UNORM;
    case D3DDDIFMT_A8B8G8R8:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case D3DDDIFMT_YUY2:
        return DXGI_FORMAT_YUY2;
    case D3DDDIFMT_DXT1:
        return DXGI_FORMAT_BC1_UNORM;
    case D3DDDIFMT_DXT2:
    case D3DDDIFMT_DXT3:
        return DXGI_FORMAT_BC2_UNORM;
    case D3DDDIFMT_DXT4:
    case D3DDDIFMT_DXT5:
        return DXGI_FORMAT_BC3_UNORM;
    case D3DDDIFMT_V8U8:
        return DXGI_FORMAT_R8G8_SNORM;
    case D3DDDIFMT_D16:
    case D3DDDIFMT_D16_LOCKABLE:
        return DXGI_FORMAT_D16_UNORM;
    case D3DDDIFMT_D32F_LOCKABLE:
        return DXGI_FORMAT_D32_FLOAT;
    case D3DDDIFMT_S8D24:
    case D3DDDIFMT_X8D24:
    case D3DDDIFMT_D24S8:
    case D3DDDIFMT_D24X8:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case D3DDDIFMT_A16B16G16R16:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    case D3DDDIFMT_R16F:
        return DXGI_FORMAT_R16_FLOAT;
    case D3DDDIFMT_G16R16F:
        return DXGI_FORMAT_R16G16_FLOAT;
    case D3DDDIFMT_A16B16G16R16F:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case D3DDDIFMT_R32F:
        return DXGI_FORMAT_R32_FLOAT;
    case D3DDDIFMT_G32R32F:
        return DXGI_FORMAT_R32G32_FLOAT;
    case D3DDDIFMT_A32B32G32R32F:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case D3DDDIFMT_A2B10G10R10:
        return DXGI_FORMAT_R10G10B10A2_UNORM;
    case D3DDDIFMT_G16R16:
        return DXGI_FORMAT_R16G16_UNORM;
    case D3DDDIFMT_V16U16:
        return DXGI_FORMAT_R16G16_SNORM;
    case D3DDDIFMT_Q8W8V8U8:
        return DXGI_FORMAT_R8G8B8A8_SNORM;
    case D3DDDIFMT_Q16W16V16U16:
        return DXGI_FORMAT_R16G16B16A16_SNORM;
    case D3DDDIFMT_A4R4G4B4:
        return DXGI_FORMAT_B4G4R4A4_UNORM;
    case D3DDDIFMT_P8:
        return DXGI_FORMAT_P8;
    case D3DDDIFMT_A8P8:
        return DXGI_FORMAT_A8P8;
    case D3DDDIFMT_VERTEXDATA:
    case D3DDDIFMT_INDEX16:
    case D3DDDIFMT_INDEX32:
    case D3DDDIFMT_UNKNOWN:
        return DXGI_FORMAT_UNKNOWN;
    default:
        NT_ASSERT(!"Unknown format");
        return DXGI_FORMAT_UNKNOWN;
    }
}

VC4_PAGED_SEGMENT_END; //=====================================================


#if VC4

//
// Make sure PDB file has type definition for VC4 Control List commands
//

// Code: 28
static VC4StoreTileBufferGeneral   *pVC4StoreTileBufferGeneral = NULL;

// Code: 32
static VC4IndexedPrimitiveList     *pVC4IndexedPrimitiveList = NULL;

// Code: 33
static VC4VertexArrayPrimitives    *pVC4VertexArrayPrimitives = NULL;

// Code: 56
static VC4PrimitiveListFormat      *pVC4PrimitiveListFormat = NULL;

// Code: 64
static VC4GLShaderState            *pVC4GLShaderState = NULL;

static VC4GLShaderStateRecord      *pVC4GLShaderStateRecord = NULL;

static VC4VertexAttribute          *pVC4VertexAttribute = NULL;

// Code: 65
static VC4NVShaderState            *pVC4NVShaderState = NULL;

static VC4NVShaderStateRecord      *pVC4NVShaderStateRecord = NULL;

static VC4TextureConfigParameter0  *pVC4TextureConfigParameter0 = NULL;
static VC4TextureConfigParameter1  *pVC4TextureConfigParameter1 = NULL;
static VC4TextureDataType          vc4TextureDataType = { VC4_TEX_RGBA32R };

// Code: 96
static VC4ConfigBits               *pVC4ConfigBits = NULL;

// Code: 97
static VC4FlatShadeFlags           *pVC4FlatShadeFlags = NULL;

// Code: 98
static VC4PointSize                *pVC4PointSize = NULL;

// Code: 99
static VC4LineWidth                *pVC4LineWidth = NULL;

// Code: 101
static VC4DepthOffset              *pVC4DepthOffset = NULL;

// Code: 102
static VC4ClipWindow               *pVC4ClipWindow = NULL;

// Code: 103
static VC4ViewportOffset           *pVC4ViewportOffset = NULL;

// Code: 104
static VC4ZClippingPlanes          *pVC4ZClippingPlanes = NULL;

// Code: 105
static VC4ClipperXYScaling         *pVC4ClipperXYScaling = NULL;

// Code: 106
static VC4ClipperZScaleAndOffset   *pVC4ClipperZScaleAndOffset = NULL;

// Code: 112,   Binning only
static VC4TileBinningModeConfig    *pVC4TileBinningModeConfig = NULL;

// Code: 113,   Rendering only
static VC4TileRenderingModeConfig  *pVC4TileRenderingModeConfig = NULL;

// Code: 114,   Rendering only
static VC4ClearColors              *pVC4ClearColors = NULL;

// Code: 115,   Rendering only
static VC4TileCoordinates          *pVC4TileCoordinates = NULL;

#endif

#if USE_SIMPENROSE

extern "C"
{

void * __cdecl malloc(
    _In_ size_t _Size)
{
    UNREFERENCED_PARAMETER(_Size);

    return NULL;
}

//
// TODO[indyz]: Add support for print out from SimPenrose
//

int __cdecl printf(
    _In_z_ _Printf_format_string_ char const* const _Format,
    ...)
{
    UNREFERENCED_PARAMETER(_Format);

    return 0;
}

int __cdecl fprintf(
    _Inout_                       FILE*       const _Stream,
    _In_z_ _Printf_format_string_ char const* const _Format,
    ...)
{
    UNREFERENCED_PARAMETER(_Stream);
    UNREFERENCED_PARAMETER(_Format);

    return 0;
}

FILE * __cdecl __acrt_iob_func(unsigned)
{
    return NULL;
}

}

#endif // USE_SIMPENROSE

