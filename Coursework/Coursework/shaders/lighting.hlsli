#include "math.hlsli"

// DIFFUSE NDF's
float3 lambertian_diffuse(float3 albedo)
{
	return albedo / PI;
}

// SPECULAR NDF's
float blinn_phong_ndf(float3 n, float3 h, float m)
{
	#define ROUGHNESS_MULTIPLIER 256
	m = (1.0f - m) * ROUGHNESS_MULTIPLIER; // unlike ggx, blinn-phong doesnt use a normalized roughness
										   // so for convenience just define the max smoothness to be 256
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

// FRESNEL
float3 shlick_fresnel_reflectance(float3 f0, float3 v, float3 h)
{
	return f0 + (1.0f - f0) * pow(1.0f - saturate(dot(h, v)), 5.0f);
}

float3 shlick_fresnel_roughness_reflectance(float3 f0, float3 v, float3 h, float roughness)
{
    return f0 + (max((1.0f - roughness).xxx, f0) - f0) * pow(1.0f - saturate(dot(h, v)), 5.0f);
}

// GEOMETRY FUNCTIONS
float schlichggx_geometry(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0f - k) + k);
}
  
float smith_geometry(float3 n, float3 v, float3 l, float k)
{
	float NdotV = saturate(dot(n, v));
	float NdotL = saturate(dot(n, l));
	float ggx1 = schlichggx_geometry(NdotV, k);
	float ggx2 = schlichggx_geometry(NdotL, k);
	
	return ggx1 * ggx2;
}

float schlichggx_geometry_ibl(float NdotV, float roughness)
{
    float k = (roughness * roughness) / 2.0f; // k has a different definition for ibl than direct lighting
    return NdotV / (NdotV * (1.0f - k) + k);
}
float smith_geometry_ibl(float3 n, float3 v, float3 l, float roughness)
{
    float NdotV = max(dot(n, v), 0.0f);
    float NdotL = max(dot(n, l), 0.0f);
    float ggx2 = schlichggx_geometry_ibl(NdotV, roughness);
    float ggx1 = schlichggx_geometry_ibl(NdotL, roughness);

    return ggx1 * ggx2;
}


// BRDF's
/*
float3 blinn_phong_brdf(float3 v, float3 l, float3 n, float3 albedo, float3 f0, float roughness, float metallic)
{
	float3 h = normalize(l + v);
 
	// specular
	
	// fresnel effect tells us how much light is reflected
	float3 Rf = shlick_fresnel_reflectance(f0, v, h);
	float ndf = blinn_phong_ndf(n, h, roughness);
	// blinn-phong has an implicit geometry factor of cos(Wi)*cos(Wo)
	
	float3 fspec = Rf * ndf / 4.0f;

	// diffuse
	
	// all light that is not reflected is diffused
	float3 cdiff = albedo - f0;
	cdiff *= 1.0f - metallic;
	
	float3 fdiff = lambertian_diffuse(cdiff);
	
	return fdiff + fspec;
}
*/

float3 ggx_brdf(float3 v, float3 l, float3 n, float3 albedo, float3 f0, float roughness, float metallic)
{
	float3 h = normalize(l + v);
 
	// specular
	
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


// IBL

// adapted GGX NDF used to generate orientated and biased sample vectors for importance sampling, used for preprocessing the environment map for IBL
float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
	
    float phi = 2.0f * PI * Xi.x;
    float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    float3 up = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}
