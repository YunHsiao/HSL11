//--------------------------------------------------------------------------------------
// File: hsl.hlsl
//
// The HLSL file for the HSL11 project for the Direct3D 11 device
// 
//--------------------------------------------------------------------------------------

cbuffer cbPerObject : register( b0 ) {
    matrix  g_mWorldViewProjection  : packoffset( c0 );
    matrix  g_mWorld                : packoffset( c4 );
}

Texture2D    g_txDiffuse : register( t0 );
SamplerState g_samLinear : register( s0 );

struct VS_INPUT
{
    float3 Position     : POSITION; // vertex position 
    float2 TextureUV    : TEXCOORD0;// vertex texture coords 
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION; // vertex position 
    float2 TextureUV    : TEXCOORD0;   // vertex texture coords 
};

VS_OUTPUT RenderSceneVS( VS_INPUT input )
{
    VS_OUTPUT Output;
    Output.Position = mul( float4(input.Position, 1.f), g_mWorldViewProjection );
    Output.TextureUV = input.TextureUV;
    return Output;
}

float4 RenderScenePS( VS_OUTPUT In ) : SV_TARGET
{ 
    return g_txDiffuse.Sample( g_samLinear, In.TextureUV );
}
