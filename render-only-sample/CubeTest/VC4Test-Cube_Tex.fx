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

cbuffer VSConstantBuffer : register(b0)
{
    uniform matrix World;
    uniform matrix View;
    uniform matrix Projection;
}

cbuffer PSConstantBuffer : register(b0)
{
    uniform float3 Ambient;
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
    output.pos = mul(Pos, World);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.texcoord.xy = TexCoord.xy;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PSInput input) : SV_Target
{
    float3 color = Texture.Sample(Sampler, input.texcoord);
    return float4(color * Ambient, 1.0f);
}