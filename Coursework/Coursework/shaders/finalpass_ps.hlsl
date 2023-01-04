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
	float lumWhite;
	float middleGrey;

	// bloom
    int enableBloom;
    float bloomStrength;
	
    float2 padding;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD;
	float3 viewVector : POSITION0;
};

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
	    
        color.rgb *= middleGrey;
        color.rgb *= (1.0f + color / lumWhite);
        color.rgb /= (1.0f + color);
    }

	return float4(color, 1.0f);
}
