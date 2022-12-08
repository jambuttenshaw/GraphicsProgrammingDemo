#include "math.hlsli"

TextureCube environmentMap : register(t0);
SamplerState environmentSampler : register(s0);

RWTexture2D<float4> irradianceMap : register(u0);

cbuffer ParamsBuffer : register(b0)
{
    float3 faceNormal;
    float padding0;
    float3 faceTangent;
    float padding1;
    float3 faceBitangent;
    float padding2;
}


[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 irradianceMapDims;
    irradianceMap.GetDimensions(irradianceMapDims.x, irradianceMapDims.y);
    
    if (DTid.x > irradianceMapDims.x || DTid.y > irradianceMapDims.y)
        return;
    
    float2 uv = DTid.xy / (float2) (irradianceMapDims);
    uv = uv * 2.0f - 1.0f;
    
    float3 worldPos = faceNormal + uv.x * faceTangent + uv.y * faceBitangent;
    float3 normal = normalize(worldPos);
    
    // calculate irradiance
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);

    float3 up = float3(0.0f, 1.0f, 0.0f);
    
    // fix artifacts at the poles of the irradiance map when normal and up are co-linear
    if (abs(normal.y) == 1.0f)
        up.yz = up.zy;
    
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));
    float sampleDelta = 0.025f;
    float nrSamples = 0.0f;
    for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
        {
        // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
        // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += environmentMap.SampleLevel(environmentSampler, sampleVec, 0).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance / nrSamples;
    
    irradianceMap[DTid.xy] = float4(irradiance, 1.0f);
}