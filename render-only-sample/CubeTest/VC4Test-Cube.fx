//--------------------------------------------------------------------------------------
// File: Tutorial02.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

Texture2D Texture;

SamplerState Sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PSInput VS( float4 Pos : POSITION, float3 TexCoord : TEXCOORD )
{
    PSInput output;
    output.pos = Pos;
    output.texcoord = TexCoord;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PSInput input ) : SV_Target
{
    return Texture.Sample(Sampler, input.texcoord);
}
