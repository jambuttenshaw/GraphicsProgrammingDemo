#include "lighting.hlsli"


// textures
Texture2D texture2DBuffer[TEX_BUFFER_SIZE] : register(t0);
TextureCube textureCubeBuffer[TEX_BUFFER_SIZE] : register(t8);

SamplerComparisonState shadowSampler : register(s0);

SamplerState irradianceMapSampler : register(s1);
SamplerState brdfIntegrationSampler : register(s2);
SamplerState materialSampler : register(s3);

// lighting
cbuffer LightBuffer : register(b0)
{
    LightBuffer lighting;
};

// materials
cbuffer MaterialBuffer : register(b1)
{
    const MaterialData materialData;
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
                                     materialData,
                                     lighting,
                                     texture2DBuffer, textureCubeBuffer,
                                     materialSampler, shadowSampler, irradianceMapSampler, brdfIntegrationSampler);
    return float4(color, 1.0f);
}
