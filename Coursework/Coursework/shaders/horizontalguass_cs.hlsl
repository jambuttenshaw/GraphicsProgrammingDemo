
Texture2D input : register(t0);
RWTexture2D<float4> output : register(u0);

SamplerState samplerState : register(s0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 dims;
    output.GetDimensions(dims.x, dims.y);
    
    if (DTid.x >= dims.x || DTid.y >= dims.y)
        return;

    float2 uv = (float2) (DTid.xy) / (float2) (dims - uint2(1, 1));
    
	// Create the weights that each neighbor pixel will contribute to the blur.
    float weight0 = 0.4062f;
    float weight1 = 0.2442f;
    float weight2 = 0.0545f;

    // Initialize the colour to black.
    float4 colour = float4(0.0f, 0.0f, 0.0f, 0.0f);

    uint2 inDims;
    input.GetDimensions(inDims.x, inDims.y);
    float inTexelSize = 1.0f / float(inDims.x);
    
    // Add the vertical pixels to the colour by the specific weight of each.
    colour += input.SampleLevel(samplerState, uv + float2(inTexelSize * -2.0f, 0.0f), 0) * weight2;
    colour += input.SampleLevel(samplerState, uv + float2(inTexelSize * -1.0f, 0.0f), 0) * weight1;
    colour += input.SampleLevel(samplerState, uv, 0) * weight0;       
    colour += input.SampleLevel(samplerState, uv + float2(inTexelSize * 1.0f, 0.0f), 0) * weight1;
    colour += input.SampleLevel(samplerState, uv + float2(inTexelSize * 2.0f, 0.0f), 0) * weight2;
 
    // Set the alpha channel to one.
    colour.a = 1.0f;

    output[DTid.xy] = colour;
}