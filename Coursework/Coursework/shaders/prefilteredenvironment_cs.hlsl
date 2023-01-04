#include "lighting.hlsli"


TextureCube environmentMap : register(t0);
SamplerState environmentSampler : register(s0);

RWTexture2D<float4> PEM : register(u0);

cbuffer ParamsBuffer : register(b0)
{
    float3 faceNormal;
    float roughness;
    float3 faceTangent;
    float padding1;
    float3 faceBitangent;
    float padding2;
}


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 PEMDims;
    PEM.GetDimensions(PEMDims.x, PEMDims.y);
    
    if (DTid.x > PEMDims.x || DTid.y > PEMDims.y)
        return;
    
    // calculate normal (direction from the centre of the cubemap through this fragment
    float2 uv = DTid.xy / (float2) (PEMDims - uint2(1, 1));
    uv = uv * 2.0f - 1.0f;
    
    float3 worldPos = faceNormal + uv.x * faceTangent + uv.y * faceBitangent;
    
    float3 N = normalize(worldPos);
    float3 R = N;
    float3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0f;
    float3 prefilteredColor = float3(0.0f, 0.0f, 0.0f);
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0f);
        if (NdotL > 0.0f)
        {
            prefilteredColor += environmentMap.SampleLevel(environmentSampler, L, 0.0f).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;
    
    PEM[DTid.xy] = float4(prefilteredColor, 1.0f);
}
