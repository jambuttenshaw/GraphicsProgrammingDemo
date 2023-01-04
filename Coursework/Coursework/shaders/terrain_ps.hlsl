// Terrain Pixel Shader

#include "material.hlsli"

// textures
Texture2D texture2DBuffer[TEX_BUFFER_SIZE] : register(t0);
TextureCube textureCubeBuffer[TEX_BUFFER_SIZE] : register(t32);

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
    float heightmapDims;
    float terrainSize;
    
    int heightmapIndex;
    float uvScale;
    float flatThreshold;
    float cliffThreshold;
    
    float steepnessSmoothing;
    float heightSmoothing;
    float shoreThreshold;
    float snowHeightThreshold;
    
    float2 minMaxSnowSteepness;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION0;
    float3 viewDir : POSITION1;
    float4 lightViewPos[MAX_LIGHTS] : POSITION2;
};

float3 calculateNormalTangentSpace(float2 pos)
{
    const float2 gWorldCellSpace = float2(1.0f, 1.0f) / terrainSize;
	
    const float2 gTexelCellSpace = float2(1.0f, 1.0f) / heightmapDims;
	
	// calculate normal from displacement map
    float2 leftTex = pos - float2(gTexelCellSpace.x, 0.0f);
    float2 rightTex = pos + float2(gTexelCellSpace.x, 0.0f);
    float2 topTex = pos - float2(0.0f, gTexelCellSpace.y);
    float2 bottomTex = pos + float2(0.0f, gTexelCellSpace.y);
	
    
    float leftY      = SampleTexture2DLOD(texture2DBuffer, heightmapIndex, heightmapSampler, leftTex, 0).r;
    float rightY     = SampleTexture2DLOD(texture2DBuffer, heightmapIndex, heightmapSampler, rightTex, 0).r;
    float topY       = SampleTexture2DLOD(texture2DBuffer, heightmapIndex, heightmapSampler, topTex, 0).r;
    float bottomY    = SampleTexture2DLOD(texture2DBuffer, heightmapIndex, heightmapSampler, bottomTex, 0).r;
	
    float3 tangent = normalize(float3(2.0f * gWorldCellSpace.x, 0.0f, rightY - leftY));
    float3 bitangent = normalize(float3(0.0f, 2.0f * gWorldCellSpace.y, bottomY - topY));
    return normalize(cross(tangent, bitangent));
}

float4 main(InputType input) : SV_TARGET
{
    input.normal = normalize(input.normal);
    float3 tangentSpaceNormal = calculateNormalTangentSpace(input.tex);
    
    float2 uv = input.tex * uvScale;
    float3 v = -normalize(input.viewDir);
    float3 n = tangentSpaceToWorldSpace(tangentSpaceNormal, input.normal, float3(1.0f, 0.0f, 0.0f));
    
    // steepness:
    float steepness = 1 - dot(input.normal, n);
    
    // fractional values blend between adjacent materials
    float s1 = steepnessSmoothing * 0.5f;
    float s2 = heightSmoothing * 0.5f;
    
    // blend the slope materials
    float matBlend = smoothstep(flatThreshold - s1, flatThreshold + s1, steepness);
    matBlend += smoothstep(cliffThreshold - s1, cliffThreshold + s1, steepness);
    
    // mix materials
    
    MaterialData groundMaterial = materialMix(
            materialBuffer.materials[int(matBlend) + 1],
            materialBuffer.materials[min(int(matBlend) + 2, materialBuffer.materialCount)],
            frac(matBlend),
            uv, texture2DBuffer, anisotropicSampler);
    
    float3 materialNormal = blendNormals(materialBuffer.materials[int(matBlend) + 1].normalMapIndex,
            materialBuffer.materials[min(int(matBlend) + 2, materialBuffer.materialCount)].normalMapIndex,
            frac(matBlend), n, v,
            uv, texture2DBuffer, anisotropicSampler);
    
    
    {
        // blend the shore
        float shoreMix = smoothstep(shoreThreshold - s2, shoreThreshold + s2, input.worldPosition.y);
        groundMaterial = materialMix(
            materialBuffer.materials[0],
            groundMaterial,
            shoreMix,
            uv, texture2DBuffer, anisotropicSampler);
        materialNormal = blendNormals(materialBuffer.materials[0].normalMapIndex,
            materialNormal,
            shoreMix, n, v,
            uv, texture2DBuffer, anisotropicSampler);
    }
    
    
    {
        // blend snow
        float snowMix = mul(
                smoothstep(snowHeightThreshold - s2, snowHeightThreshold + s2, input.worldPosition.y),
                (1.0f - smoothstep(minMaxSnowSteepness.x - s1, minMaxSnowSteepness.x + s1, steepness)) +
                smoothstep(minMaxSnowSteepness.y - s1, minMaxSnowSteepness.y + s1, steepness)
        );
        
        groundMaterial = materialMix(
            groundMaterial,
            materialBuffer.materials[4],
            snowMix,
            uv, texture2DBuffer, anisotropicSampler);
        materialNormal = blendNormals(materialBuffer.materials[4].normalMapIndex,
            materialNormal,
            1.0f - snowMix, n, v,
            uv, texture2DBuffer, anisotropicSampler);
    }
    
    // unsure why this is needed, but this fixes the normals
    materialNormal.z = -materialNormal.z;
    
    // lighting
    float3 color = calculateLighting(input.worldPosition, input.lightViewPos, materialNormal, v, uv,
        groundMaterial,
        lightBuffer,
        texture2DBuffer, textureCubeBuffer,
        bilinearSampler, trilinearSampler, anisotropicSampler, shadowSampler);
    
    return float4(color, 1.0f);
}


