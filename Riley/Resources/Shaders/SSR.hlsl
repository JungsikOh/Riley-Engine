#include "Common.hlsli"

Texture2D DiffuseRoughnessTex : register(t0);
Texture2D NormalMetallicTex : register(t1);
Texture2D EmissiveTex : register(t2);
Texture2D<float> DepthTex : register(t3);
Texture2D SceneTex : register(t4);

static const int SSR_MAX_STEPS = 16;
static const int SSR_BINARY_SEARCH_STEPS = 16;
static const float SSR_THICKNESS = postData.ssrThickness;
static const int INFINITY = 1e10;

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

bool NotInsideNDC(float4 coords)
{
    return (coords.x < 0 || coords.x > 1 || coords.y < 0 || coords.y > 1);
}

// 이진 탐색을 하는 이유는 충돌위치에서 정확한 색깔값을 얻기 위한 작업이다.
// 충돌하더라도 두께로 인해서 잘못된 색깔값을 갖고 오는 경우가 있을텐데 그것을 방지하기 위해서
// 차이를 최대한 0으로 만들려고 탐색하는 과정이 바로 이진 탐색이다.
float4 SSRBinarySearch(float3 dir, inout float3 hitCoord)
{
    float depth;
    for (int i = 0; i < SSR_BINARY_SEARCH_STEPS; ++i)
    {
        float4 projectedCoord = mul(float4(hitCoord, 1.0), frameData.proj);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
        depth = DepthTex.SampleLevel(PointClampSampler, projectedCoord.xy, 0).r;
        float3 samplePosition = GetViewSpacePosition(projectedCoord.xy, depth);
        float diff = hitCoord.z - samplePosition.z;
        
        if (diff <= 0.0)
        {
            hitCoord += dir;
        }
        dir *= 0.5;
        hitCoord -= dir;
    }
    
    float4 projectedCoord = mul(float4(hitCoord, 1.0), frameData.proj);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
    depth = DepthTex.SampleLevel(PointClampSampler, projectedCoord.xy, 0).r;
    float3 samplePosition = GetViewSpacePosition(projectedCoord.xy, depth);
    float diff = hitCoord.z - samplePosition.z;
    
    return float4(projectedCoord.xy, depth, abs(diff) < SSR_THICKNESS ? 1.0 : 0.0);
}

float4 SSRRayMarch(float3 dir, inout float3 hitCoord)
{
    float depth;
    for (int i = 0; i < SSR_MAX_STEPS; ++i)
    {
        hitCoord += dir;
        float4 projectedCoord = mul(float4(hitCoord, 1.0), frameData.proj);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
        
        if (NotInsideNDC(projectedCoord))
        {
            break;
        }
        
        depth = DepthTex.SampleLevel(PointClampSampler, projectedCoord.xy, 0).r;
        float3 samplePosition = GetViewSpacePosition(projectedCoord.xy, depth);
        float diff = hitCoord.z - samplePosition.z;
        
        if (diff > 0.0)
        {
            return SSRBinarySearch(dir, hitCoord);
        }
        
        dir *= postData.ssrRayStep;
    }
    return float4(0, 0, 0, 0);
}

float4 SSR(VSToPS input) : SV_Target
{
    float4 normalMetallic = NormalMetallicTex.Sample(LinearBorderSampler, input.texcoord);
    float4 sceneColor = SceneTex.Sample(LinearClampSampler, input.texcoord);
    
    float metallic = normalMetallic.a;
    if (metallic < 0.01)
        return sceneColor;
    
    float3 normal = normalMetallic.rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(normal);
    
    float depth = DepthTex.Sample(LinearClampSampler, input.texcoord).r;
    float3 viewPosition = GetViewSpacePosition(input.texcoord, depth);
    float3 incidentVector = normalize(viewPosition);
    float3 reflectDirection = normalize(reflect(incidentVector, normal));
    
    float3 reflectionColor = float3(0, 0, 0);
    
    // 기존 방법 */
    float3 hitPosition;
    float4 coords = SSRRayMarch(reflectDirection, hitPosition);
    float2 coordsEdgeFactor = float2(1.0, 1.0) - pow(saturate(abs(coords.xy - float2(0.5, 0.5))) * 2.0, 4.0);
    float screenEdgeFactor = saturate(min(coordsEdgeFactor.x, coordsEdgeFactor.y));
    
    reflectionColor = SceneTex.SampleLevel(LinearClampSampler, coords.xy, 0).rgb * coords.w * screenEdgeFactor;
    
    return sceneColor + float4(reflectionColor, 1.0);
}