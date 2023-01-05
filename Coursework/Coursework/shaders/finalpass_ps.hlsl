#include "common.hlsli"

Texture2D renderTextureColour : register(t0);
Texture2D renderTextureDepth : register(t1);
StructuredBuffer<float> lum : register(t2);
Texture2D bloomTex : register(t3);

SamplerState trilinearSampler;

cbuffer Params : register(b0)
{
    int enableTonemapping;
    float avgLumFactor; // avgLum = lum[0] * avgLumFactor
    
    float hdrMax; // How much HDR range before clipping. HDR modes likely need this pushed up to say 25.0.
    float contrast; // Use as a baseline to tune the amount of contrast the tonemapper has.
    float shoulder; // Likely don’t need to mess with this factor, unless matching existing tonemapper is not working well..
    float midIn; // most games will have a {0.0 to 1.0} range for LDR so midIn should be 0.18.
    float midOut; // Use for LDR. For HDR10 10:10:10:2 use maybe 0.18/25.0 to start. For scRGB, I forget what a good starting point is, need to re-calculate.
    float crosstalk; // Amount of channel crosstalk
    float white;
    
	// bloom
    int enableBloom;
    float bloomStrength;
	
    float padding;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD;
    float3 viewVector : POSITION0;
};



float ColToneB(float hdrMax, float contrast, float shoulder, float midIn, float midOut)
{
    return
        -((-pow(midIn, contrast) + (midOut * (pow(hdrMax, contrast * shoulder) * pow(midIn, contrast) -
            pow(hdrMax, contrast) * pow(midIn, contrast * shoulder) * midOut)) /
            (pow(hdrMax, contrast * shoulder) * midOut - pow(midIn, contrast * shoulder) * midOut)) /
            (pow(midIn, contrast * shoulder) * midOut));
}

// General tonemapping operator, build 'c' term.
float ColToneC(float hdrMax, float contrast, float shoulder, float midIn, float midOut)
{
    return (pow(hdrMax, contrast * shoulder) * pow(midIn, contrast) - pow(hdrMax, contrast) * pow(midIn, contrast * shoulder) * midOut) /
           (pow(hdrMax, contrast * shoulder) * midOut - pow(midIn, contrast * shoulder) * midOut);
}

// General tonemapping operator, p := {contrast,shoulder,b,c}.
float ColTone(float x, float4 p)
{
    float z = pow(x, p.r);
    return z / (pow(z, p.g) * p.b + p.a);
}

float3 TimothyTonemapper(float3 color)
{
    float b = ColToneB(hdrMax, contrast, shoulder, midIn, midOut);
    float c = ColToneC(hdrMax, contrast, shoulder, midIn, midOut);

    float peak = max(color.r, max(color.g, color.b));
    float3 ratio = color / peak;
    peak = ColTone(peak, float4(contrast, shoulder, b, c));
    // then process ratio

    // probably want send these pre-computed (so send over saturation/crossSaturation as a constant)
    float saturation = contrast; // full tonal range saturation control
    float crossSaturation = contrast * 16.0f; // crosstalk saturation

    // wrap crosstalk in transform
    ratio = pow(abs(ratio), saturation / crossSaturation);
    ratio = lerp(ratio, white, pow(peak, crosstalk));
    ratio = pow(abs(ratio), crossSaturation);

    // then apply ratio to peak
    color = peak * ratio;
    return color;
}



float4 main(InputType input) : SV_TARGET
{
    float3 color = renderTextureColour[input.position.xy].rgb;
	
    if (enableBloom)
    {
	    // apply bloom
        float3 bloom = bloomTex.SampleLevel(trilinearSampler, input.tex, 0).rgb;
        color += bloom * bloomStrength;
    }
	
    if (enableTonemapping)
    {
        
        // Tone mapping
        float fLum = lum[0] * avgLumFactor;
        color /= fLum;

        color = TimothyTonemapper(color);
    }

    return float4(color, 1.0f);
}
