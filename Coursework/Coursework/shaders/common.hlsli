#include "defines.hlsli"

// for calculating luminance of a colour
static const float4 LUM_VECTOR = float4(0.299f, 0.587f, 0.114f, 0.0f);

// Struct definitions (THESE MUST MATCH THOSE DEFINED IN ShaderUtility.h!!!!)
struct LightData
{
    float4 irradiance;
    float4 position;
    float4 direction;
    
    float type;
    float range;
    float2 spotAngles;
    
    int shadowMapIndex;
    float2 shadowBiasCoeffs;
    
    float padding;
};
struct MaterialData
{
    float3 albedoColor;
    int albedoMapIndex;
    
    float roughnessValue;
    int roughnessMapIndex;
    
    float metallic;
    int metalnessMapIndex;
    
    int normalMapIndex;
    
    float3 padding;
};


struct PointLightMatrices
{
    matrix right[MAX_LIGHTS];
    matrix left[MAX_LIGHTS];
    matrix up[MAX_LIGHTS];
    matrix down[MAX_LIGHTS];
    matrix forward[MAX_LIGHTS];
    matrix back[MAX_LIGHTS];
};

// Buffer types

struct VSLightBuffer
{
    matrix lightMatrix[MAX_LIGHTS];
    float4 lightPosAndType[MAX_LIGHTS];
    float3 cameraPos;
    float padding;
    PointLightMatrices pointLightMatrices;
};
struct PSLightBuffer
{
    LightData lights[MAX_LIGHTS];
    
    int lightCount;
    bool enableEnvironmentalLighting;
    int irradianceMapIndex;
    int prefilterMapIndex;
    
    int brdfIntegrationMapIndex;
    float3 padding;
};

struct MaterialBuffer
{
    MaterialData materials[MAX_MATERIALS];
    int materialCount;
    float3 padding;
};


matrix GetPointLightViewMatrix(int i, float3 toFrag, PointLightMatrices mats)
{
    float3 absL = abs(toFrag);
    float maxComponent = max(absL.x, max(absL.y, absL.z));
    if (maxComponent == absL.x)
    {
        if (toFrag.x > 0)
            return mats.right[i];
        else
            return mats.left[i];
    }
    else if (maxComponent == absL.y)
    {
        if (toFrag.y > 0)
            return mats.up[i];
        else
            return mats.down[i];
    }
    else
    {
        if (toFrag.z > 0)
            return mats.forward[i];
        else
            return mats.back[i];
    }
}

#include "texturefuncs.hlsli"
