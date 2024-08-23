#include "../Common.hlsli"

Texture2D<float> DepthTex : register(t0);

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

int RaySphereIntersection(in float3 start, in float3 dir, in float3 center, in float radius, out float t1, out float t2)
{
    float3 P = start - center;
    float pdotd = dot(P, dir);
    float p2 = dot(P, P);
    float r2 = radius * radius;
    float m = pdotd * pdotd - (p2 - r2);
    
    if (m < 0.0)
    {
        t1 = 0;
        t2 = 0;
        return 0;
    }
    else
    {
        t1 = -pdotd - sqrt(m);
        t2 = -pdotd + sqrt(m);
        return 1;
    }
}

float HaloEmission(float3 positionVS, float radius)
{
    float3 rayStart = 0.0.xxx;
    float3 rayDir = normalize(positionVS - 0.0.xxx);
    float3 center = lightData.position.xyz;
    float t1 = 0.0;
    float t2 = 0.0;
    
    // Integral Light Sphere
    float3 P = rayStart - center;
    float r2 = radius * radius;
    float p2 = dot(P, P);
    float pdotd = dot(P, rayDir);
    
    if (RaySphereIntersection(rayStart, rayDir, center, radius, t1, t2) && t2 > 0.0 && t1 < positionVS.z)
    {
        if(t2 > positionVS.z)
        {
            t2 = positionVS.z;
        }
        
        float haloEmission = (1 - p2 / r2) * (t2 - t1) - (pdotd / r2) * (t2 * t2 - t1 * t1) - (1.0 / (3.0 * r2)) * (t2 * t2 * t2 - t1 * t1 * t1);
        haloEmission /= (4.0 * radius / 3.0);
        
        return haloEmission;
    }
    return 0.0;
}

float4 Halo(VSToPS input) : SV_TARGET
{
    float depth = DepthTex.Sample(LinearClampSampler, input.texcoord);
    float3 positionVS = GetViewSpacePosition(input.texcoord, depth);
    float3 haloEmission = HaloEmission(positionVS, lightData.range / 100.0) * lightData.lightColor.xyz * lightData.haloStrength;
    
    if(lightData.type == SPOT_LIGHT)
    {        
        float3 L = normalize(lightData.position.xyz - positionVS);
        float3 lightDir = normalize(lightData.direction.xyz);
        float cosAngle = saturate(dot(-lightDir, L));
        
        haloEmission *= cosAngle;
    }
    
    return float4(haloEmission, 1.0);
}