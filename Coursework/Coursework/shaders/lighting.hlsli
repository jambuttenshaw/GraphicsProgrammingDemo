#include "math.hlsli"
#include "defines.hlsli"

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

// BRDF
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


// attenuation
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


// shadows
float calculateShadowFactor(Texture2D texBuffer[TEX_BUFFER_SIZE], int shadowMapIndex, SamplerComparisonState shadowSampler, float4 lightViewPos)
{
    const float SMAP_SIZE = 1024.0f;
    const float dx = 1.0f / SMAP_SIZE;
    
    float2 uv = remap01_noclamp(lightViewPos.xy / lightViewPos.w);
    uv.y = 1.0f - uv.y;
    
    if (abs(uv.x) > 1.0f || abs(uv.y) > 1.0f)
        return 1.0f;
    
    float distFromLight = lightViewPos.z / lightViewPos.w;
    
    // perform 3x3 box blur
    
    const float2 offsets[9] =
    {
        { -dx, -dx }, { 0.0f, -dx }, { dx, -dx },
        { -dx, 0.0f }, { 0.0f, 0.0f }, { dx, 0.0f },
        { -dx, dx }, { 0.0f, dx }, { dx, dx }
    };
    
    float percentLit = 0.0f;
    [unroll]
    for (int i = 0; i < 9; i++)
        percentLit += SampleTexture2DComp(texBuffer, shadowMapIndex, shadowSampler, uv + offsets[i], distFromLight).r;
    
    return percentLit / 9.0f;
}

float calculateShadowFactor(TextureCube texBuffer[TEX_BUFFER_SIZE], int shadowMapIndex, SamplerComparisonState shadowSampler, float4 lightViewPos, float3 lightToFrag)
{
    const float SMAP_SIZE = 1024.0f;
    const float dx = 1.0f / SMAP_SIZE;
    
    float3 uvw = normalize(lightToFrag);
        
    float distFromLight = lightViewPos.z / lightViewPos.w;

    float percentLit = SampleTextureCubeComp(texBuffer, shadowMapIndex, shadowSampler, uvw, distFromLight).r;
    
    return percentLit;
}


// ambient lighting
float3 calculateAmbientLighting(float3 n, float3 v, float3 albedo, float3 f0, float roughness, float metallic,
                                Texture2D tex2dBuffer[TEX_BUFFER_SIZE], TextureCube texCubeBuffer[TEX_BUFFER_SIZE],
                                int irradianceMapIndex, int prefilterMapIndex, int brdfMapIndex,
                                SamplerState trilinearSampler, SamplerState bilinearClampSampler)
{
    // ues IBL for ambient lighting
    float3 F = shlick_fresnel_roughness_reflectance(f0, v, n, roughness);
        
    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0f - metallic;
        
    float3 irradiance = SampleTextureCube(texCubeBuffer, irradianceMapIndex, trilinearSampler, n).rgb;
    float3 diffuse = irradiance * albedo;
        
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0f;
    float3 r = normalize(reflect(-v, n));
    
    float3 prefilteredColor = SampleTextureCubeLOD(texCubeBuffer, prefilterMapIndex, trilinearSampler, r, roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdf = SampleTexture2D(tex2dBuffer, brdfMapIndex, bilinearClampSampler, float2(abs(dot(n, v)), roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    return kD * diffuse + specular;
}



// the big calculate lighting function
float3 calculateLighting(
    float3 p,                       // world pos
    float4 l_p[MAX_LIGHTS],         // light view pos
    float3 n,                       // world space normal
    float3 v,                       // world space view direction
    float2 uv,                      // tex coords
    const MaterialData material,    // material properties
    const LightBuffer lights,       // light data
    Texture2D texture2DBuffer[TEX_BUFFER_SIZE],
    TextureCube textureCubeBuffer[TEX_BUFFER_SIZE],
    SamplerState bilinearSampler,
    SamplerState trilinearSampler,
    SamplerState anisotropicSampler,
    SamplerComparisonState shadowSampler
)
{
    float c = calculateShadowFactor(textureCubeBuffer, lights.lights[0].shadowMapIndex, shadowSampler, l_p[0], p - lights.lights[0].position.xyz);
    return c;
    /*
    if (material.normalMapIndex > -1)
    {
        float3 map = SampleTexture2D(texture2DBuffer, material.normalMapIndex, bilinearSampler, uv).rgb;
        map = (map * 2.0f) - 1.0f;
        
        float3x3 TBN = cotangent_frame(n, -v, uv);
        n = normalize(mul(map, TBN));
    }
    
    float3 albedo = material.albedoMapIndex > -1 ? SampleTexture2D(texture2DBuffer, material.albedoMapIndex, anisotropicSampler, uv).rgb : material.albedoColor.rgb;
    float roughness = material.roughnessMapIndex > -1 ? SampleTexture2D(texture2DBuffer, material.roughnessMapIndex, anisotropicSampler, uv).r : material.roughnessValue;
    
    float3 f0 = float3(0.04f, 0.04f, 0.04f);
    f0 = lerp(f0, albedo, material.metallic);
    
    float3 lo = float3(0.0f, 0.0f, 0.0f);
	
    for (int i = 0; i < lights.lightCount; i++)
    {
        LightData light = lights.lights[i];
		// calculate light direction and irradiance
        float type = light.type;
        float3 el = light.irradiance.rgb;
		
        float3 l = float3(0.0f, 0.0f, 1.0f);
        if (type == LIGHT_TYPE_DIRECTIONAL)
        {
            l = -light.direction.xyz;
        }
        else if (type == LIGHT_TYPE_POINT)
        {
            l = normalize(light.position.xyz - p);
            el *= distanceAttenuation(length(light.position.xyz - p), light.range);
        }
        else if (type == LIGHT_TYPE_SPOT)
        {
            float3 toLight = light.position.xyz - p;
            l = normalize(toLight);
            el *= distanceAttenuation(length(toLight), light.range) * spotAttenuation(l, -light.direction.xyz, light.spotAngles);
        }
    
		// evaluate shading equation
        float3 brdf = ggx_brdf(v, l, n, albedo, f0, roughness, material.metallic) * el * saturate(dot(n, l));
        
        float shadowFactor = 1.0f;
        if (light.shadowMapIndex > -1)
        {
            if (light.type == LIGHT_TYPE_POINT)
                shadowFactor = calculateShadowFactor(textureCubeBuffer, light.shadowMapIndex, shadowSampler, l_p[i], p - light.position.xyz);
            else
                shadowFactor = calculateShadowFactor(texture2DBuffer, light.shadowMapIndex, shadowSampler, l_p[i]);
        }
        
        lo += brdf * shadowFactor;
    }
    
    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    if (lights.enableEnvironmentalLighting)
    {
        ambient = calculateAmbientLighting( n, v, albedo, f0, roughness, material.metallic,
                                            texture2DBuffer, textureCubeBuffer,
                                            lights.irradianceMapIndex, lights.prefilterMapIndex, lights.brdfIntegrationMapIndex,
                                            trilinearSampler, bilinearSampler);
    }
    
    return ambient + lo;
    */
}
