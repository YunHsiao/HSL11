
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
static const float Pi = 3.14159265f;
static const float P2O1 = 2 * Pi;
static const float P1O3 = Pi / 3;
static const float P2O3 = 2 * Pi / 3;
static const float P4O3 = 4 * Pi / 3;
texture g_MeshTexture;              // Color texture for mesh
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix
float4 offset;
float4 threshold;

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float2 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;
    
    Output.Position = mul(vPos, g_mWorldViewProjection);
    Output.TextureUV = vTexCoord0; 
    
    return Output;
}

/**
float4 RGB2HSL(float4 c) {
	float4 hsl; hsl.w = 0.f;
    float cmax = max(max(c.r, c.g), c.b), cmin = min(min(c.r, c.g), c.b);
    hsl.z = (cmax + cmin) / 2;
	if (cmax == cmin) { hsl.xy = 0.f; return hsl; }
	float delta = cmax - cmin;
	hsl.y = hsl.z > .5f ? delta / (2 - cmax - cmin) : delta / (cmax + cmin);
	if (cmax == c.r) hsl.x = (c.g - c.b) / delta + (c.g < c.b ? 6 : 0);
	else if (cmax == c.g) hsl.x = (c.b - c.r) / delta + 2.f;
	else hsl.x = (c.r - c.g) / delta + 4.f;
	hsl.x /= 6.f;
	return hsl;
}

float4 HSL2RGB(float4 hsl) {
	float4 c; c.w = 0.f;
    float cc = (1 - abs(2 * hsl.z - 1)) * hsl.y, h1 = hsl.x * 360;
    float x = cc * (1 - abs(((hsl.x * 6) % 2) - 1)), m = hsl.z - cc / 2;
    if (h1 < 60) c.rgb = float3(cc, x, 0);
    else if (h1 < 120) c.rgb = float3(x, cc, 0);
    else if (h1 < 180) c.rgb = float3(0, cc, x);
    else if (h1 < 240) c.rgb = float3(0, x, cc);
    else if (h1 < 300) c.rgb = float3(x, 0, cc);
    else c.rgb = float3(cc, 0, x);
    c.rgb += m;
	return c;
}
/**/

float e = 1e-3;
float3 RGB2HCV(float3 rgb) {
    float4 p = (rgb.g < rgb.b) ? float4(rgb.bg, -1.0, 2.0 / 3.0) : float4(rgb.gb, 0.0, -1.0 / 3.0);
    float4 q = (rgb.r < p.x) ? float4(p.xyw, rgb.r) : float4(rgb.r, p.yzx);
    float c = q.x - min(q.w, q.y);
    float h = abs((q.w - q.y) / (6.0 * c + e) + q.z);
    return float3(h, c, q.x);
}

float4 RGB2HSL(float4 c) {
    float4 hsl; hsl.a = c.a;
    float3 hcv = RGB2HCV(c.rgb);
    hsl.z = hcv.z - hcv.y * 0.5;
    hsl.y = hcv.y / (1.0 - abs(hsl.z * 2.0 - 1.0) + e);
    hsl.x = hcv.x;
    return hsl;
}

float3 HUE2RGB(float h) {
    float r = abs(h * 6.0 - 3.0) - 1.0;
    float g = 2.0 - abs(h * 6.0 - 2.0);
    float b = 2.0 - abs(h * 6.0 - 4.0);
    return saturate(float3(r, g, b));
}

float4 HSL2RGB(float4 hsl) {
    float4 c; c.a = hsl.a;
    float cc = (1.0 - abs(2.0 * hsl.z - 1.0)) * hsl.y;
    c.rgb = (HUE2RGB(hsl.x) - 0.5) * cc + hsl.z;
    return c;
}

//--------------------------------------------------------------------------------------
// Calculate result with user-defined offset in HSI space
//--------------------------------------------------------------------------------------
float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
    bool valid;
    float blend = 1.f, t;
	float4 c = tex2D(MeshTextureSampler, In.TextureUV);
	/** LIGHTNESS: all colors **/
	if (threshold.x < 0.f) {
		if (offset.z <= 0.f) c.rgb *= (1.f + offset.z);
		else c.rgb = c.rgb + offset.z * (1.f - c.rgb);
		c = saturate(c);
	}
    float4 hsl = RGB2HSL(c), curoff = offset;

	/* is current pixel inside the specified range? */
    if (threshold.x < threshold.w) valid = (hsl.x > threshold.x && hsl.x < threshold.w);
    else valid = (hsl.x > threshold.x || hsl.x < threshold.w);
    if (!valid) return c;

	/* blend factor */
    if (hsl.x > threshold.x && hsl.x < threshold.y) blend = (hsl.x - threshold.x) * 12;
    else if (hsl.x > threshold.z && hsl.x < threshold.w) blend = (threshold.w - hsl.x) * 12;
    curoff.xz *= blend;
	/* 1. inverse */
	if (curoff.y > 0.f) {
		//t = (1.f - curoff.y); // 1. linear
		t = (2.f * curoff.y - 2.f) / (curoff.y - 2.f); // 2. inverse
		curoff.y *= (blend * (t + 1.f) / (blend + t));
	}
	/* 2. exponential *
	if (curoff.y > 0.f) curoff.y *= (1.f - pow(abs(1.f - blend), .7f / (1.f - curoff.y)));
	/**/
	else curoff.y *= blend;
	
	/** LIGHTNESS: specified color range **/
	if (threshold.x > 0.f) {
		if (curoff.z < 0.f) c.rgb = c.rgb + curoff.z * (c.rgb - min(min(c.r, c.g), c.b));
		else c.rgb = c.rgb + curoff.z * (max(max(c.r, c.g), c.b) - c.rgb);
		c = saturate(c);
		hsl = RGB2HSL(c);
	}

	/** HUE **/
	hsl.x += curoff.x;
	hsl.x -= floor(hsl.x / 1.0);

	/** SATURATION **/
	if (curoff.y <= 0.f) hsl.y *= (1.f + curoff.y);
	/* 1. linear multiply *
	else hsl.y *= (1.f + (1.f - hsl.y) / hsl.y * curoff.y);
	/* 2. linear increment *
	else hsl.y = saturate(hsl.y + curoff.y);
	/* 3. linear morph to polynomial(cubic) */
	else hsl.y = saturate(hsl.y + curoff.y * saturate(hsl.y + curoff.y) * saturate(hsl.y + curoff.y));
	/* 4. polynomial(cubic) with exponential offset *
	else if (curoff.y > 0.f && hsl.y > 0.f) {
		hsl.y *= (8.88 * curoff.y * curoff.y * curoff.y - 4.357 * curoff.y * curoff.y + 1.93 * curoff.y + 1);
		hsl.y += (1.706e-15f * exp(33.96 * curoff.y));
	}
	hsl.y = saturate(hsl.y);
	/**/

    return HSL2RGB(hsl);
}


//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique RenderScene
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneVS();
        PixelShader  = compile ps_3_0 RenderScenePS(); 
    }
}
