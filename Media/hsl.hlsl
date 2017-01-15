
cbuffer cbVS : register(b0)
{
    matrix g_mWorldViewProjection : packoffset(c0);
}

cbuffer cbPS : register(b1)
{
    float4 offset : packoffset(c0);
    float4 threshold : packoffset(c1);
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

//-----------------------------------------------------------------------------

float4 RGB2HSL(float4 rgb)
{
    float4 p = (rgb.g < rgb.b) ? float4(rgb.bg, -1.f, 2.f / 3.f) : float4(rgb.gb, 0.f, -1.f / 3.f);
    float4 q = (rgb.r < p.x) ? float4(p.xyw, rgb.r) : float4(rgb.r, p.yzx);
    float c = q.x - min(q.w, q.y);
    float4 hsl;
    hsl.a = rgb.a;
    hsl.z = q.x - c * .5f;
    hsl.y = c / (1.f - abs(hsl.z * 2.f - 1.f) + 1e-10);
    hsl.x = abs((q.w - q.y) / (6.f * c + 1e-10) + q.z);
    return hsl;
}

float4 HSL2RGB(float4 hsl)
{
    float R = abs(hsl.x * 6.f - 3.f) - 1.f;
    float G = 2.f - abs(hsl.x * 6.f - 2.f);
    float B = 2.f - abs(hsl.x * 6.f - 4.f);
    float cc = (1.f - abs(2.f * hsl.z - 1.f)) * hsl.y;
    float3 d = saturate(float3(R, G, B)) - .5f;
    return float4(d * cc + hsl.z, hsl.a);
}

float4 RenderScenePS(VS_OUTPUT In) : SV_TARGET
{
    bool valid;
    float blend = 1.f, t;
    float4 c = g_txDiffuse.Sample(g_samLinear, In.TextureUV);
    /** LIGHTNESS: all colors **/
    if (threshold.x < 0.f)
    {
        if (offset.z <= 0.f)
            c.rgb *= (1.f + offset.z);
        else
            c.rgb = c.rgb + offset.z * (1.f - c.rgb);
        c = saturate(c);
    }
    float4 hsl = RGB2HSL(c), curoff = offset;

    /* is current pixel inside the specified range? */
    if (threshold.x < threshold.w)
        valid = (hsl.x > threshold.x && hsl.x < threshold.w);
    else
        valid = (hsl.x > threshold.x || hsl.x < threshold.w);
    if (!valid)
        return c;
    /* blend factor */
    if (hsl.x > threshold.x && hsl.x < threshold.y)
        blend = (hsl.x - threshold.x) * 12;
    else if (hsl.x > threshold.z && hsl.x < threshold.w)
        blend = (threshold.w - hsl.x) * 12;

    // float3 b = float3(step(threshold.x, threshold.w), step(threshold.x, hsl.x), step(hsl.x, threshold.w));
    // float4 d = float4(step(threshold.x, hsl.x), step(hsl.x, threshold.y), step(threshold.z, hsl.x), step(hsl.x, threshold.w));
    // float3 p = float3(all(d.xy), all(d.zw), any(!(d.xy)) && any(!(d.zw)));
    // float3 q = float3((hsl.x - threshold.x) * 12.f, (threshold.w - hsl.x) * 12.f, 1.f);
    // blend = float(all(b) || (!b.x && any(b.yz)));
    // blend *= dot(p, q);

    curoff.xz *= blend;
    /* 1. inverse proportional */
    if (curoff.y > 0.f)
    {
        //t = (1.f - curoff.y); // 1. linear
        t = (2.f * curoff.y - 2.f) / (curoff.y - 2.f); // 2. inverse proportional
        curoff.y *= (blend * (t + 1.f) / (blend + t));
    }
    /* 2. exponential *
    if (curoff.y > 0.f) curoff.y *= (1.f - pow(abs(1.f - blend), .7f / (1.f - curoff.y)));
    /**/
    else
        curoff.y *= blend;
    
    /** LIGHTNESS: specified color range **/
    if (threshold.x > 0.f)
    {
        if (curoff.z < 0.f)
            c.rgb = c.rgb + curoff.z * (c.rgb - min(min(c.r, c.g), c.b));
        else
            c.rgb = c.rgb + curoff.z * (max(max(c.r, c.g), c.b) - c.rgb);
        c = saturate(c);
        hsl = RGB2HSL(c);
    }

    /** HUE **/
    hsl.x += curoff.x;
    if (hsl.x < 0.f)
        hsl.x += 1.f;
    else if (hsl.x > 1.f)
        hsl.x -= 1.f;

    /** SATURATION **/
    if (curoff.y <= 0.f)
        hsl.y *= (1.f + curoff.y);
    /* 1. linear multiply *
    else hsl.y *= (1.f + (1.f - hsl.y) / hsl.y * curoff.y);
    /* 2. linear increment *
    else hsl.y = saturate(hsl.y + curoff.y);
    /* 3. polynomial(cubic) morph to linear */
    else
        hsl.y = saturate(hsl.y + curoff.y * saturate(hsl.y + curoff.y) * saturate(hsl.y + curoff.y));
    /* 4. polynomial(cubic) with exponential offset *
    else if (curoff.y > 0.f && hsl.y > 0.f) {
        hsl.y *= (8.88 * curoff.y * curoff.y * curoff.y - 4.357 * curoff.y * curoff.y + 1.93 * curoff.y + 1);
        hsl.y += (1.706e-15f * exp(33.96 * curoff.y));
    }
    hsl.y = saturate(hsl.y);
    /**/

    return HSL2RGB(hsl);
}
