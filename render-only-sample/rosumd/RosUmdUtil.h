////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utility
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#if VC4

#include "Vc4Hw.h"

#endif

template<typename InputType>
FORCEINLINE
void
AlignValue(
    InputType &Value,
    UINT Alignment
    )
{
    Value = InputType((SIZE_T(Value) + (Alignment - 1)) & ~SIZE_T(Alignment - 1));
}

UINT
ConvertFloatColor(
    DXGI_FORMAT Format,
    FLOAT * pColor);

#if VC4

VC4PrimitiveMode
ConvertD3D11Topology(
    D3D10_DDI_PRIMITIVE_TOPOLOGY    topology);

VC4DepthTestFunc
ConvertD3D11DepthComparisonFunc(
    D3D10_DDI_COMPARISON_FUNC   comparisonFunc);

VC4TextureWrap
ConvertD3D11TextureAddressMode(
    D3D10_DDI_TEXTURE_ADDRESS_MODE  texAddressMode);

VC4TextureMagFilter
ConvertD3D11TextureMagFilter(
    D3D10_DDI_FILTER    texFilter);

VC4TextureMinFilter
ConvertD3D11TextureMinFilter(
    D3D10_DDI_FILTER    texFilter,
    BOOLEAN             bClampToLOD0);

#endif

inline bool operator== (const D3D10DDI_MIPINFO &Lhs, const D3D10DDI_MIPINFO &Rhs)
{
    return (Lhs.TexelWidth == Rhs.TexelWidth) &&
        (Lhs.TexelHeight == Rhs.TexelHeight) &&
        (Lhs.TexelDepth == Rhs.TexelDepth) &&
        (Lhs.PhysicalWidth == Rhs.PhysicalWidth) &&
        (Lhs.PhysicalHeight == Rhs.PhysicalHeight) &&
        (Lhs.PhysicalDepth == Rhs.PhysicalDepth);
}

inline bool operator== (const DXGI_SAMPLE_DESC &Lhs, const DXGI_SAMPLE_DESC &Rhs)
{
    return (Lhs.Count == Rhs.Count) && (Lhs.Quality == Rhs.Quality);
}

inline bool operator== (const DXGI_DDI_RATIONAL &Lhs, const DXGI_DDI_RATIONAL &Rhs)
{
    return (Lhs.Numerator == Rhs.Numerator) && 
        (Lhs.Denominator == Rhs.Denominator);
}

inline bool operator== (const DXGI_DDI_MODE_DESC &Lhs, const DXGI_DDI_MODE_DESC &Rhs)
{
    return (Lhs.Width == Rhs.Width) &&
        (Lhs.Height == Rhs.Height) &&
        (Lhs.Format == Rhs.Format) &&
        (Lhs.RefreshRate == Rhs.RefreshRate) &&
        (Lhs.ScanlineOrdering == Rhs.ScanlineOrdering) &&
        (Lhs.Rotation == Rhs.Rotation) &&
        (Lhs.Scaling == Rhs.Scaling);
}

inline bool operator== (const DXGI_DDI_PRIMARY_DESC &Lhs, const DXGI_DDI_PRIMARY_DESC &Rhs)
{
    return (Lhs.Flags == Rhs.Flags) &&
        (Lhs.VidPnSourceId == Rhs.VidPnSourceId) &&
        (Lhs.ModeDesc == Rhs.ModeDesc) &&
        (Lhs.DriverFlags == Rhs.DriverFlags);
}
