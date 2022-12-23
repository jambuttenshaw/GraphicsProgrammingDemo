#include "lighting.hlsli"


// textures
Texture2D texture2DBuffer[TEX_BUFFER_SIZE] : register(t0);
TextureCube textureCubeBuffer[TEX_BUFFER_SIZE] : register(t16);

SamplerState bilinearSampler : register(s0);
SamplerState trilinearSampler : register(s1);
SamplerState anisotropicSampler : register(s2);
SamplerComparisonState shadowSampler : register(s3);

// lighting
cbuffer LightCB : register(b0)
{
    const PSLightBuffer lightBuffer;
};

// materials
cbuffer MaterialCB : register(b1)
{
    const MaterialBuffer materialBuffer;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 worldPos : POSITION0;
	float3 viewDir : POSITION1;
    float4 lightViewPos[MAX_LIGHTS] : POSITION2;
}; 


float4 main(InputType input) : SV_TARGET
{   
	float3 n = normalize(input.normal);
	float3 v = -normalize(input.viewDir);
    
    float3 color = calculateLighting(input.worldPos, input.lightViewPos, n, v, input.tex,
                                     materialBuffer.material,
                                     lightBuffer,
                                     texture2DBuffer, textureCubeBuffer,
                                     bilinearSampler, trilinearSampler, anisotropicSampler, shadowSampler);
    return float4(color, 1.0f);
}
