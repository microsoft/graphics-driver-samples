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

#endif

