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
PSInput VS( float4 Pos : POSITION )
{
    PSInput output;
    output.pos = Pos;
    output.color = float3(1.0, 1.0, 10);
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PSInput input ) : SV_Target
{
    return float4(input.color, 1.0f); // vertex color
}
