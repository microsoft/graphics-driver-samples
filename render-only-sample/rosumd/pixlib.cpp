
#include "precomp.h"

#include "RosUmdLogging.h"
#include "pixlib.tmh"

#include "pixel.cpp"

// Bytes per "pixel" table index by DXGI_FORMAT
// BCn (Block Compression) texture's size is negative bytes for the block
// FOURCC format's byte size is for 2x2 blocks
static UINT s_BytesPerPixel[] =
{
    1,          // DXGI_FORMAT_UNKNOWN
    16,         // DXGI_FORMAT_R32G32B32A32_TYPELESS
    16,         // DXGI_FORMAT_R32G32B32A32_FLOAT
    16,         // DXGI_FORMAT_R32G32B32A32_UINT
    16,         // DXGI_FORMAT_R32G32B32A32_SINT
    12,         // DXGI_FORMAT_R32G32B32_TYPELESS
    12,         // DXGI_FORMAT_R32G32B32_FLOAT
    12,         // DXGI_FORMAT_R32G32B32_UINT
    12,         // DXGI_FORMAT_R32G32B32_SINT
    8,          // DXGI_FORMAT_R16G16B16A16_TYPELESS
    8,          // DXGI_FORMAT_R16G16B16A16_FLOAT
    8,          // DXGI_FORMAT_R16G16B16A16_UNORM
    8,          // DXGI_FORMAT_R16G16B16A16_UINT
    8,          // DXGI_FORMAT_R16G16B16A16_SNORM
    8,          // DXGI_FORMAT_R16G16B16A16_SINT
    8,          // DXGI_FORMAT_R32G32_TYPELESS
    8,          // DXGI_FORMAT_R32G32_FLOAT
    8,          // DXGI_FORMAT_R32G32_UINT
    8,          // DXGI_FORMAT_R32G32_SINT
    8,          // DXGI_FORMAT_R32G8X24_TYPELESS
    8,          // DXGI_FORMAT_D32_FLOAT_S8X24_UINT
    8,          // DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS
    8,          // DXGI_FORMAT_X32_TYPELESS_G8X24_UINT
    4,          // DXGI_FORMAT_R10G10B10A2_TYPELESS
    4,          // DXGI_FORMAT_R10G10B10A2_UNORM
    4,          // DXGI_FORMAT_R10G10B10A2_UINT
    4,          // DXGI_FORMAT_R11G11B10_FLOAT
    4,          // DXGI_FORMAT_R8G8B8A8_TYPELESS
    4,          // DXGI_FORMAT_R8G8B8A8_UNORM
    4,          // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
    4,          // DXGI_FORMAT_R8G8B8A8_UINT
    4,          // DXGI_FORMAT_R8G8B8A8_SNORM
    4,          // DXGI_FORMAT_R8G8B8A8_SINT
    4,          // DXGI_FORMAT_R16G16_TYPELESS
    4,          // DXGI_FORMAT_R16G16_FLOAT
    4,          // DXGI_FORMAT_R16G16_UNORM
    4,          // DXGI_FORMAT_R16G16_UINT
    4,          // DXGI_FORMAT_R16G16_SNORM
    4,          // DXGI_FORMAT_R16G16_SINT
    4,          // DXGI_FORMAT_R32_TYPELESS
    4,          // DXGI_FORMAT_D32_FLOAT
    4,          // DXGI_FORMAT_R32_FLOAT
    4,          // DXGI_FORMAT_R32_UINT
    4,          // DXGI_FORMAT_R32_SINT
    4,          // DXGI_FORMAT_R24G8_TYPELESS
    4,          // DXGI_FORMAT_D24_UNORM_S8_UINT
    4,          // DXGI_FORMAT_R24_UNORM_X8_TYPELESS
    4,          // DXGI_FORMAT_X24_TYPELESS_G8_UINT
    2,          // DXGI_FORMAT_R8G8_TYPELESS
    2,          // DXGI_FORMAT_R8G8_UNORM
    2,          // DXGI_FORMAT_R8G8_UINT
    2,          // DXGI_FORMAT_R8G8_SNORM
    2,          // DXGI_FORMAT_R8G8_SINT
    2,          // DXGI_FORMAT_R16_TYPELESS
    2,          // DXGI_FORMAT_R16_FLOAT
    2,          // DXGI_FORMAT_D16_UNORM
    2,          // DXGI_FORMAT_R16_UNORM
    2,          // DXGI_FORMAT_R16_UINT
    2,          // DXGI_FORMAT_R16_SNORM
    2,          // DXGI_FORMAT_R16_SINT
    1,          // DXGI_FORMAT_R8_TYPELESS
    1,          // DXGI_FORMAT_R8_UNORM
    1,          // DXGI_FORMAT_R8_UINT
    1,          // DXGI_FORMAT_R8_SNORM
    1,          // DXGI_FORMAT_R8_SINT
    1,          // DXGI_FORMAT_A8_UNORM
    1,          // DXGI_FORMAT_R1_UNORM. special cased
    4,          // DXGI_FORMAT_R9G9B9E5_SHAREDEXP
    2,          // DXGI_FORMAT_R8G8_B8G8_UNORM
    2,          // DXGI_FORMAT_G8R8_G8B8_UNORM
    (UINT)-8,   // DXGI_FORMAT_BC1_TYPELESS
    (UINT)-8,   // DXGI_FORMAT_BC1_UNORM
    (UINT)-8,   // DXGI_FORMAT_BC1_UNORM_SRGB
    (UINT)-16,  // DXGI_FORMAT_BC2_TYPELESS
    (UINT)-16,  // DXGI_FORMAT_BC2_UNORM
    (UINT)-16,  // DXGI_FORMAT_BC2_UNORM_SRGB
    (UINT)-16,  // DXGI_FORMAT_BC3_TYPELESS
    (UINT)-16,  // DXGI_FORMAT_BC3_UNORM
    (UINT)-16,  // DXGI_FORMAT_BC3_UNORM_SRGB
    (UINT)-8,   // DXGI_FORMAT_BC4_TYPELESS
    (UINT)-8,   // DXGI_FORMAT_BC4_UNORM
    (UINT)-8,   // DXGI_FORMAT_BC4_SNORM
    (UINT)-16,  // DXGI_FORMAT_BC5_TYPELESS
    (UINT)-16,  // DXGI_FORMAT_BC5_UNORM
    (UINT)-16,  // DXGI_FORMAT_BC5_SNORM
    2,          // DXGI_FORMAT_B5G6R5_UNORM
    2,          // DXGI_FORMAT_B5G5R5A1_UNORM
    4,          // DXGI_FORMAT_B8G8R8A8_UNORM
    4,          // DXGI_FORMAT_B8G8R8X8_UNORM
    4,          // DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
    4,          // DXGI_FORMAT_B8G8R8A8_TYPELESS
    4,          // DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
    4,          // DXGI_FORMAT_B8G8R8X8_TYPELESS
    4,          // DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
    (UINT)-16,  // DXGI_FORMAT_BC6H_TYPELESS
    (UINT)-16,  // DXGI_FORMAT_BC6H_UF16
    (UINT)-16,  // DXGI_FORMAT_BC6H_SF16
    (UINT)-16,  // DXGI_FORMAT_BC7_TYPELESS
    (UINT)-16,  // DXGI_FORMAT_BC7_UNORM
    (UINT)-16,  // DXGI_FORMAT_BC7_UNORM_SRGB
    4*4,        // DXGI_FORMAT_AYUV
    4*4,        // DXGI_FORMAT_Y410
    4*6,        // DXGI_FORMAT_Y416
    4+2,        // DXGI_FORMAT_NV12
    8+4,        // DXGI_FORMAT_P010
    8+4,        // DXGI_FORMAT_P016
    4+2,        // DXGI_FORMAT_420_OPAQUE
    4+4,        // DXGI_FORMAT_YUY2
    8+8,        // DXGI_FORMAT_Y210
    8+8,        // DXGI_FORMAT_Y216
    4+2,        // DXGI_FORMAT_NV11
    4,          // DXGI_FORMAT_AI44
    4,          // DXGI_FORMAT_IA44
    1,          // DXGI_FORMAT_P8
    2,          // DXGI_FORMAT_A8P8
    2,          // DXGI_FORMAT_B4G4R4A4_UNORM
    4+2,        // DXGI_FORMAT_P208
    4+2,        // DXGI_FORMAT_V208
    4+4         // DXGI_FORMAT_V408
};

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::BytesPerPixel"

UINT CPixel::BytesPerPixel(DXGI_FORMAT Format)
{
    return s_BytesPerPixel[Format];
}; // BytesPerPixel

