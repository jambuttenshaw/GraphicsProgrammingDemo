Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

cbuffer LightMatrixBuffer : register(b0)
{
    Matrix projection;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float calculateLinearDepth(float z)
{
    return projection._43 / (z - projection._33);
}

float4 main(InputType input) : SV_TARGET
{
    float depth = texture0.Sample(Sampler0, input.tex).r;
    float linearDepth = calculateLinearDepth(depth);
    float c = linearDepth;
    return float4(c, c, c, 1.0f);
}