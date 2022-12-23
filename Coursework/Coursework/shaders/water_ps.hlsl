// Water post processing effect

#include "lighting.hlsli"

Texture2D texture2DBuffer[TEX_BUFFER_SIZE] : register(t0);
TextureCube textureCubeBuffer[TEX_BUFFER_SIZE] : register(t16);

SamplerState normalMapSampler : register(s0);

cbuffer WaterBuffer : register(b0)
{
    // 64 bytes
    matrix projection;
    // 16 bytes
    float4 deepColour;
    // 16 bytes
    float4 shallowColour;
    // 16 bytes
    float3 cameraPos;
    float depthMultiplier;
    // 16 bytes
    float3 oceanBoundsMin;
    float alphaMultiplier;
    // 16 bytes
    float3 oceanBoundsMax;
    int rtColourMapIndex;
    // 16 bytes
    int rtDepthMapIndex;
    int normalMapAIndex;
    int normalMapBIndex;
    float normalMapScale;
    // 16 bytes
    float normalMapStrength;
    float smoothness;
    float time;
    float padding0;
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
        
        float depthThroughWater = min(depthToScene - distToOcean , distThroughOcean) ;
        
        float tDepth = 1 - exp(-depthMultiplier * depthThroughWater);
        float tAlpha = 1 - exp(-alphaMultiplier * depthThroughWater);
        
        float4 waterColour = lerp(shallowColour, deepColour, tDepth);
        
        // normal mapping
        
        // work out where on the ocean surface this fragment lies
        float3 intersectionPoint = cameraPos + input.viewVector * distToOcean;
        // find out the uv of the intersection point
        float2 uv = ((intersectionPoint - oceanBoundsMin) / (oceanBoundsMax - oceanBoundsMin)).xz;
        uv.y = 1 - uv.y;
        
        // this is not ideal
        const float3 normalW = float3(0, 1, 0);
        
        float3 normalMapASample = SampleTexture2D(texture2DBuffer, normalMapAIndex, normalMapSampler,
                                                    (uv * normalMapScale) + float2(0.1f * time, 0.08f * time)).rgb;
        float3 normalMapBSample = SampleTexture2D(texture2DBuffer, normalMapAIndex, normalMapSampler,
                                                    (uv * normalMapScale) + float2(-0.08f * time, -0.1f * time)).rgb;
        
        // convert normals to world space
        float3 bumpedNormal = tangentToWorld(normalMapASample, normalW, input.viewVector, input.tex);
        bumpedNormal = tangentToWorld(normalMapBSample, bumpedNormal, input.viewVector, input.tex);
        
        colour = lerp(colour, waterColour, tAlpha);
    }
    
    return colour;
}
