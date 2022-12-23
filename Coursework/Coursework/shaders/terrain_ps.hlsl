// Terrain Pixel Shader

#include "lighting.hlsli"

// textures
Texture2D texture2DBuffer[TEX_BUFFER_SIZE] : register(t0);
TextureCube textureCubeBuffer[TEX_BUFFER_SIZE] : register(t16);

SamplerState bilinearSampler : register(s0);
SamplerState trilinearSampler : register(s1);
SamplerState anisotropicSampler : register(s2);
SamplerComparisonState shadowSampler : register(s3);
SamplerState heightmapSampler : register(s4);

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

cbuffer TerrainBuffer : register(b2)
{
    int heightmapIndex;
    float flatThreshold;
    float cliffThreshold;
    float steepnessSmoothing;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 worldPosition : POSITION0;
    float3 viewDir : POSITION1;
    float4 lightViewPos[MAX_LIGHTS] : POSITION2;
};

float3 calculateNormal(float2 pos)
{
    const float gWorldCellSpace = 1 / 100.0f;
	
    const float gTexelCellSpaceU = 1.0f / 1024.0f;
    const float gTexelCellSpaceV = 1.0f / 1024.0f;
	
	// calculate normal from displacement map
    float2 leftTex = pos - float2(gTexelCellSpaceU, 0.0f);
    float2 rightTex = pos + float2(gTexelCellSpaceU, 0.0f);
    float2 topTex = pos - float2(0.0f, gTexelCellSpaceV);
    float2 bottomTex = pos + float2(0.0f, gTexelCellSpaceV);
	
    
    float leftY      = SampleTexture2DLOD(texture2DBuffer, heightmapIndex, heightmapSampler, leftTex, 0).r;
    float rightY     = SampleTexture2DLOD(texture2DBuffer, heightmapIndex, heightmapSampler, rightTex, 0).r;
    float topY       = SampleTexture2DLOD(texture2DBuffer, heightmapIndex, heightmapSampler, topTex, 0).r;
    float bottomY    = SampleTexture2DLOD(texture2DBuffer, heightmapIndex, heightmapSampler, bottomTex, 0).r;
	
    float3 tangent = normalize(float3(2.0f * gWorldCellSpace, rightY - leftY, 0.0f));
    float3 bitangent = normalize(float3(0.0f, bottomY - topY, -2.0f * gWorldCellSpace));
    return normalize(cross(tangent, bitangent));
}

float4 main(InputType input) : SV_TARGET
{
    float3 n = calculateNormal(input.tex);
    float3 v = -normalize(input.viewDir);
    
    /*
    // steepness:
    // global up is always (0, 1, 0), so dot(normal, worldNormal) simplifies to normal.y
    float steepness = 1 - n.y;
    
    float transitionA = steepnessSmoothing * (1 - flatThreshold);
    float transitionB = steepnessSmoothing * (1 - cliffThreshold);
    
    float steepnessStrength = 0.5f * (
        smoothstep(flatThreshold - transitionA, flatThreshold + transitionA, steepness) +
        smoothstep(cliffThreshold - transitionB, cliffThreshold + transitionB, steepness)
    );
    
    // flat ground colouring/texturing
    float shoreMix = 1 - smoothstep(0, 1.5f, input.worldPosition.y);
    float3 shoreColour = float3(0.89f, 0.8f, 0.42f);
    float3 flatColour = float3(0.3f, 0.5f, 0.05f);
    flatColour = lerp(flatColour, shoreColour, shoreMix);
    
    
    // sloped ground colouring
    float3 slopeColour = float3(0.35f, 0.23f, 0.04f);
    
    
    // cliff ground colouring
    float3 cliffColour = float3(0.19f, 0.18f, 0.15f);
    
    
    // texturing:
	
    
    // calculate final ground colour
    float3 groundColour;
    if (steepnessStrength < 0.5f)
        groundColour = lerp(flatColour, slopeColour, remap01(steepnessStrength, 0.0f, 0.5f));
    else
        groundColour = lerp(slopeColour, cliffColour, remap01(steepnessStrength, 0.5f, 1.0f));
    */    

    // lighting
    float3 color = calculateLighting(input.worldPosition, input.lightViewPos, n, v, input.tex,
        materialBuffer.material,
        lightBuffer,
        texture2DBuffer, textureCubeBuffer,
        bilinearSampler, trilinearSampler, anisotropicSampler, shadowSampler);
    
    return float4(color, 1.0f);
}


