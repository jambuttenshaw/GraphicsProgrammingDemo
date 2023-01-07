#include "defines.hlsli"

// Texture helpers

float4 SampleTexture2D(Texture2D texBuffer[TEX_BUFFER_SIZE], int i, SamplerState s, float2 uv)
{
    #define CASESAMPLETEX2D(n) case n: return texBuffer[n].Sample(s, uv);
    switch (i)
    {
        CASESAMPLETEX2D( 0) CASESAMPLETEX2D( 1) CASESAMPLETEX2D( 2) CASESAMPLETEX2D( 3)
        CASESAMPLETEX2D( 4) CASESAMPLETEX2D( 5) CASESAMPLETEX2D( 6) CASESAMPLETEX2D( 7)
        CASESAMPLETEX2D( 8) CASESAMPLETEX2D( 9) CASESAMPLETEX2D(10) CASESAMPLETEX2D(11)
        CASESAMPLETEX2D(12) CASESAMPLETEX2D(13) CASESAMPLETEX2D(14) CASESAMPLETEX2D(15)
        CASESAMPLETEX2D(16) CASESAMPLETEX2D(17) CASESAMPLETEX2D(18) CASESAMPLETEX2D(19)
        CASESAMPLETEX2D(20) CASESAMPLETEX2D(21) CASESAMPLETEX2D(22) CASESAMPLETEX2D(23)
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float4 SampleTextureCube(TextureCube texBuffer[TEX_BUFFER_SIZE], int i, SamplerState s, float3 uvw)
{
    #define CASESAMPLETEXCUBE(n) case n: return texBuffer[n].Sample(s, uvw);
    switch (i)
    {
        CASESAMPLETEXCUBE( 0) CASESAMPLETEXCUBE( 1) CASESAMPLETEXCUBE( 2) CASESAMPLETEXCUBE( 3)
        CASESAMPLETEXCUBE( 4) CASESAMPLETEXCUBE( 5) CASESAMPLETEXCUBE( 6) CASESAMPLETEXCUBE( 7)
        CASESAMPLETEXCUBE( 8) CASESAMPLETEXCUBE( 9) CASESAMPLETEXCUBE(10) CASESAMPLETEXCUBE(11)
        CASESAMPLETEXCUBE(12) CASESAMPLETEXCUBE(13) CASESAMPLETEXCUBE(14) CASESAMPLETEXCUBE(15)
        CASESAMPLETEXCUBE(16) CASESAMPLETEXCUBE(17) CASESAMPLETEXCUBE(18) CASESAMPLETEXCUBE(19)
        CASESAMPLETEXCUBE(20) CASESAMPLETEXCUBE(21) CASESAMPLETEXCUBE(22) CASESAMPLETEXCUBE(23)
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 SampleTexture2DLOD(Texture2D texBuffer[TEX_BUFFER_SIZE], int i, SamplerState s, float2 uv, float lod)
{
    #define CASESAMPLELEVELTEX2D(n) case n: return texBuffer[n].SampleLevel(s, uv, lod);
    switch (i)
    {
        CASESAMPLELEVELTEX2D( 0) CASESAMPLELEVELTEX2D( 1) CASESAMPLELEVELTEX2D( 2) CASESAMPLELEVELTEX2D( 3)
        CASESAMPLELEVELTEX2D( 4) CASESAMPLELEVELTEX2D( 5) CASESAMPLELEVELTEX2D( 6) CASESAMPLELEVELTEX2D( 7)
        CASESAMPLELEVELTEX2D( 8) CASESAMPLELEVELTEX2D( 9) CASESAMPLELEVELTEX2D(10) CASESAMPLELEVELTEX2D(11)
        CASESAMPLELEVELTEX2D(12) CASESAMPLELEVELTEX2D(13) CASESAMPLELEVELTEX2D(14) CASESAMPLELEVELTEX2D(15)
        CASESAMPLELEVELTEX2D(16) CASESAMPLELEVELTEX2D(17) CASESAMPLELEVELTEX2D(18) CASESAMPLELEVELTEX2D(19)
        CASESAMPLELEVELTEX2D(20) CASESAMPLELEVELTEX2D(21) CASESAMPLELEVELTEX2D(22) CASESAMPLELEVELTEX2D(23)
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float4 SampleTextureCubeLOD(TextureCube texBuffer[TEX_BUFFER_SIZE], int i, SamplerState s, float3 uvw, float lod)
{
    #define CASESAMPLELEVELTEXCUBE(n) case n: return texBuffer[n].SampleLevel(s, uvw, lod);
    switch (i)
    {
        CASESAMPLELEVELTEXCUBE( 0) CASESAMPLELEVELTEXCUBE( 1) CASESAMPLELEVELTEXCUBE( 2) CASESAMPLELEVELTEXCUBE( 3)
        CASESAMPLELEVELTEXCUBE( 4) CASESAMPLELEVELTEXCUBE( 5) CASESAMPLELEVELTEXCUBE( 6) CASESAMPLELEVELTEXCUBE( 7)
        CASESAMPLELEVELTEXCUBE( 8) CASESAMPLELEVELTEXCUBE( 9) CASESAMPLELEVELTEXCUBE(10) CASESAMPLELEVELTEXCUBE(11)
        CASESAMPLELEVELTEXCUBE(12) CASESAMPLELEVELTEXCUBE(13) CASESAMPLELEVELTEXCUBE(14) CASESAMPLELEVELTEXCUBE(15)
        CASESAMPLELEVELTEXCUBE(16) CASESAMPLELEVELTEXCUBE(17) CASESAMPLELEVELTEXCUBE(18) CASESAMPLELEVELTEXCUBE(19)
        CASESAMPLELEVELTEXCUBE(20) CASESAMPLELEVELTEXCUBE(21) CASESAMPLELEVELTEXCUBE(22) CASESAMPLELEVELTEXCUBE(23)
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float4 SampleTexture2DComp(Texture2D texBuffer[TEX_BUFFER_SIZE], int i, SamplerComparisonState s, float2 uv, float compareValue)
{
    #define CASESAMPLECOMPTEX2D(n) case n: return texBuffer[n].SampleCmpLevelZero(s, uv, compareValue);
    switch (i)
    {
        CASESAMPLECOMPTEX2D( 0) CASESAMPLECOMPTEX2D( 1) CASESAMPLECOMPTEX2D( 2) CASESAMPLECOMPTEX2D( 3)
        CASESAMPLECOMPTEX2D( 4) CASESAMPLECOMPTEX2D( 5) CASESAMPLECOMPTEX2D( 6) CASESAMPLECOMPTEX2D( 7)
        CASESAMPLECOMPTEX2D( 8) CASESAMPLECOMPTEX2D( 9) CASESAMPLECOMPTEX2D(10) CASESAMPLECOMPTEX2D(11)
        CASESAMPLECOMPTEX2D(12) CASESAMPLECOMPTEX2D(13) CASESAMPLECOMPTEX2D(14) CASESAMPLECOMPTEX2D(15)
        CASESAMPLECOMPTEX2D(16) CASESAMPLECOMPTEX2D(17) CASESAMPLECOMPTEX2D(18) CASESAMPLECOMPTEX2D(19)
        CASESAMPLECOMPTEX2D(20) CASESAMPLECOMPTEX2D(21) CASESAMPLECOMPTEX2D(22) CASESAMPLECOMPTEX2D(23)
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float4 SampleTextureCubeComp(TextureCube texBuffer[TEX_BUFFER_SIZE], int i, SamplerComparisonState s, float3 uvw, float compareValue)
{
    #define CASESAMPLECOMPTEXCUBE(n) case n: return texBuffer[n].SampleCmpLevelZero(s, uvw, compareValue);
    switch (i)
    {
        CASESAMPLECOMPTEXCUBE( 0) CASESAMPLECOMPTEXCUBE( 1) CASESAMPLECOMPTEXCUBE( 2) CASESAMPLECOMPTEXCUBE( 3)
        CASESAMPLECOMPTEXCUBE( 4) CASESAMPLECOMPTEXCUBE( 5) CASESAMPLECOMPTEXCUBE( 6) CASESAMPLECOMPTEXCUBE( 7)
        CASESAMPLECOMPTEXCUBE( 8) CASESAMPLECOMPTEXCUBE( 9) CASESAMPLECOMPTEXCUBE(10) CASESAMPLECOMPTEXCUBE(11)
        CASESAMPLECOMPTEXCUBE(12) CASESAMPLECOMPTEXCUBE(13) CASESAMPLECOMPTEXCUBE(14) CASESAMPLECOMPTEXCUBE(15)
        CASESAMPLECOMPTEXCUBE(16) CASESAMPLECOMPTEXCUBE(17) CASESAMPLECOMPTEXCUBE(18) CASESAMPLECOMPTEXCUBE(19)
        CASESAMPLECOMPTEXCUBE(20) CASESAMPLECOMPTEXCUBE(21) CASESAMPLECOMPTEXCUBE(22) CASESAMPLECOMPTEXCUBE(23)
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 LoadTexture2D(Texture2D texBuffer[TEX_BUFFER_SIZE], int i, int2 pos)
{
    #define CASELOADTEX2D(n) case n: return texBuffer[n].Load(int3(pos, 0));
    switch (i)
    {
        CASELOADTEX2D( 0) CASELOADTEX2D( 1) CASELOADTEX2D( 2) CASELOADTEX2D( 3)
        CASELOADTEX2D( 4) CASELOADTEX2D( 5) CASELOADTEX2D( 6) CASELOADTEX2D( 7)
        CASELOADTEX2D( 8) CASELOADTEX2D( 9) CASELOADTEX2D(10) CASELOADTEX2D(11)
        CASELOADTEX2D(12) CASELOADTEX2D(13) CASELOADTEX2D(14) CASELOADTEX2D(15)
        CASELOADTEX2D(16) CASELOADTEX2D(17) CASELOADTEX2D(18) CASELOADTEX2D(19)
        CASELOADTEX2D(20) CASELOADTEX2D(21) CASELOADTEX2D(22) CASELOADTEX2D(23)
        default:
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}