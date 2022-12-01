#include "lighting.hlsli"

#define MAX_LIGHTS 4

cbuffer LightBuffer : register(b0)
{
    float4 globalAmbience;
    float4 lightIrradiance[MAX_LIGHTS];
    float4 lightPosition[MAX_LIGHTS];
    float4 lightDirection[MAX_LIGHTS];
    float4 lightTypeAndSpotAngles[MAX_LIGHTS];
	float lightCount;
    float3 padding0;
};

cbuffer MaterialBuffer : register(b1)
{
	float4 albedo;
	float metallic;
	float roughness;
	float2 padding1;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 worldPos : POSITION0;
	float3 viewDir : POSITION1;
};


float4 main(InputType input) : SV_TARGET
{
    float3 l0 = globalAmbience.rgb;

	float3 n = normalize(input.normal);
	float3 v = -normalize(input.viewDir);
	
	for (int i = 0; i < lightCount; i++)
	{
		// calculate light direction and irradiance
        float type = lightTypeAndSpotAngles[i].x;
        float3 el = lightIrradiance[i].rgb;
		
        float3 l = float3(0.0f, 0.0f, 1.0f);
        if (type == 0.0f)
        {
			l = -normalize(lightDirection[i].xyz);
        }
		else if (type == 1.0f)
        {
            l = normalize(lightPosition[i].xyz - input.worldPos);
        }
		else if (type == 2.0f)
        {
            l = -normalize(lightDirection[i].xyz);
			
            float3 toLight = normalize(lightPosition[i].xyz - input.worldPos);
            float spotAttenuation = 1.0f - smoothstep(lightTypeAndSpotAngles[i].y, lightTypeAndSpotAngles[i].z, dot(l, toLight));
            el *= spotAttenuation;
        }
    
		// evaluate shading equation
		l0 += ggx_brdf(v, l, n, albedo.rgb, roughness, metallic) * el * saturate(dot(n, l));
	}

	return float4(l0, 1.0f);
}
