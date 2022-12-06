
Texture2D renderTextureColour : register(t0);
Texture2D renderTextureDepth : register(t1);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD;
    float3 viewVector : POSITION0;
};

float4 main(InputType input) : SV_TARGET
{
    float3 c = renderTextureColour[input.position.xy].rgb;
    
    return float4(c, 1.0f);
}