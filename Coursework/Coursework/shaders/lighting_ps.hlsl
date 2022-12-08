#include "lighting.hlsli"
#include "defines.hlsli"


// shadows
Texture2D shadowMap0 : register(t0);
Texture2D shadowMap1 : register(t1);
Texture2D shadowMap2 : register(t2);
Texture2D shadowMap3 : register(t3);

SamplerState shadowSampler : register(s0);

// ibl
TextureCube irradianceMap : register(t4);
TextureCube prefilterMap : register(t5);
Texture2D brdfIntegrationMap : register(t6);

SamplerState irradianceMapSampler : register(s1);
SamplerState brdfIntegrationSampler : register(s2);

// material
Texture2D albedoMap : register(t7);
Texture2D roughnessMap : register(t8);
Texture2D normalMap : register(t9);

SamplerState materialSampler : register(s3);

// lighting
cbuffer LightBuffer : register(b0)
{
    // per light:
    float4 lightIrradiance[MAX_LIGHTS];
    float4 lightPosition[MAX_LIGHTS]; // xyz = pos, w = range
    float4 lightDirection[MAX_LIGHTS];
    // x = type
	// y = range
	// z = inner spot angle
	// w = outer spot angle
    float4 lightParams0[MAX_LIGHTS];
    // x = shadows enabled
    // y = shadow bias
    float4 lightParams1[MAX_LIGHTS];

    // global:
	int lightCount;
    bool enableEnvironmentalLighting;
    float2 padding0;
};

// materials
cbuffer MaterialBuffer : register(b1)
{
	float3 albedoColor;
	float useAlbedoMap;
	float roughnessValue;
	float useRoughnessMap;
	float metallic;
	float useNormalMap;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 worldPos : POSITION0;
	float3 viewDir : POSITION1;
    float4 lightViewPos[MAX_LIGHTS] : POSITION2;
};

// shadows
// returns 0 if in shadow or 1 if illuminated
float shadowed(Texture2D shadowMap, float4 lightViewPos, float bias)
{
    float2 uv = remap01_noclamp(lightViewPos.xy / lightViewPos.w);
    uv.y = 1.0f - uv.y;
    
    if (abs(uv.x) > 1.0f || abs(uv.y) > 1.0f)
        return 1.0f;
    
    float depthValue = shadowMap.Sample(shadowSampler, uv).r;
    float distFromLight = lightViewPos.z / lightViewPos.w;
    distFromLight -= bias;
    
    return depthValue >= distFromLight;
}

float distanceAttenuation(float r, float rmax)
{
	float a1 = r / rmax;
	float a2 = max(0.0f, 1.0f - (a1 * a1));
	return a2 * a2;
}

float spotAttenuation(float3 toLight, float3 lightDir, float2 spotAngles)
{
	return (1.0f - smoothstep(spotAngles.x, spotAngles.y, dot(lightDir, toLight)));
}

float3x3 cotangent_frame(float3 N, float3 p, float2 uv)
{ 
    // get edge vectors of the pixel triangle 
	float3 dp1 = ddx(p);
	float3 dp2 = ddy(p);
	float2 duv1 = ddx(uv);
	float2 duv2 = ddy(uv);
    // solve the linear system 
	float3 dp2perp = cross(dp2, N);
	float3 dp1perp = cross(N, dp1);
	float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
    // construct a scale-invariant frame 
    float invmax = rsqrt( max( dot(T,T), dot(B,B) ) ); 
	return float3x3(T * invmax, B * invmax, N);
}


float4 main(InputType input) : SV_TARGET
{
    Texture2D shadowMaps[4];
    shadowMaps[0] = shadowMap0;
    shadowMaps[1] = shadowMap1;
    shadowMaps[2] = shadowMap2;
    shadowMaps[3] = shadowMap3;
    
	float3 n = normalize(input.normal);
	float3 v = -normalize(input.viewDir);
    float3 r = normalize(reflect(-v, n));
    
	if (useNormalMap)
	{
		float3 map = normalMap.Sample(materialSampler, input.tex);
		map = (map * 2.0f) - 1.0f;
        
		float3x3 TBN = cotangent_frame(n, -v, input.tex);
		n = normalize(mul(map, TBN));
	}
    
    float3 albedo = useAlbedoMap ? albedoMap.Sample(materialSampler, input.tex).rgb : albedoColor.rgb;
	float roughness = useRoughnessMap ? roughnessMap.Sample(materialSampler, input.tex).r : roughnessValue;
    
    float3 f0 = float3(0.04f, 0.04f, 0.04f);
    f0 = lerp(f0, albedo, metallic);
    
    float3 lo = float3(0.0f, 0.0f, 0.0f);
	
	for (int i = 0; i < lightCount; i++)
	{       
		// calculate light direction and irradiance
        float type = lightParams0[i].x;
        float3 el = lightIrradiance[i].rgb;
		
        float3 l = float3(0.0f, 0.0f, 1.0f);
        if (type == 0.0f)
        {
            l = -lightDirection[i].xyz;
        }
		else if (type == 1.0f)
        {
            l = normalize(lightPosition[i].xyz - input.worldPos);
			el *= distanceAttenuation(length(lightPosition[i].xyz - input.worldPos), lightParams0[i].y);
		}
		else if (type == 2.0f)
        {
			float3 toLight = lightPosition[i].xyz - input.worldPos;
			l = normalize(toLight);
            el *= distanceAttenuation(length(toLight), lightParams0[i].y) * spotAttenuation(l, -lightDirection[i].xyz, lightParams0[i].zw);
        }
    
		// evaluate shading equation
		float3 brdf = ggx_brdf(v, l, n, albedo, f0, roughness, metallic) * el * saturate(dot(n, l));
        
        float shadow = 1.0f;
        if (lightParams1[i].x)
            shadow = shadowed(shadowMaps[i], input.lightViewPos[i], lightParams1[i].y);
        
        lo += brdf * shadow;
    }
    
    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    if (enableEnvironmentalLighting)
    {
        // ues IBL for ambient lighting
        float3 F = shlick_fresnel_roughness_reflectance(f0, v, n, roughness);
        
        float3 kS = F;
        float3 kD = 1.0f - kS;
        kD *= 1.0f - metallic;
        
        float3 irradiance = irradianceMap.Sample(irradianceMapSampler, n).rgb;
        float3 diffuse = irradiance * albedo;
        
        // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
        const float MAX_REFLECTION_LOD = 4.0f;
        float3 prefilteredColor = prefilterMap.SampleLevel(irradianceMapSampler, r, roughness * MAX_REFLECTION_LOD).rgb;
        float2 brdf = brdfIntegrationMap.Sample(brdfIntegrationSampler, float2(abs(dot(n, v)), roughness)).rg;
        float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        ambient = kD * diffuse + specular;
	}
    
    float3 color = ambient + lo;
    
    return float4(color, 1.0f);
}
