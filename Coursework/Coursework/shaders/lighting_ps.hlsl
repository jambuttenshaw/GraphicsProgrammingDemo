#include "lighting/lighting.hlsli"

#define MAX_LIGHTS 4

cbuffer LightBuffer : register(b0)
{
    float4 irradiance[MAX_LIGHTS];
    float4 lightDirectionAndType[MAX_LIGHTS];
	float lightCount;
};

cbuffer MaterialBuffer : register(b1)
{
	float4 albedo;
	float metallic;
	float roughness;
	float2 padding1;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewDir : POSITION;
};


float4 main(InputType input) : SV_TARGET
{
	float3 l0 = float3(0.0f, 0.0f, 0.0f);

	float3 n = normalize(input.normal);
	float3 v = -normalize(input.viewDir);
	
	for (int i = 0; i < lightCount; i++)
	{
		// calculate light direction and irradiance
        float3 l;
        float3 el;
		
        switch (lightDirectionAndType[i].w)
        {
		case 0:
			l = -normalize(lightDirectionAndType[i].xyz);
			el = irradiance[i].rgb;
            break;
        }
    
		// evaluate shading equation
		l0 += ggx_brdf(v, l, n, albedo.rgb, roughness, metallic) * el * saturate(dot(n, l));
	}

	return float4(l0, 1.0f);
}
