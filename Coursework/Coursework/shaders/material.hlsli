#include "lighting.hlsli"


float3 blendNormals(int normalMapIndexA, int normalMapIndexB, float t,
                        float3 n, float3 v, float2 uv, Texture2D tex2DBuffer[TEX_BUFFER_SIZE], SamplerState materialSampler)
{
    float3 mapA, mapB;
    if (normalMapIndexA > -1)
        mapA = SampleTexture2D(tex2DBuffer, normalMapIndexA, materialSampler, uv).rgb;
    if (normalMapIndexB > -1)
        mapB = SampleTexture2D(tex2DBuffer, normalMapIndexB, materialSampler, uv).rgb;
    float3 map = lerp(mapA, mapB, t);
    
    return normalMapToWorld(map, n, v, uv);
}

float3 blendNormals(int normalMapIndex, float3 nB, float t,
                        float3 n, float3 v, float2 uv, Texture2D tex2DBuffer[TEX_BUFFER_SIZE], SamplerState materialSampler)
{
    if (normalMapIndex > -1)
    {
        float3 map = SampleTexture2D(tex2DBuffer, normalMapIndex, materialSampler, uv).rgb;
        float3 nA = normalMapToWorld(map, n, v, uv);
        return normalize(lerp(nA, nB, t));
    }
    return nB;
}


MaterialData materialMix(const MaterialData matA, const MaterialData matB, float t,
                         float2 uv, Texture2D tex2DBuffer[TEX_BUFFER_SIZE], SamplerState materialSampler)
{
    t = saturate(t);
    
    MaterialData mixed;

    // mix albedo
    float3 albedoA, albedoB;
    if (matA.albedoMapIndex > -1)
        albedoA = SampleTexture2D(tex2DBuffer, matA.albedoMapIndex, materialSampler, uv).rgb;
    else
        albedoA = matA.albedoColor;
    if (matB.albedoMapIndex > -1)
        albedoB = SampleTexture2D(tex2DBuffer, matB.albedoMapIndex, materialSampler, uv).rgb;
    else
        albedoB = matB.albedoColor;
    mixed.albedoColor = lerp(albedoA, albedoB, t);
    mixed.albedoMapIndex = -1;
    
    // mix roughness
    float roughnessA, roughnessB;
    if (matA.roughnessMapIndex > -1)
        roughnessA = SampleTexture2D(tex2DBuffer, matA.roughnessMapIndex, materialSampler, uv).r;
    else
        roughnessA = matA.roughnessValue;
    if (matB.roughnessMapIndex > -1)
        roughnessB = SampleTexture2D(tex2DBuffer, matB.roughnessMapIndex, materialSampler, uv).r;
    else
        roughnessB = matB.roughnessValue;
    mixed.roughnessValue = lerp(roughnessA, roughnessB, t);
    mixed.roughnessMapIndex = -1;
    
    // mixing normals is more complicated than simply interpolating
    mixed.normalMapIndex = -1;
    
    mixed.metallic = lerp(matA.metallic, matB.metallic, t);
    
    return mixed;
}
