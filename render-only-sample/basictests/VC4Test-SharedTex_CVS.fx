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
PSInput VS( float2 Pos : POSITION, float2 TexCoord : TEXCOORD )
{
    PSInput output;

    output.pos = float4(Pos, 1.0, 1.0);
    output.color = float3(TexCoord, 1.0);
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PSInput input ) : SV_Target
{
    return float4(input.color, 1.0f); // vertex color
}
