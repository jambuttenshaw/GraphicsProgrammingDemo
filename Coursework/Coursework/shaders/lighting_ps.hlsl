#include "include/math.hlsli"

cbuffer LightBuffer : register(b0)
{
    float4 irradiance;
    float3 lightDirection;
    float padding0;
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
	float3 viewDir : POSITION;
};

float3 lambertian_diffuse(float3 albedo)
{
	return albedo / PI;
}

float3 shlick_fresnel_reflectance(float3 f0, float3 v, float3 h)
{
	return f0 + (1.0f - f0) * pow(1.0f - saturate(dot(h, v)), 5.0f);
}

float blinn_phong_ndf(float3 n, float3 h, float3 m)
{
	float normalizeFactor = (m + 8.0f) / (8.0f * PI);
	return normalizeFactor * pow(saturate(dot(n, h)), m);
}

float ggx_ndf(float3 n, float3 h, float a)
{
	float a2 = a * a;
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	
	float denom = (NdotH2 * (a2 - 1.0f)) + 1.0f;
	denom = PI * denom * denom;
	
	return a2 / denom;
}

float schlichggx_geometry(float NdotV, float k)
{
	float denom = NdotV * (1.0f - k) + k;
	
	return NdotV / denom;
}
  
float smith_geometry(float3 n, float3 v, float3 l, float k)
{
	float NdotV = saturate(dot(n, v));
	float NdotL = saturate(dot(n, l));
	float ggx1 = schlichggx_geometry(NdotV, k);
	float ggx2 = schlichggx_geometry(NdotL, k);
	
	return ggx1 * ggx2;
}

float3 ggx_brdf(float3 v, float3 l, float3 n, float3 albedo, float roughness, float metallic)
{
	float3 h = normalize(l + v);
 
	// specular
	float3 f0 = float3(0.04f, 0.04f, 0.04f);
	f0 = lerp(f0, albedo.rgb, metallic);
	
	// fresnel effect tells us how much light is reflected
	float3 Rf = shlick_fresnel_reflectance(f0, v, h);
	float ndf = ggx_ndf(n, h, roughness);
	float G = smith_geometry(n, v, l, roughness);
	
	float3 numerator = Rf * ndf * G;
	float denominator = 4.0f * saturate(dot(n, v)) * saturate(dot(n, l)) + 0.0001f;
	
	float3 fspec = numerator / denominator;

	// diffuse
	
	// all light that is not reflected is diffused
	float3 cdiff = albedo - f0;
	cdiff *= 1.0f - metallic;
	
	float3 fdiff = lambertian_diffuse(cdiff);
	
	return fdiff + fspec;
}

float4 main(InputType input) : SV_TARGET
{
	float3 l0 = float3(0.0f, 0.0f, 0.0f);

	float3 n = normalize(input.normal);
	float3 l = -normalize(lightDirection);
	float3 v = -normalize(input.viewDir);
    
	
	l0 = ggx_brdf(v, l, n, albedo.rgb, roughness, metallic) * irradiance.rgb * saturate(dot(n, l));
	
	return float4(l0, 1.0f);
}
