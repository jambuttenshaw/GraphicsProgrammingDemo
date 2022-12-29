// Water post processing effect

#include "lighting.hlsli"

Texture2D texture2DBuffer[TEX_BUFFER_SIZE] : register(t0);
TextureCube textureCubeBuffer[TEX_BUFFER_SIZE] : register(t32);

SamplerState normalMapSampler : register(s0);
SamplerState bilinearSampler : register(s1);
SamplerState trilinearSampler : register(s2);

cbuffer WaterBuffer : register(b0)
{
    matrix projection;
    
    float4 specularColour;
    float4 transmittanceColour;
    
    int rtColourMapIndex;
    int rtDepthMapIndex;
    int normalMapAIndex;
    int normalMapBIndex;
    
    float3 cameraPos;
    float transmittanceDepth;
    
    float3 oceanBoundsMin;
    float normalMapScale;
    
    float3 oceanBoundsMax;
    float normalMapStrength;
    
    float time;
    float waveSpeed;
    float waveAngle;
    float padding;
};

cbuffer LightCB : register(b1)
{
    PSLightBuffer lightBuffer;
};

struct InputType
{
    float4 position : SV_Position;
    float2 tex : TEXCOORD;
    float3 viewVector : POSITION0;
};


float calculateLinearDepth(float z)
{
    return projection._43 / (z - projection._33);
}


float4 main(InputType input) : SV_TARGET
{
    float4 colour = LoadTexture2D(texture2DBuffer, rtColourMapIndex, input.position.xy);
    
    float2 hitInfo = intersectAABB(cameraPos, input.viewVector, oceanBoundsMin, oceanBoundsMax);
    // the units of these distances are length(input.viewVector)
    // for some reason normalizing the view vector messes lots of stuff up?
    float distToOcean = hitInfo.x;
    float distThroughOcean = hitInfo.y;
    
    float depthSample = LoadTexture2D(texture2DBuffer, rtDepthMapIndex, input.position.xy).r;
    float depthToScene = calculateLinearDepth(depthSample);

    if (distToOcean < distThroughOcean && distThroughOcean > 0 && distToOcean < depthToScene)
    {
        float depthThroughWater = min(depthToScene - distToOcean, distThroughOcean);
        
        // normal mapping
        
        // work out where on the ocean surface this fragment lies
        float3 intersectionPoint = cameraPos + input.viewVector * distToOcean;
        // find out the uv of the intersection point
        float2 uv = ((intersectionPoint - oceanBoundsMin) / (oceanBoundsMax - oceanBoundsMin)).xz;
        uv.y = 1 - uv.y;
        
        // this is not ideal
        const float3 normalW = float3(0, 1, 0);
        
        float2 uvScale = uv * normalMapScale;
        float2 uvOffsetA = waveSpeed * time * float2(cos(waveAngle), sin(waveAngle));
        float2 uvOffsetB = waveSpeed * time * float2(cos(waveAngle + 0.5f * PI), sin(waveAngle + 0.5f * PI));
        float3 normalMapASample = SampleTexture2D(texture2DBuffer, normalMapAIndex, normalMapSampler,
                                                    uvScale + uvOffsetA).rgb;
        float3 normalMapBSample = SampleTexture2D(texture2DBuffer, normalMapAIndex, normalMapSampler,
                                                    uvScale + uvOffsetB).rgb;
        
        // convert normals to world space
        float3 bumpedNormal = tangentToWorld(normalMapASample, normalW, input.viewVector, input.tex);
        bumpedNormal = tangentToWorld(normalMapBSample, bumpedNormal, input.viewVector, input.tex);
        // lighting
        
        // calculate the transmission luminance
        float3 physicalExtinction = -log(transmittanceColour.rgb) / transmittanceDepth;
        float3 T = exp(-physicalExtinction * depthThroughWater);
        
        // calculate the specular reflection on the waters surface
        float3 n = normalize(lerp(normalW, bumpedNormal, normalMapStrength));
        float3 v = -input.viewVector;
        float3 p = intersectionPoint;
        
        float3 specular = float3(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < lightBuffer.lightCount; i++)
        {
            LightData light = lightBuffer.lights[i];
            
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
            float3 brdf_specular = ggx_brdf_specular(v, l, n, specularColour.rgb, 0.005f) * el * saturate(dot(n, l));
        
        
            specular += brdf_specular;
        }
        
        if (lightBuffer.enableEnvironmentalLighting)
        {
            specular += calculateAmbientSpecular(n, v, specularColour.rgb, 0.0f,
                                                       texture2DBuffer, textureCubeBuffer,
                                                       lightBuffer.prefilterMapIndex, lightBuffer.brdfIntegrationMapIndex,
                                                       trilinearSampler, bilinearSampler);
        }
        
        float3 waterColour = specular + T * colour.rgb;
        colour = float4(waterColour, 1.0f);
    }
    
    return colour;
}
