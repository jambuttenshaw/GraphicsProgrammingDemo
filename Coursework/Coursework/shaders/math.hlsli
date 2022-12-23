

#define PI 3.14159268535


// Smooth minimum of two values, controlled by smoothing factor k
// When k = 0, this behaves identically to min(a, b)
float smoothMin(float a, float b, float k)
{
    k = max(0, k);
    // https://www.iquilezles.org/www/articles/smin/smin.htm
    float h = max(0, min(1, (b - a + k) / (2 * k)));
    return a * h + b * (1 - h) - k * h * (1 - h);
}

// Smooth maximum of two values, controlled by smoothing factor k
// When k = 0, this behaves identically to max(a, b)
float smoothMax(float a, float b, float k)
{
    k = min(0, -k);
    float h = max(0, min(1, (b - a + k) / (2 * k)));
    return a * h + b * (1 - h) - k * h * (1 - h);
}


// taken from: https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d

// adapted from intersectCube in https://github.com/evanw/webgl-path-tracing/blob/master/webgl-path-tracing.js

// compute the near and far intersections of the cube (stored in the x and y components) using the slab method
// no intersection means vec.x > vec.y (really tNear > tFar)
float2 intersectAABB(float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax)
{
    float3 tMin = (boxMin - rayOrigin) / rayDir;
    float3 tMax = (boxMax - rayOrigin) / rayDir;
    float3 t1 = min(tMin, tMax);
    float3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return float2(tNear, tFar);
};


// taken from: https://learnopengl.com/PBR/IBL/Specular-IBL
// function for the Hammersley low-discrepancy sequence, based on the Van Der Corput sequence

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}


// other utility functions

float remap01(float v)
{
    return saturate(0.5f * (1.0f + v));
}
float remap01(float v, float v_min, float v_max)
{
    return saturate((v - v_min) / (v_max - v_min));
}
float2 remap01(float2 v)
{
    return saturate(0.5f * (1.0f + v));
}
float3 remap01(float3 v)
{
    return saturate(0.5f * (1.0f + v));
}
float4 remap01(float4 v)
{
    return saturate(0.5f * (1.0f + v));
}


float remap01_noclamp(float v)
{
    return 0.5f * (1.0f + v);
}
float remap01_noclamp(float v, float v_min, float v_max)
{
    return (v - v_min) / (v_max - v_min);
}
float2 remap01_noclamp(float2 v)
{
    return 0.5f * (1.0f + v);
}
float3 remap01_noclamp(float3 v)
{
    return 0.5f * (1.0f + v);
}
float4 remap01_noclamp(float4 v)
{
    return 0.5f * (1.0f + v);
}


// change of basis
float3x3 cotangent_frame(float3 n, float3 p, float2 uv)
{
    // get edge vectors of the pixel triangle 
    float3 dp1 = ddx(p);
    float3 dp2 = ddy(p);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);
    // solve the linear system 
    float3 dp2perp = cross(dp2, n);
    float3 dp1perp = cross(n, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
    // construct a scale-invariant frame (larger geometry will have smaller partial derivatives per pixel)
    float invmax = rsqrt(max(dot(T, T), dot(B, B)));
    // create TBN matrix
    return float3x3(T * invmax, B * invmax, n);
}

float3 tangentToWorld(float3 sample, float3 n, float3 p, float2 uv)
{
    sample = (sample * 2.0f) - 1.0f;
        
    float3x3 TBN = cotangent_frame(n, p, uv);
    return normalize(mul(sample, TBN));
}
