
TextureCube skybox : register(t0);
SamplerState skyboxSampler : register(s0);


struct InputType
{
    float4 position : SV_POSITION;
    float3 objectSpacePos : POSITION;
};


float4 main(InputType input) : SV_TARGET
{
    return skybox.Sample(skyboxSampler, input.objectSpacePos);
}
