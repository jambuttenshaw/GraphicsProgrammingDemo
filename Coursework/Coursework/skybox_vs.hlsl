cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
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
    float3 objectSpacePos : POSITION;
};

OutputType main(InputType input)
{
    OutputType output;
    
    output.position = mul(mul(mul(input.position, worldMatrix), viewMatrix), projectionMatrix).xyww;
    output.objectSpacePos = input.position.xyz;
    
    return output;
}