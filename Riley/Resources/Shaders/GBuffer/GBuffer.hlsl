#include "../Common.hlsli"

struct VSInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentModel : TANGENT0;
    float3 bitangentModel : BITANGENT0;
};

struct VSToPS
{
    float4 posProj : SV_POSITION; // Screen position
    float2 texcoord : TEXCOORD0;
    float3 normalWS : NORMAL0;
    float3 normalVS : NORMAL1;
    float3 tangentWS : TANGENT0;
    float3 bitangentWS : BITANGENT0;
};

VSToPS GBufferVS(VSInput input)
{
    VSToPS output;
    
    float4 pos = float4(input.posModel, 1.0);
    pos = mul(pos, meshData.world);
    output.posProj = mul(pos, frameData.viewProj);
    
    output.normalWS = mul(input.normalModel, (float3x3) meshData.worldInvTranspose);
    output.normalVS = mul(output.normalWS, (float3x3) frameData.view);
    output.tangentWS = mul(input.tangentModel, (float3x3) meshData.world);
    output.bitangentWS = mul(input.bitangentModel, (float3x3) meshData.world);
    output.texcoord = input.texcoord;

    return output;
}

Texture2D AlbedoTex : register(t0);
Texture2D NormalTex : register(t1);
Texture2D MetallicRoughnessTex : register(t2);
Texture2D EmissiveTex : register(t3);

struct PSOutput
{
    float4 DiffuseRoughness : SV_Target0;
    float4 NormalMetallic : SV_Target1;
    float4 Emissive : SV_Target2;
};

float3 GetNormal(VSToPS input)
{
    float3 normalWorld = normalize(input.normalWS);
    
    if (materialData.useNormalMap)
    {
        float4 viewPos = mul(input.posProj, frameData.invProj);
        
        float dist = length(float3(0, 0, 0) - viewPos.xyz);
        float distMin = 3.0;
        float distMax = 10.0;
        float lod = 2.0 * saturate(dist / (distMax - distMin));
        
        float3 normal = NormalTex.SampleLevel(LinearWrapSampler, input.texcoord, lod).rgb; // ¹üÀ§ [0, 1]
        normal = 2.0 * normal - 1.0;
        
        float3 N = normalWorld;
        //float3 T = normalize(input.tangentWS - dot(input.tangentWS, N) * N);
        float3 T = normalize(input.tangentWS);
        float3 B = normalize(cross(N, T));
        
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = mul(normal, TBN);
    }

    return normalWorld;
}

PSOutput PackGBuffer(float3 baseColor, float3 normalVS, float4 emissive, float roughness, float metallic)
{
    PSOutput output;
    output.NormalMetallic = float4(0.5 * normalVS + 0.5, metallic);
    output.DiffuseRoughness = float4(baseColor, roughness);
    output.Emissive = float4(emissive.xyz, emissive.w / 256);
    return output;
}

PSOutput GBUfferPS(VSToPS input)
{
    float3 albedoColor = AlbedoTex.Sample(LinearWrapSampler, input.texcoord);
    float3 normalWS = GetNormal(input);
    float3 normalVS = normalize(mul(normalWS, (float3x3) frameData.view));
    
    float ao = materialData.albedoFactor;
    float metallic = MetallicRoughnessTex.Sample(LinearWrapSampler, input.texcoord).b * materialData.metallicFactor;
    if(metallic < 0.1)
        metallic = materialData.metallicFactor;
    float roughness = MetallicRoughnessTex.Sample(LinearWrapSampler, input.texcoord).g * materialData.roughnessFactor;
    float3 aoRoughnessMetallic = float3(ao, roughness, metallic);
    
    float3 emissive = EmissiveTex.Sample(LinearWrapSampler, input.texcoord);
    
    return PackGBuffer(albedoColor.xyz, normalVS, float4(emissive, materialData.emissiveFactor), aoRoughnessMetallic.g, aoRoughnessMetallic.b);

}