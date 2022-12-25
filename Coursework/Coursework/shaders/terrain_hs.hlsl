#include "math.hlsli"

// rgb = best fit plane normal
//   a = standard deviation from best fit plane
Texture2D preprocessedHeightmap : register(t0);
SamplerState pointSampler : register(s0);

cbuffer TessellationBuffer : register(b0)
{
    float2 minMaxDistance;
    float2 minMaxHeightDeviation;
    float2 minMaxLOD;
    float distanceLODBlending;
    float padding;
    float3 cameraPos;
    float size;
}
   

struct VSOutputType
{
    float3 position : WORLD_SPACE_CONTROL_POINT_POSITION;
    float2 tex : CONTROL_POINT_TEXCOORD;
};

struct HSControlPointOutput
{
    float3 position : WORLD_SPACE_CONTROL_POINT_POSITION;
    float2 tex : CONTROL_POINT_TEXCOORD;
};

struct HSConstantOutput
{
	float edgeTessFactor[4] : SV_TessFactor;
	float insideTessFactor[2] : SV_InsideTessFactor;
};


float3 ComputePatchMidpoint(float3 cp0, float3 cp1, float3 cp2, float3 cp3)
{
    return (cp0 + cp1 + cp2 + cp3) / 4.0f;
}

float ComputeScaledDistance(float3 from, float3 to)
{
    float d = distance(from, to);
    return smoothstep(minMaxDistance.x, minMaxDistance.y, d);
}

float ComputePatchLOD(float3 midpoint)
{
    float2 uv = midpoint.xz / size;
    uv += float2(0.5f, 0.5f);
    
    float heightDeviation = preprocessedHeightmap.SampleLevel(pointSampler, uv, 0.0f).a;
    float scaledHeightDeviation = smoothstep(minMaxHeightDeviation.x, minMaxHeightDeviation.y, heightDeviation);
    
    float d = ComputeScaledDistance(cameraPos, midpoint);
    
    float lod01 = saturate(scaledHeightDeviation + distanceLODBlending * (1.0f - d));
    return lerp(minMaxLOD.x, minMaxLOD.y, lod01);
}


HSConstantOutput PatchConstantFunction(InputPatch<VSOutputType, 12> ip, uint patchID : SV_PrimitiveID)
{
    HSConstantOutput output;
    
	// determine the midpoint of this and surrounding patches
    float3 midPoints[] =
    {
        // main quad
        ComputePatchMidpoint(ip[0].position, ip[1].position, ip[2].position, ip[3].position),

        // +x neighbour
		ComputePatchMidpoint(ip[2].position, ip[3].position, ip[4].position, ip[5].position),

        // +z neighbour
		ComputePatchMidpoint(ip[1].position, ip[3].position, ip[6].position, ip[7].position),

        // -x neighbour
		ComputePatchMidpoint(ip[0].position, ip[1].position, ip[8].position, ip[9].position),

        // -z neighbour
		ComputePatchMidpoint(ip[0].position, ip[2].position, ip[10].position, ip[11].position)
    };

    float lod[] =
    {
        ComputePatchLOD(midPoints[0]),
        ComputePatchLOD(midPoints[1]),
        ComputePatchLOD(midPoints[2]),
        ComputePatchLOD(midPoints[3]),
        ComputePatchLOD(midPoints[4])
    };

    output.insideTessFactor[0] = lod[0];
    output.insideTessFactor[1] = lod[0];

    output.edgeTessFactor[0] = min(lod[0], lod[4]);
    output.edgeTessFactor[1] = min(lod[0], lod[3]);
    output.edgeTessFactor[2] = min(lod[0], lod[2]);
    output.edgeTessFactor[3] = min(lod[0], lod[1]);
    
    return output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
HSControlPointOutput main(InputPatch<VSOutputType, 12> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
    HSControlPointOutput output;

    output.position = ip[i].position;
    output.tex = ip[i].tex;

    return output;
}
