
#include "defines.hlsli"


Texture2D input : register(t0);
RWStructuredBuffer<float> output : register(u0);

cbuffer CSBuffer : register(b0)
{
    uint2 inputDims;
    uint2 groupCount;
}

#define groupthreadsX 8
#define groupthreadsY 8
#define groupthreads (groupthreadsX * groupthreadsY)

groupshared float accum[groupthreads];

[numthreads(groupthreadsX, groupthreadsY, 1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex )
{
    float4 s = input.Load(uint3(DTid.xy, 0));
    
    // calculate luminance and store in group shared memory
    accum[GI] = dot(s, LUM_VECTOR);
    
    // parallel reduction algorithm (unrolled)
    GroupMemoryBarrierWithGroupSync();
    if (GI < 32)
        accum[GI] += accum[32 + GI];
    
    GroupMemoryBarrierWithGroupSync();
    if (GI < 16)
        accum[GI] += accum[16 + GI];
    
    GroupMemoryBarrierWithGroupSync();
    if (GI < 8)
        accum[GI] += accum[8 + GI];
    
    GroupMemoryBarrierWithGroupSync();
    if (GI < 4)
        accum[GI] += accum[4 + GI];
    
    GroupMemoryBarrierWithGroupSync();
    if (GI < 2)
        accum[GI] += accum[2 + GI];
    
    GroupMemoryBarrierWithGroupSync();
    if (GI < 1)
        accum[GI] += accum[1 + GI];

    if (GI == 0)
    {
        output[Gid.y * groupCount.x + Gid.x] = accum[0];
    }
}