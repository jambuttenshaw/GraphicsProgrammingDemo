
Texture2D renderTextureColour : register(t0);
Texture2D renderTextureDepth : register(t1);
StructuredBuffer<float> lum : register(t2);

cbuffer Params : register(b0)
{
    float avgLumFactor; // avgLum = lum[0] * avgLumFactor
	float w;	// white point
	float b;	// black point
	float t;	// toe amount
	float s;	// shoulder amount
	float c;	// cross over point
	
	float2 padding;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD;
	float3 viewVector : POSITION0;
};

float CalculateK()
{
	float num = (1.0f - t) * (c - b);
	float denom = (1.0f - s) * (w - c) + num;
	return num / denom;
}

float Remap(float x, float cross_over_point, float4 toe_coeffs, float4 shoulder_coeffs)
{
	float4 coeffs = (x < cross_over_point) ? toe_coeffs : shoulder_coeffs;
	float2 fraction = coeffs.xy * x + coeffs.zw;
	return fraction.x / fraction.y;
}

static const float MIDDLE_GRAY = 0.72f;
static const float LUM_WHITE = 1.5f;

float4 main(InputType input) : SV_TARGET
{
    float3 color = renderTextureColour[input.position.xy].rgb;
	
	/*
	float k = CalculateK();
	
	float4 toe_coeffs =
	{ 
		k * (1.0f - t),
		k * b * (t - 1.0f),
		-t,
		c - b * (1.0f - t)
	};
	float4 shoulder_coeffs =
	{
		1.0f + k * (s - 1.0f),
		k * w * (1.0f - s),
		s,
		w * (1.0f - s) - c
	};
	
	color.r = Remap(color.r, c, toe_coeffs, shoulder_coeffs);
	color.g = Remap(color.g, c, toe_coeffs, shoulder_coeffs);
	color.b = Remap(color.b, c, toe_coeffs, shoulder_coeffs);
	*/
	
    float fLum = lum[0] * avgLumFactor;
	
    color *= MIDDLE_GRAY / (fLum + 0.001f);
    color *= (1.0f + color / LUM_WHITE);
    color /= (1.0f + color);
	
	return float4(color, 1.0f);
}
