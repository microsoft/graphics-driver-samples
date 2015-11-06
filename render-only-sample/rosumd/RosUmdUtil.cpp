////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Device implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "d3dumddi_.h"
#include "RosUmdUtil.h"
#include <math.h>

#if VC4

#include "Vc4Hw.h"

#endif

UINT
ConvertFloatColor(
    FLOAT * pColor)
{
    BYTE    red, green, blue, alpha;

    red   = (BYTE)round(pColor[0] * 255.0);
    green = (BYTE)round(pColor[1] * 255.0);
    blue  = (BYTE)round(pColor[2] * 255.0);
    alpha = (BYTE)round(pColor[3] * 255.0);

    return (alpha << 24) | (blue << 16) | (green << 8) | red;
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

#endif

