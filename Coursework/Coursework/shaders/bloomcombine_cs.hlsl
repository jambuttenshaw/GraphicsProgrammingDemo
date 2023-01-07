
Texture2D input : register(t0);
RWTexture2D<float4> output : register(u0);

SamplerState samplerState : register(s0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 dims;
    output.GetDimensions(dims.x, dims.y);
    
    if (DTid.x >= dims.x || DTid.y >= dims.y)
        return;

    float2 uv = (float2) (DTid.xy) / (float2) (dims - uint2(1, 1));
    float4 s = input.SampleLevel(samplerState, uv, 0.0f);
    
    output[DTid.xy] += s;
}