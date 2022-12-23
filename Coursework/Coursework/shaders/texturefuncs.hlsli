#include "defines.hlsli"

// Texture helpers

float4 SampleTexture2D(Texture2D texBuffer[TEX_BUFFER_SIZE], int i, SamplerState s, float2 uv)
{
    switch (i)
    {
        case 0:
            return texBuffer[0].Sample(s, uv);
        case 1:
            return texBuffer[1].Sample(s, uv);
        case 2:
            return texBuffer[2].Sample(s, uv);
        case 3:
            return texBuffer[3].Sample(s, uv);
        case 4:
            return texBuffer[4].Sample(s, uv);
        case 5:
            return texBuffer[5].Sample(s, uv);
        case 6:
            return texBuffer[6].Sample(s, uv);
        case 7:
            return texBuffer[7].Sample(s, uv);
        case 8:
            return texBuffer[8].Sample(s, uv);
        case 9:
            return texBuffer[9].Sample(s, uv);
        case 10:
            return texBuffer[10].Sample(s, uv);
        case 11:
            return texBuffer[11].Sample(s, uv);
        case 12:
            return texBuffer[12].Sample(s, uv);
        case 13:
            return texBuffer[13].Sample(s, uv);
        case 14:
            return texBuffer[14].Sample(s, uv);
        case 15:
            return texBuffer[15].Sample(s, uv);
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float4 SampleTextureCube(TextureCube texBuffer[TEX_BUFFER_SIZE], int i, SamplerState s, float3 uvw)
{
    switch (i)
    {
        case 0:
            return texBuffer[0].Sample(s, uvw);
        case 1:
            return texBuffer[1].Sample(s, uvw);
        case 2:
            return texBuffer[2].Sample(s, uvw);
        case 3:
            return texBuffer[3].Sample(s, uvw);
        case 4:
            return texBuffer[4].Sample(s, uvw);
        case 5:
            return texBuffer[5].Sample(s, uvw);
        case 6:
            return texBuffer[6].Sample(s, uvw);
        case 7:
            return texBuffer[7].Sample(s, uvw);
        case 8:
            return texBuffer[7].Sample(s, uvw);
        case 9:
            return texBuffer[7].Sample(s, uvw);
        case 10:
            return texBuffer[7].Sample(s, uvw);
        case 11:
            return texBuffer[7].Sample(s, uvw);
        case 12:
            return texBuffer[7].Sample(s, uvw);
        case 13:
            return texBuffer[7].Sample(s, uvw);
        case 14:
            return texBuffer[7].Sample(s, uvw);
        case 15:
            return texBuffer[7].Sample(s, uvw);
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 SampleTexture2DLOD(Texture2D texBuffer[TEX_BUFFER_SIZE], int i, SamplerState s, float2 uv, float lod)
{
    switch (i)
    {
        case 0:
            return texBuffer[0].SampleLevel(s, uv, lod);
        case 1:
            return texBuffer[1].SampleLevel(s, uv, lod);
        case 2:
            return texBuffer[2].SampleLevel(s, uv, lod);
        case 3:
            return texBuffer[3].SampleLevel(s, uv, lod);
        case 4:
            return texBuffer[4].SampleLevel(s, uv, lod);
        case 5:
            return texBuffer[5].SampleLevel(s, uv, lod);
        case 6:
            return texBuffer[6].SampleLevel(s, uv, lod);
        case 7:
            return texBuffer[7].SampleLevel(s, uv, lod);
        case 8:
            return texBuffer[8].SampleLevel(s, uv, lod);
        case 9:
            return texBuffer[9].SampleLevel(s, uv, lod);
        case 10:
            return texBuffer[10].SampleLevel(s, uv, lod);
        case 11:
            return texBuffer[11].SampleLevel(s, uv, lod);
        case 12:
            return texBuffer[12].SampleLevel(s, uv, lod);
        case 13:
            return texBuffer[13].SampleLevel(s, uv, lod);
        case 14:
            return texBuffer[14].SampleLevel(s, uv, lod);
        case 15:
            return texBuffer[15].SampleLevel(s, uv, lod);
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float4 SampleTextureCubeLOD(TextureCube texBuffer[TEX_BUFFER_SIZE], int i, SamplerState s, float3 uvw, float lod)
{
    switch (i)
    {
        case 0:
            return texBuffer[0].SampleLevel(s, uvw, lod);
        case 1:
            return texBuffer[1].SampleLevel(s, uvw, lod);
        case 2:
            return texBuffer[2].SampleLevel(s, uvw, lod);
        case 3:
            return texBuffer[3].SampleLevel(s, uvw, lod);
        case 4:
            return texBuffer[4].SampleLevel(s, uvw, lod);
        case 5:
            return texBuffer[5].SampleLevel(s, uvw, lod);
        case 6:
            return texBuffer[6].SampleLevel(s, uvw, lod);
        case 7:
            return texBuffer[7].SampleLevel(s, uvw, lod);
        case 8:
            return texBuffer[8].SampleLevel(s, uvw, lod);
        case 9:
            return texBuffer[9].SampleLevel(s, uvw, lod);
        case 10:
            return texBuffer[10].SampleLevel(s, uvw, lod);
        case 11:
            return texBuffer[11].SampleLevel(s, uvw, lod);
        case 12:
            return texBuffer[12].SampleLevel(s, uvw, lod);
        case 13:
            return texBuffer[13].SampleLevel(s, uvw, lod);
        case 14:
            return texBuffer[14].SampleLevel(s, uvw, lod);
        case 15:
            return texBuffer[15].SampleLevel(s, uvw, lod);
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float4 SampleTexture2DComp(Texture2D texBuffer[TEX_BUFFER_SIZE], int i, SamplerComparisonState s, float2 uv, float compareValue)
{
    switch (i)
    {
        case 0:
            return texBuffer[0].SampleCmpLevelZero(s, uv, compareValue);
        case 1:
            return texBuffer[1].SampleCmpLevelZero(s, uv, compareValue);
        case 2:
            return texBuffer[2].SampleCmpLevelZero(s, uv, compareValue);
        case 3:
            return texBuffer[3].SampleCmpLevelZero(s, uv, compareValue);
        case 4:
            return texBuffer[4].SampleCmpLevelZero(s, uv, compareValue);
        case 5:
            return texBuffer[5].SampleCmpLevelZero(s, uv, compareValue);
        case 6:
            return texBuffer[6].SampleCmpLevelZero(s, uv, compareValue);
        case 7:
            return texBuffer[7].SampleCmpLevelZero(s, uv, compareValue);
        case 8:
            return texBuffer[8].SampleCmpLevelZero(s, uv, compareValue);
        case 9:
            return texBuffer[9].SampleCmpLevelZero(s, uv, compareValue);
        case 10:
            return texBuffer[10].SampleCmpLevelZero(s, uv, compareValue);
        case 11:
            return texBuffer[11].SampleCmpLevelZero(s, uv, compareValue);
        case 12:
            return texBuffer[12].SampleCmpLevelZero(s, uv, compareValue);
        case 13:
            return texBuffer[13].SampleCmpLevelZero(s, uv, compareValue);
        case 14:
            return texBuffer[14].SampleCmpLevelZero(s, uv, compareValue);
        case 15:
            return texBuffer[15].SampleCmpLevelZero(s, uv, compareValue);
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float4 SampleTextureCubeComp(TextureCube texBuffer[TEX_BUFFER_SIZE], int i, SamplerComparisonState s, float3 uvw, float compareValue)
{
    switch (i)
    {
        case 0:
            return texBuffer[0].SampleCmpLevelZero(s, uvw, compareValue);
        case 1:
            return texBuffer[1].SampleCmpLevelZero(s, uvw, compareValue);
        case 2:
            return texBuffer[2].SampleCmpLevelZero(s, uvw, compareValue);
        case 3:
            return texBuffer[3].SampleCmpLevelZero(s, uvw, compareValue);
        case 4:
            return texBuffer[4].SampleCmpLevelZero(s, uvw, compareValue);
        case 5:
            return texBuffer[5].SampleCmpLevelZero(s, uvw, compareValue);
        case 6:
            return texBuffer[6].SampleCmpLevelZero(s, uvw, compareValue);
        case 7:
            return texBuffer[7].SampleCmpLevelZero(s, uvw, compareValue);
        case 8:
            return texBuffer[8].SampleCmpLevelZero(s, uvw, compareValue);
        case 9:
            return texBuffer[9].SampleCmpLevelZero(s, uvw, compareValue);
        case 10:
            return texBuffer[10].SampleCmpLevelZero(s, uvw, compareValue);
        case 11:
            return texBuffer[11].SampleCmpLevelZero(s, uvw, compareValue);
        case 12:
            return texBuffer[12].SampleCmpLevelZero(s, uvw, compareValue);
        case 13:
            return texBuffer[13].SampleCmpLevelZero(s, uvw, compareValue);
        case 14:
            return texBuffer[14].SampleCmpLevelZero(s, uvw, compareValue);
        case 15:
            return texBuffer[15].SampleCmpLevelZero(s, uvw, compareValue);
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 LoadTexture2D(Texture2D texBuffer[TEX_BUFFER_SIZE], int i, int2 pos)
{
    switch (i)
    {
        case 0:
            return texBuffer[0].Load(int3(pos, 0));
        case 1:
            return texBuffer[1].Load(int3(pos, 0));
        case 2:
            return texBuffer[2].Load(int3(pos, 0));
        case 3:
            return texBuffer[3].Load(int3(pos, 0));
        case 4:
            return texBuffer[4].Load(int3(pos, 0));
        case 5:
            return texBuffer[5].Load(int3(pos, 0));
        case 6:
            return texBuffer[6].Load(int3(pos, 0));
        case 7:
            return texBuffer[7].Load(int3(pos, 0));
        case 8:
            return texBuffer[8].Load(int3(pos, 0));
        case 9:
            return texBuffer[9].Load(int3(pos, 0));
        case 10:
            return texBuffer[10].Load(int3(pos, 0));
        case 11:
            return texBuffer[11].Load(int3(pos, 0));
        case 12:
            return texBuffer[12].Load(int3(pos, 0));
        case 13:
            return texBuffer[13].Load(int3(pos, 0));
        case 14:
            return texBuffer[14].Load(int3(pos, 0));
        case 15:
            return texBuffer[15].Load(int3(pos, 0));
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}