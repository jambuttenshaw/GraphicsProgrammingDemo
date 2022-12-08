
#define MAX_LIGHTS 4

#define LIGHT_TYPE_DIRECTIONAL 0.0f
#define LIGHT_TYPE_POINT 1.0f
#define LIGHT_TYPE_SPOT 2.0f


struct LightData
{
    float4 irradiance;
    float4 position;
    float4 direction;
    
    float type;
    float range;
    float2 spotAngles;
    
    float shadowsEnabled;
    float shadowBias;
    
    float2 padding;
};

struct LightBuffer
{
    LightData lights[MAX_LIGHTS];
    int lightCount;
    bool enableEnvironmentalLighting;
    float2 padding;
};

struct MaterialData
{
    float3 albedoColor;
    float useAlbedoMap;
    float roughnessValue;
    float useRoughnessMap;
    float metallic;
    float useNormalMap;
};