#include "common.hlsli"

Texture2D input : register(t0);
RWTexture2D<float4> output : register(u0);

SamplerState samplerState : register(s0);

cbuffer CSBuffer : register(b0)
{
    float threshold;
    float smoothing;
    float2 padding;
}


[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 dims;
    output.GetDimensions(dims.x, dims.y);
    
    if (DTid.x >= dims.x || DTid.y >= dims.y)
        return;

    float2 uv = (float2) (DTid.xy) / (float2) (dims - uint2(1, 1));

    // sample texture
    float4 s = input.SampleLevel(samplerState, uv, 0.0f);
    float lum = dot(s, LUM_VECTOR);
    
    // perform brightpass filter
    float filteredLum = smoothstep(threshold - smoothing, threshold + smoothing, lum);
    
    // store in output texture
    float3 outColor = s.rgb * (filteredLum / lum);
    output[DTid.xy] = float4(outColor, 1.0f);
}
