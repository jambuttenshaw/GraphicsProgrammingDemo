#include "lighting.hlsli"


// these functions perform a linear interpolation of the normals
// this is not the best way to interpolate normal data
// but is good enough for the purposes of this project

float3 blendNormals(int normalMapIndexA, int normalMapIndexB, float t,
                        float3 n, float3 v, float2 uv, Texture2D tex2DBuffer[TEX_BUFFER_SIZE], SamplerState materialSampler)
{
    float3 mapA = n;
    float3 mapB = n;
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
    return normalize(lerp(n, nB, t));
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
    
    // mix metallic
    float metallicA, metallicB;
    //if (matA.metalnessMapIndex > -1)
    if (false)
        metallicA = SampleTexture2D(tex2DBuffer, matA.metalnessMapIndex, materialSampler, uv).r;
    else
        metallicA = matA.metallic;
    if (matB.metalnessMapIndex > -1)
        metallicB = SampleTexture2D(tex2DBuffer, matB.metalnessMapIndex, materialSampler, uv).r;
    else
        metallicB = matB.metallic;
    mixed.metallic = lerp(metallicA, metallicB, t);
    mixed.metalnessMapIndex = -1;
    
    return mixed;
}
