#include "defines.hlsli"

Texture2D heightmap : register(t0);
SamplerState heightmapSampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
}

cbuffer CameraBuffer : register(b1)
{
    matrix lightMatrix[MAX_LIGHTS];
    float4 lightPosAndType[MAX_LIGHTS];
    float3 cameraPos;
    float padding;
};

cbuffer PointLightViewMatrices : register(b2)
{
    matrix viewRight[MAX_LIGHTS];
    matrix viewLeft[MAX_LIGHTS];
    matrix viewUp[MAX_LIGHTS];
    matrix viewDown[MAX_LIGHTS];
    matrix viewForward[MAX_LIGHTS];
    matrix viewBack[MAX_LIGHTS];
}


struct DSOutput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : POSITION0;
    float3 viewDir : POSITION1;
    float4 lightViewPos[MAX_LIGHTS] : POSITION2;
};

struct HSControlPointOutput
{
    float3 position : WORLD_SPACE_CONTROL_POINT_POSITION;
    float2 tex : CONTROL_POINT_TEXCOORD;
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


matrix GetPointLightViewMatrix(int i, float3 toFrag)
{
    float3 absL = abs(toFrag);
    float maxComponent = max(absL.x, max(absL.y, absL.z));
    if (maxComponent == absL.x)
    {
        if (toFrag.x > 0)
            return viewRight[i];
        else
            return viewLeft[i];
    }
    else if (maxComponent == absL.y)
    {
        if (toFrag.y > 0)
            return viewUp[i];
        else
            return viewDown[i];
    }
    else
    {
        if (toFrag.z > 0)
            return viewForward[i];
        else
            return viewBack[i];
    }
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
    
    // apply displacement map
    output.worldPosition.y += GetHeight(output.tex);

    // transform vertex
    output.position = mul(float4(output.worldPosition, 1.0f), viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // get position from pov of each light for shadow mapping
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lightPosAndType[i].w == LIGHT_TYPE_POINT)
        {
            float3 toFrag = normalize(output.worldPosition - lightPosAndType[i].xyz);
            float4 viewPos = mul(float4(output.worldPosition, 1.0f), GetPointLightViewMatrix(i, toFrag));
            output.lightViewPos[i] = mul(viewPos, lightMatrix[i]);
        }
        else
            output.lightViewPos[i] = mul(float4(output.worldPosition, 1.0f), lightMatrix[i]);
    }
    
    // view dir is direction from camera to vertex
    output.viewDir = output.worldPosition - cameraPos;
    
    return output;
}
