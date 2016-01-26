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

inline operator== (const D3D10DDI_MIPINFO &Lhs, const D3D10DDI_MIPINFO &Rhs)
{
    return memcmp(&Lhs, &Rhs, sizeof(Lhs)) == 0;
}

inline operator== (const DXGI_SAMPLE_DESC &Lhs, const DXGI_SAMPLE_DESC &Rhs)
{
    return memcmp(&Lhs, &Rhs, sizeof(Lhs)) == 0;
}

inline operator== (const DXGI_DDI_PRIMARY_DESC &Lhs, const DXGI_DDI_PRIMARY_DESC &Rhs)
{
    return memcmp(&Lhs, &Rhs, sizeof(Lhs)) == 0;
}
