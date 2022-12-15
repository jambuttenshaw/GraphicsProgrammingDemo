#include "defines.hlsli"

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
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

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : POSITION0;
    float3 viewDir : POSITION1;
    float4 lightViewPos[MAX_LIGHTS] : POSITION2;
};


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


OutputType main(InputType input)
{
    OutputType output;
	
	// Calculate the position of the vertex against the world, view, and projection matrices.
    float4 worldPos = mul(input.position, worldMatrix);
    output.worldPos = worldPos.xyz;
    output.position = mul(mul(worldPos, viewMatrix), projectionMatrix);
    
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lightPosAndType[i].w == LIGHT_TYPE_POINT)
        {
            float3 toFrag = normalize(output.worldPos - lightPosAndType[i].xyz);
            float4 viewPos = mul(worldPos, GetPointLightViewMatrix(i, toFrag));
            output.lightViewPos[i] = mul(viewPos, lightMatrix[i]);
        }
        else
            output.lightViewPos[i] = mul(worldPos, lightMatrix[i]);
    }
    
	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

	// transform the normal into world space
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    // normal will be normalized in pixel shader post-interpolation
	
    // view dir is direction from camera to vertex
    output.viewDir = output.worldPos - cameraPos;
    // view dir will be normalized in pixel shader post-interpolation

    return output;
}