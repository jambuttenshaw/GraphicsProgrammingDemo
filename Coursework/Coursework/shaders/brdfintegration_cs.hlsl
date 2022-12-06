#include "lighting.hlsli"

RWTexture2D<float2> BRDFIntegrationMap : register(u0);


float2 IntegrateBRDF(float NdotV, float roughness)
{
    float3 V;
    V.x = sqrt(1.0f - NdotV * NdotV);
    V.y = 0.0f;
    V.z = NdotV;

    float A = 0.0f;
    float B = 0.0f;

    float3 N = float3(0.0f, 0.0f, 1.0f);

    const uint SAMPLE_COUNT = 1024u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0f);
        float NdotH = max(H.z, 0.0f);
        float VdotH = max(dot(V, H), 0.0f);

        if (NdotL > 0.0f)
        {
            float G = smith_geometry_ibl(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return float2(A, B);
}


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 BRDFIntegrationDims;
    BRDFIntegrationMap.GetDimensions(BRDFIntegrationDims.x, BRDFIntegrationDims.y);
    
    if (DTid.x > BRDFIntegrationDims.x || DTid.y > BRDFIntegrationDims.y)
        return;
    
    // calculate normal (direction from the centre of the cubemap through this fragment
    float2 uv = DTid.xy / (float2) (BRDFIntegrationDims);
    
    float2 integratedBRDF = IntegrateBRDF(uv.x, uv.y);
    
    BRDFIntegrationMap[DTid.xy] = integratedBRDF;
}
