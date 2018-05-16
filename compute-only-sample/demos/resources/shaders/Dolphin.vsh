//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//--------------------------------------------------------------------------------------
// Shaders for the Dolphin sample
//--------------------------------------------------------------------------------------

struct VSOUT
{
    float4 vPosition    : SV_POSITION;
    float4 vTexCoords   : TEXCOORD1;            // TEXCOORD0.xy = basetex, TEXCOORD0.zw = caustictex
    float4 vLightAndFog : COLOR0_center;        // COLOR0.x = light, COLOR0.y = fog
};


//--------------------------------------------------------------------------------------
// Vertex shader constants
//--------------------------------------------------------------------------------------

cbuffer cb0 : register(b0)
{
    uniform float4   g_vZero             : packoffset(c0);  // ( 0, 0, 0, 0 )
    uniform float4   g_vConstants        : packoffset(c1);  // ( 1, 0.5, -, - )
    uniform float3   g_vBlendWeights     : packoffset(c2);  // ( fWeight1, fWeight2, fWeight3, 0 )
    uniform float4x4 g_matWorldViewProj  : packoffset(c3);  // world-view-projection matrix
    uniform float4x4 g_matWorldView      : packoffset(c7);  // world-view matrix
    uniform float4x4 g_matView           : packoffset(c11); // view matrix
    uniform float4x4 g_matProjection     : packoffset(c15); // projection matrix
    uniform float3   g_vSeafloorLightDir : packoffset(c19); // seafloor light direction
    uniform float3   g_vDolphinLightDir  : packoffset(c20); // dolphin light direction
    uniform float4   g_vDiffuse          : packoffset(c21);
    uniform float4   g_vAmbient          : packoffset(c22);
    uniform float4   g_vFogRange         : packoffset(c23); // ( x, fog_end, (1/(fog_end-fog_start)), x)
    uniform float4   g_vTexGen           : packoffset(c24);
}

//--------------------------------------------------------------------------------------
// Name: ShadeSeaFloorVertex()
// Desc: Vertex shader for the seafloor
//--------------------------------------------------------------------------------------
VSOUT ShadeSeaFloorVertex( const float3 vPosition      : POSITION,
                           const float3 vNormal        : NORMAL,
                           const float2 vBaseTexCoords : TEXCOORD0 )
{
    // Transform to view space (world matrix is identity)
    float4 vViewPosition = mul( float4(vPosition, 1.0f), g_matView );

    // Transform to projection space
    float4 vOutputPosition = mul( vViewPosition, g_matProjection );

    // Lighting calculation
    float fLightValue = max( dot( vNormal, g_vSeafloorLightDir ), g_vZero.x );

    // Generate water caustic tex coords from vertex xz position
    float2 vCausticTexCoords = g_vTexGen.xx * vViewPosition.xz + g_vTexGen.zw;
    
    // Fog calculation:
    float fFogValue = clamp( (g_vFogRange.y - vViewPosition.z) * g_vFogRange.z, g_vZero.x, g_vConstants.x );

    // Compress output values
    VSOUT  Output;
    Output.vPosition      = vOutputPosition;
    Output.vLightAndFog.x = fLightValue;
    Output.vLightAndFog.y = fFogValue;
    Output.vTexCoords.xy  = 10*vBaseTexCoords;
    Output.vTexCoords.zw  = vCausticTexCoords;
    
    Output.vLightAndFog.z = 0.0f;
    Output.vLightAndFog.w = 0.0f;
    
    return Output;
}


//--------------------------------------------------------------------------------------
// Name: ShadeDolphinVertex()
// Desc: Vertex shader for the dolphin
//--------------------------------------------------------------------------------------
VSOUT ShadeDolphinVertex( const float3 vPosition0     : POSITION0,
                          const float3 vPosition1     : POSITION1,
                          const float3 vPosition2     : POSITION2,
// #if VC4_HACK
//                        const float3 vNormal0       : NORMAL0,
//                        const float3 vNormal1       : NORMAL1,
//                        const float3 vNormal2       : NORMAL2,
// #else
                          const float3 vNormal4       : NORMAL4,
// #endif // VC4_HACK
                          const float2 vBaseTexCoords : TEXCOORD0)
{
    // Tween the 3 positions (v0,v1,v2) into one position
    float4 vModelPosition = float4( vPosition0 * g_vBlendWeights.x + vPosition1 * g_vBlendWeights.y + vPosition2 * g_vBlendWeights.z, 1.0f );

    // Transform position to the clipping space
    float4 vOutputPosition = mul( vModelPosition, g_matWorldViewProj );
    
    // Transform position to the camera space
    float4 vViewPosition = mul( vModelPosition, g_matWorldView );

    // Tween the 3 normals (v3,v4,v5) into one normal

// #if VC4_HACK
//  float3 vModelNormal = vNormal0 * g_vBlendWeights.x + vNormal1 * g_vBlendWeights.y + vNormal2 * g_vBlendWeights.z;
// #else
    float3 vModelNormal = vNormal4;
// #endif // VC4_HACK

    // Do the lighting calculation
    float fLightValue = max( dot( vModelNormal, g_vDolphinLightDir ), g_vZero.x );

    // Generate water caustic tex coords from vertex xz position
    float2 vCausticTexCoords = g_vConstants.yy * vViewPosition.xz;

    // Fog calculation:
    float fFogValue = clamp( (g_vFogRange.y - vViewPosition.z) * g_vFogRange.z, g_vZero.x, g_vConstants.x );

    // Compress output values
    VSOUT  Output;
    Output.vPosition      = vOutputPosition;
    Output.vLightAndFog.x = fLightValue;
    Output.vLightAndFog.y = fFogValue;
    Output.vTexCoords.xy  = vBaseTexCoords;
    Output.vTexCoords.zw  = vCausticTexCoords;

    Output.vLightAndFog.z = 0.0f;
    Output.vLightAndFog.w = 1.0f;

    return Output;
}


