#include "common.hlsli"

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
}

cbuffer LightCB : register(b1)
{
    VSLightBuffer lightBuffer;
};



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


OutputType main(InputType input)
{
    OutputType output;
	
	// Calculate the position of the vertex against the world, view, and projection matrices.
    float4 worldPos = mul(input.position, worldMatrix);
    output.worldPos = worldPos.xyz;
    output.position = mul(mul(worldPos, viewMatrix), projectionMatrix);
    
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lightBuffer.lightPosAndType[i].w == LIGHT_TYPE_POINT)
        {
            float3 toFrag = normalize(output.worldPos - lightBuffer.lightPosAndType[i].xyz);
            float4 viewPos = mul(worldPos, GetPointLightViewMatrix(i, toFrag, lightBuffer.pointLightMatrices));
            output.lightViewPos[i] = mul(viewPos, lightBuffer.lightMatrix[i]);
        }
        else
            output.lightViewPos[i] = mul(worldPos, lightBuffer.lightMatrix[i]);
    }
    
	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

	// transform the normal into world space
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    // normal will be normalized in pixel shader post-interpolation
	
    // view dir is direction from camera to vertex
    output.viewDir = output.worldPos - lightBuffer.cameraPos;
    // view dir will be normalized in pixel shader post-interpolation

    return output;
}