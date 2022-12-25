
Texture2D<float4> heightmap : register(t0);
RWTexture2D<float4> preprocessMap : register(u0);

groupshared float   groupResults[16 * 16];
groupshared float4  plane;
groupshared float3  rawNormals[2][2];
groupshared float3  corners[2][2];

[numthreads(16, 16, 1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex )
{
    // phase one: calculate the height samples at the 4 corners of this group
    if (
        ((GTid.x == 0) && (GTid.y == 0)) ||
        ((GTid.x == 15) && (GTid.y == 0)) ||
        ((GTid.x == 0) && (GTid.y == 15)) ||
        ((GTid.x == 15) && (GTid.y == 15))
    )
    {
        groupResults[GI] = heightmap.Load(uint3(DTid.xy, 0)).r;
        
        uint x = GTid.x / 15;
        uint y = GTid.y / 15;
        corners[x][y] = float3(x, groupResults[GI], y);
        corners[x][y].xz /= 64.0f;
    }

    GroupMemoryBarrierWithGroupSync();
    
    // phase 2: the corner threads will calculate their normals while all other threads sample the heightmap
    if (((GTid.x == 0) && (GTid.y == 0)))
    {
        rawNormals[0][0] = normalize(cross(
                                corners[0][1] - corners[0][0],
                                corners[1][0] - corners[0][0]
                            ));
    }
    else if ((GTid.x == 15) && (GTid.y == 0))
    {
        rawNormals[1][0] = normalize(cross(
                                corners[0][0] - corners[1][0],
                                corners[1][1] - corners[1][0]
                            ));
    }
    else if ((GTid.x == 0) && (GTid.y == 15))
    {
        rawNormals[0][1] = normalize(cross(
                                corners[1][1] - corners[0][1],
                                corners[0][0] - corners[0][1]
                            ));
    }
    else if ((GTid.x == 15) && (GTid.y == 15))
    {
        rawNormals[1][1] = normalize(cross(
                                corners[1][0] - corners[1][1],
                                corners[0][1] - corners[1][1]
                            ));
    }
    else
    {
        groupResults[GI] = heightmap.Load(uint3(DTid.xy, 0)).r;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    // phase 3: calculate plane equation from the average of the 4 normals
    
    if (GI == 0)
    {
        float3 n = normalize(
                        rawNormals[0][0] +
                        rawNormals[0][1] +
                        rawNormals[1][0] +
                        rawNormals[1][1]
                    );

        float3 p = corners[0][0];
        if (corners[0][1].y < p.y)
            p = corners[0][1];
        if (corners[1][0].y < p.y)
            p = corners[1][0];
        if (corners[1][1].y < p.y)
            p = corners[1][1];
        
        plane = float4(n, -dot(n, p));
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    // phase 4: calculate the the distance from the plane for each point on the heightmap
    {
        float3 position = float3((float) (GTid.x) / 15.0f, groupResults[GI], (float) (GTid.y) / 15.0f);
        float distance = dot(plane.xyz, position) - plane.w;
        groupResults[GI] = distance;
    }
    
    GroupMemoryBarrierWithGroupSync();

    // phase 5: compute the standard deviation of the distance from the plane
    
    if (GI == 0)
    {
        float stddev = 0.0f;
        for (int i = 0; i < 16 * 16; i++)
            stddev += pow(groupResults[i], 2);
        stddev /= (16.0f * 16.0f) - 1.0f;
        stddev = sqrt(stddev);
        
        // output to the preprocess texture
        preprocessMap[Gid.xy] = float4(plane.xyz, stddev);
    }
}