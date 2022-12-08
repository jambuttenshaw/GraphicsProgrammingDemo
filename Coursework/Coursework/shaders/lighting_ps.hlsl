#include "lighting.hlsli"


// shadows
Texture2D shadowMap0 : register(t0);
Texture2D shadowMap1 : register(t1);
Texture2D shadowMap2 : register(t2);
Texture2D shadowMap3 : register(t3);

SamplerState shadowSampler : register(s0);

// ibl
TextureCube irradianceMap : register(t4);
TextureCube prefilterMap : register(t5);
Texture2D brdfIntegrationMap : register(t6);

SamplerState irradianceMapSampler : register(s1);
SamplerState brdfIntegrationSampler : register(s2);

// material
Texture2D albedoMap : register(t7);
Texture2D roughnessMap : register(t8);
Texture2D normalMap : register(t9);

SamplerState materialSampler : register(s3);

// lighting
cbuffer LightBuffer : register(b0)
{
    LightBuffer lighting;
};

// materials
cbuffer MaterialBuffer : register(b1)
{
    MaterialData materialData;
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
    Texture2D shadowMaps[4];
    shadowMaps[0] = shadowMap0;
    shadowMaps[1] = shadowMap1;
    shadowMaps[2] = shadowMap2;
    shadowMaps[3] = shadowMap3;
    
	float3 n = normalize(input.normal);
	float3 v = -normalize(input.viewDir);
    
    float3 color = calculateLighting(input.worldPos, input.lightViewPos, n, v, input.tex, materialData, lighting,
                                     albedoMap, normalMap, roughnessMap, materialSampler, shadowMaps, shadowSampler,
                                     irradianceMap, prefilterMap, brdfIntegrationMap, irradianceMapSampler, brdfIntegrationSampler);
    return float4(color, 1.0f);
}
