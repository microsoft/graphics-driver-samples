////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utility implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

#if VC4

//
// Make sure PDB file has type definition for VC4 Control List commands
//

// Code: 28
static VC4StoreTileBufferGeneral   *pVC4StoreTileBufferGeneral = NULL;

// Code: 32
static VC4IndexedPrimitiveList     *pVC4IndexedPrimitiveList = NULL;

// Code: 56
static VC4PrimitiveListFormat      *pVC4PrimitiveListFormat = NULL;

// Code: 64
static VC4GLShaderState            *pVC4GLShaderState = NULL;

static VC4GLShaderStateRecord      *pVC4GLShaderStateRecord = NULL;

static VC4VertexAttribute          *pVC4VertexAttribute = NULL;

// Code: 65
static VC4NVShaderState            *pVC4NVShaderState = NULL;

static VC4NVShaderStateRecord      *pVC4NVShaderStateRecord = NULL;

// Code: 96
static VC4ConfigBits               *pVC4ConfigBits = NULL;

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

