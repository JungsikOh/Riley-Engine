#include "Common.hlsli"

Texture2D DiffuseRoughnessTex : register(t0);
Texture2D NormalMetallicTex : register(t1);
Texture2D EmissiveTex : register(t2);
Texture2D<float> DepthTex : register(t3);
Texture2D NoiseTex : register(t4);

struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 SSAO(VSToPS input) : SV_TARGET
{
    float depth = DepthTex.Sample(LinearBorderSampler, input.texcoord);
    if (depth < 1.0)
    {
        float3 positionVS = GetViewSpacePosition(input.texcoord, depth);
        float3 normalVS = NormalMetallicTex.Sample(LinearBorderSampler, input.texcoord).rgb;
        normalVS = normalize(normalVS * 2.0 - 1.0);
        
        float3 randomVec = normalize(NoiseTex.Sample(PointWrapSampler, input.texcoord * postData.noiseScale).xyz * 2.0 - 1.0);
        
        float3 tangent = normalize(randomVec - normalVS * dot(randomVec, normalVS));
        float3 bitangent = normalize(cross(normalVS, tangent));
        float3x3 TBN = float3x3(tangent, bitangent, normalVS);
        
        float occlusion = 0.0;
        [unroll(SSAO_KERNEL_SIZE)]
        for (int idx = 0; idx < SSAO_KERNEL_SIZE; ++idx)
        {
            float3 sampleDir = mul(postData.samples[idx].xyz, TBN);
            float3 samplePos = positionVS + (sampleDir * postData.ssaoRadius);
            
            float4 offset = float4(sampleDir, 1.0);
            offset = mul(offset, frameData.proj);
            offset.xyz /= offset.w;
            offset.y *= -1.0;
            offset.xy = offset.xy * 0.5 + 0.5;
            
            float sampleDepth = DepthTex.Sample(LinearBorderSampler, offset.xy);
            sampleDepth = GetViewSpacePosition(offset.xy, sampleDepth).z;
            
            float occluded = step(sampleDepth, samplePos.z - 0.005);
            float intensity = smoothstep(0.0, 1.0, postData.ssaoRadius / abs(positionVS.z - sampleDepth));
            
            occlusion += occluded * intensity;
        }
        occlusion = 1.0 - (occlusion / (float) SSAO_KERNEL_SIZE);
        occlusion = pow(abs(occlusion), postData.ssaoPower);
        return float4(occlusion, occlusion, occlusion, 1.0);
    }
    return float4(1, 1, 1, 1);
}