#include "common.hlsli"

Texture2D heightmap : register(t0);
SamplerState heightmapSampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
}

cbuffer LightBuffer : register(b1)
{
    VSLightBuffer lightBuffer;
};


struct DSOutput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION0;
    float3 viewDir : POSITION1;
    float4 lightViewPos[MAX_LIGHTS] : POSITION2;
};

struct HSControlPointOutput
{
    float3 position : WORLD_SPACE_CONTROL_POINT_POSITION;
    float2 tex : CONTROL_POINT_TEXCOORD;
    float3 normal : CONTROL_POINT_NORMAL;
};

struct HSConstantOutput
{
    float edgeTessFactor[4] : SV_TessFactor;
    float insideTessFactor[2] : SV_InsideTessFactor;
};


float GetHeight(float2 pos)
{
    return heightmap.SampleLevel(heightmapSampler, pos, 0).r;
}


[domain("quad")]
DSOutput main(HSConstantOutput input, float2 domainUV : SV_DomainLocation, const OutputPatch<HSControlPointOutput, 4> patch)
{
	DSOutput output;
    
    output.worldPosition = lerp(lerp(patch[0].position, patch[1].position, domainUV.x),
                                lerp(patch[2].position, patch[3].position, domainUV.x),
                                domainUV.y);
    
    output.tex = lerp(lerp(patch[0].tex, patch[1].tex, domainUV.x),
                      lerp(patch[2].tex, patch[3].tex, domainUV.x),
                      domainUV.y);
    
    output.normal = lerp(lerp(patch[0].normal, patch[1].normal, domainUV.x),
                         lerp(patch[2].normal, patch[3].normal, domainUV.x),
                         domainUV.y);
    output.normal = normalize(output.normal);
    
    // apply displacement map
    output.worldPosition += output.normal * GetHeight(output.tex);

    // transform vertex
    output.position = mul(float4(output.worldPosition, 1.0f), viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // get position from pov of each light for shadow mapping
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lightBuffer.lightPosAndType[i].w == LIGHT_TYPE_POINT)
        {
            float3 toFrag = normalize(output.worldPosition - lightBuffer.lightPosAndType[i].xyz);
            float4 viewPos = mul(float4(output.worldPosition, 1.0f), GetPointLightViewMatrix(i, toFrag, lightBuffer.pointLightMatrices));
            output.lightViewPos[i] = mul(viewPos, lightBuffer.lightMatrix[i]);
        }
        else
            output.lightViewPos[i] = mul(float4(output.worldPosition, 1.0f), lightBuffer.lightMatrix[i]);
    }
    
    // view dir is direction from camera to vertex
    output.viewDir = output.worldPosition - lightBuffer.cameraPos;
    
    return output;
}
