#include "include/math.hlsli"

cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour;
    float4 ambientColour;
    float3 lightDirection;
    float padding0;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDir : POSITION;
};

float3 lambertian_diffuse(float3 Rf, float3 albedo)
{
	return (1.0f - Rf) * albedo / PI;
}

float3 shlick_fresnel_reflectance(float3 f0, float3 l, float3 h)
{
	return f0 + (1.0f - f0) * pow(1.0f - dot(l, h), 5.0f);
}

float blinn_phong_ndf(float m, float3 n, float3 h)
{
	float normalizeFactor = (m + 8.0f) / (8.0f * PI);
	return normalizeFactor * pow(saturate(dot(n, h)), m);
}

float3 brdf(float3 v, float3 l, float3 n, float3 albedo, float3 f0, float m)
{
	float3 h = normalize(l + v);
 
	float3 Rf = shlick_fresnel_reflectance(f0, l, h);
    
	float3 fspec = Rf * blinn_phong_ndf(m, n, h);
	float3 fdiff = lambertian_diffuse(Rf, albedo);

	return fdiff + fspec;
}

float4 main(InputType input) : SV_TARGET
{
	float3 l0 = float3(0.0f, 0.0f, 0.0f);

	float3 n = normalize(input.normal);
	float3 l = -normalize(lightDirection);
	float3 v = -normalize(input.viewDir);
    
	float3 el = float3(1.0f, 1.0f, 1.0f);
	
	float3 albedo = float3(0.0f, 0.0f, 0.0f);
	float3 f0 = float3(1.0f, 0.71f, 0.29f);
	float m = 32;
	
	l0 = brdf(v, l, n, albedo, f0, m) * el * saturate(dot(n, l));
    
	return l0;
}
