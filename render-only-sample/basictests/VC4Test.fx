//--------------------------------------------------------------------------------------
// File: Tutorial02.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PSInput VS( float3 Pos : POSITION, float3 Color : COLOR )
{
    PSInput output;
    output.pos = float4(Pos, 1.0f);
    output.color = Color;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PSInput input ) : SV_Target
{
    return float4(input.color, 1.0f); // vertex color
}
