
StructuredBuffer<float> input : register(t0);
RWStructuredBuffer<float> output : register(u0);

cbuffer CSBuffer : register(b0)
{
    uint2 inputDims;
    uint2 groupCount;
}

#define groupthreads 128

groupshared float accum[groupthreads];

bool IsNaN(float x)
{
    return (asuint(x) & 0x7fffffff) > 0x7f800000;
}

[numthreads(groupthreads, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex)
{
    if (DTid.x < inputDims.x)
        accum[GI] = input[DTid.x];
    else
        accum[GI] = 0;
    
    // parallel reduction algorithm (unrolled)
    GroupMemoryBarrierWithGroupSync();
    if (GI < 64)
        accum[GI] += accum[64 + GI];
    
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
        if (IsNaN(accum[0]))
            output[Gid.y * groupCount.x + Gid.x] = 0;
        else
            output[Gid.y * groupCount.x + Gid.x] = accum[0];
    }
}