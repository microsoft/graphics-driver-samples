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

cbuffer cb0 : register(b0)
{
    uniform float3 g_vAmbient;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PSInput VS(float4 Pos : POSITION, float2 TexCoord : TEXCOORD)
{
    PSInput output;
    output.pos = Pos;
    output.texcoord.xy = TexCoord.xy;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PSInput input) : SV_Target
{
    float3 color = Texture.Sample(Sampler, input.texcoord);
    return float4(color * g_vAmbient, 1.0f);
}
