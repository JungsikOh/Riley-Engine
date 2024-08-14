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

float3 GetFrustumClipSpace(float3 from, float3 to, float near, float far, float2 s)
{
    float3 dir = to - from;
    float3 signDirection = sign(dir);
    
    float nearFarSlab = signDirection.z * (far - near) * 0.5 + (far - near) * 0.5;
    float lenZ = (nearFarSlab - from.z) / dir.z;
    if(dir.z == 0.0)
        lenZ == INFINITY;
    
    float2 ss = sign(dir.xy - s * dir.z) * s;
    float2 denominator = ss * dir.z - dir.xy;
    float2 lenXY = (from.xy - ss * from.z) / denominator;
    if(lenXY.x < 0.0 || denominator.x == 0.0)
        lenXY.x = INFINITY;
    if (lenXY.y < 0.0 || denominator.y == 0.0)
        lenXY.y = INFINITY;
    
    float len = min(min(1.0, lenZ), min(lenXY.x, lenXY.y));
    float3 clippedVS = from + dir * len;
    
    return clippedVS;
}

float GetThicknessDiff(float diff, float linearSampleDepth, float thicknessParam)
{
    return (diff - thicknessParam) / linearSampleDepth;
}

float4 SSRBinarySearch(float3 dir, inout float3 hitCoord)
{
    float depth;
    for (int i = 0; i < SSR_BINARY_SEARCH_STEPS; ++i)
    {
        float4 projectedCoord = mul(float4(hitCoord, 1.0f), frameData.proj);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

        depth = DepthTex.SampleLevel(PointClampSampler, projectedCoord.xy, 0).r;
        float3 viewSpacePosition = GetViewSpacePosition(projectedCoord.xy, depth);
        float depthDifference = hitCoord.z - viewSpacePosition.z;

        if (depthDifference <= 0.0f)
            hitCoord += dir;

        dir *= 0.5f;
        hitCoord -= dir;
    }

    float4 projectedCoord = mul(float4(hitCoord, 1.0f), frameData.proj);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    
    depth = DepthTex.SampleLevel(PointClampSampler, projectedCoord.xy, 0);
    float3 viewSpacePosition = GetViewSpacePosition(projectedCoord.xy, depth);
    float depthDifference = hitCoord.z - viewSpacePosition.z;
    
    return float4(projectedCoord.xy, depth, abs(depthDifference) < postData.ssrThickness ? 1.0f : 0.0f);
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
        
        depth = DepthTex.SampleLevel(PointClampSampler, projectedCoord.xy, 0);
        float3 viewSpacePosition = GetViewSpacePosition(projectedCoord.xy, depth);
        float depthDiff = hitCoord.z - viewSpacePosition.z;
        
        [branch]
        if(depthDiff > 0.0)
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
    float4 sceneColor = SceneTex.SampleLevel(LinearClampSampler, input.texcoord, 0);
    
    float metallic = normalMetallic.a;
    if (metallic < 0.01)
        return sceneColor;
    
    float3 normal = normalMetallic.rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(normal);
    
    float depth = DepthTex.Sample(LinearClampSampler, input.texcoord).r;
    float3 viewPosition = GetViewSpacePosition(input.texcoord, depth);
    float3 reflectDirection = normalize(reflect(viewPosition, normal));
    
    float3 hitPosition = viewPosition;
    float4 coords = SSRRayMarch(reflectDirection, hitPosition);
    
    float2 coordsEdgeFactor = float2(1, 1) - pow(saturate(abs(coords.xy - float2(0.5f, 0.5f)) * 2), 8);
    float screenEdgeFactor = saturate(min(coordsEdgeFactor.x, coordsEdgeFactor.y));
    float reflectionIntensity = saturate(screenEdgeFactor * saturate(reflectDirection.z) * (coords.w));
    
    float3 reflectionColor = reflectionIntensity * SceneTex.SampleLevel(LinearClampSampler, coords.xy, 0).rgb;
    
    float3 endPosition = hitPosition + reflectDirection * frameData.cameraFar;
    
    //float3 clippedVS = GetFrustumClipSpace(hitPosition, endPosition, frameData.cameraNear, frameData.cameraFar, float2(frameData.cameraFrustumX, frameData.cameraFrustumY));
    //float4 clipHitPosition = mul(float4(hitPosition, 1.0), frameData.proj);
    //float4 clipEndpostion = mul(float4(endPosition, 1.0), frameData.proj);
    
    //float k0 = 1.0 / clipHitPosition.w;
    //float k1 = 1.0 / clipEndpostion.w;
    //float3 q0 = clipHitPosition.xyz;
    //float3 q1 = clipEndpostion.xyz;
    //float2 p0 = clipHitPosition.xy * float2(1, -1) * k0 * 0.5 + 0.5; // Clip -> NDC
    //float2 p1 = clipEndpostion.xy * float2(1, -1) * k1 * 0.5 + 0.5; // Clip -> NDC
    
    //float w1 = 0.0;
    //float w2 = 0.0;
    //bool hit = false;
    //bool lastHit = false;
    //bool potentialHit = false;
    
    //float2 potentialW12 = float2(0.0, 0.0);
    //float minPotentialHitPosition = INFINITY;
    //for (int i = 0; i < SSR_MAX_STEPS; ++i)
    //{
    //    w2 = w1;
    //    w1 += 1.0 / float(SSR_MAX_STEPS);
        
    //    float2 p = lerp(p0, p1, w1);        // xy in ndc
    //    float3 q = lerp(q0, q1, w1);        // xyz in clip space
    //    float k = lerp(k0, k1, w1);         // w in clip space
    //    float sampleDepth = DepthTex.Sample(PointClampSampler, p).r;
    //    float linearSampleDepth = ConvertZToLinearDepth(sampleDepth);
    //    float linearRayDepth = ConvertZToLinearDepth(q.z * k);
        
    //    float diff = linearRayDepth - linearSampleDepth;
    //    float thicknessDiff = GetThicknessDiff(diff, linearSampleDepth, SSR_THICKNESS);
    //    if(diff > 0.0)
    //    {
    //        if (thicknessDiff < SSR_THICKNESS)
    //        {
    //            hit = true;
    //            break;
    //        }
    //        else if(!lastHit)
    //        {
    //            potentialHit = true;
    //            if(minPotentialHitPosition > thicknessDiff)
    //            {
    //                minPotentialHitPosition = thicknessDiff;
    //                potentialW12 = float2(w1, w2);
    //            }
    //        }
    //    }
    //    lastHit = diff > 0.0;
    //}
    
    //if (hit || potentialHit)
    //{
    //    if (!hit)
    //    {
    //        w1 = potentialW12.x;
    //        w2 = potentialW12.y;
    //    }
            
    //    bool realhit = false;
    //    float2 hitPos;
    //    float minThicknessDiff = SSR_THICKNESS;
    //    for (int i = 0; i < 5; ++i)
    //    {
    //        float w = 0.5 * (w1 + w2);
    //        float p = lerp(p0, p1, w);
    //        float3 q = lerp(q0, q1, w);
    //        float k = lerp(k0, k1, w);
    //        float sampleDepth = DepthTex.Sample(PointClampSampler, p).r;
    //        float linearSampleDepth = ConvertZToLinearDepth(sampleDepth);
    //        float linearRayDepth = ConvertZToLinearDepth(q.z * k);
        
    //        float diff = linearRayDepth - linearSampleDepth;
    //        if (diff > 0.0)
    //        {
    //            w1 = w;
    //            if(hit)
    //                hitPos = p;
    //            else
    //            {
    //                w2 = w;
    //            }
                
    //            float thicknessDiff = GetThicknessDiff(diff, linearSampleDepth, SSR_THICKNESS);
    //            float absThicknessDiff = abs(thicknessDiff);
                
    //            if(!hit && absThicknessDiff < minThicknessDiff)
    //            {
    //                realhit = true;
    //                minThicknessDiff = thicknessDiff;
    //                hitPos = p;

    //            }
    //        }
            
    //        if(hit || realhit)
    //            color = SceneTex.Sample(LinearClampSampler, hitPos).rgb;
    //    }
    //}
    
    //if(hit)
    //{
    //    float2 hitPos = lerp(p0, p1, w1);
    //    color = SceneTex.Sample(LinearClampSampler, hitPos).rgb;
    //}
    
    return sceneColor + float4(reflectionColor, 1.0);
}