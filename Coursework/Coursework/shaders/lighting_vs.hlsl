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
        if (lightPosAndType[i].w == LIGHT_TYPE_POINT)
        {
            float distToLight = length(lightPosAndType[i].xyz - output.worldPos);
            float4 viewPos = float4(0.0f, 0.0f, distToLight, 1.0f);
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