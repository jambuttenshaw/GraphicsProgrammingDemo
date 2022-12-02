#include "lighting.hlsli"

#define MAX_LIGHTS 4

TextureCube environmentMap : register(t0);
SamplerState environmentSampler : register(s0);


cbuffer LightBuffer : register(b0)
{
    float4 globalAmbience;
    float4 lightIrradiance[MAX_LIGHTS];
    float4 lightPosition[MAX_LIGHTS];
    float4 lightDirection[MAX_LIGHTS];
    float4 lightTypeAndSpotAngles[MAX_LIGHTS];
	int lightCount;
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
	float3 n = normalize(input.normal);
	float3 v = -normalize(input.viewDir);
    float3 r = normalize(reflect(-v, n));

    float3 f0 = float3(0.04f, 0.04f, 0.04f);
    f0 = lerp(f0, albedo.rgb, metallic);
    
    float3 lo = float3(0.0f, 0.0f, 0.0f);
	
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
		lo += ggx_brdf(v, l, n, albedo.rgb, f0, roughness, metallic) * el * saturate(dot(n, l));
    }
    
    // ues IBL for ambient lighting
    float3 F = shlick_fresnel_roughness_reflectance(f0, v, n, roughness);
    
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    float3 irradiance = environmentMap.Sample(environmentSampler, n).rgb;
    float3 diffuse = irradiance * albedo.rgb;
    
    /*
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);
*/
    float3 ambient = (kD * diffuse);
    
    float3 color = ambient + lo;

    return float4(color, 1.0f);
}
