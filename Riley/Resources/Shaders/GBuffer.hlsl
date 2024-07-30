#include "Common.hlsli"

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
    float3 tangentWS : TANGENT0;
    float3 bitangentWS : BITANGENT0;
};

VSToPS GBufferVS(VSInput input)
{
    VSToPS output;
    
    float4 pos = float4(input.posModel, 1.0);
    pos = mul(pos, meshData.world);
    output.posProj = mul(pos, frameData.viewProj);
    
    float3 normalWS = normalize(mul(input.normalModel, (float3x3) transpose(meshData.worldInvTranspose)));
    output.tangentWS = mul(input.tangentModel, (float3x3) meshData.world);
    output.bitangentWS = mul(input.bitangentModel, (float3x3) meshData.world);
    output.normalWS = normalWS;
    output.texcoord = input.texcoord;

    return output;
}

struct PSOutput
{
    float4 DiffuseRoughness      : SV_Target0;
    float4 NormalMetallic        : SV_Target1;
    float4 Emissive              : SV_Target2;
};

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
    float3 albedoColor = materialData.ambient;
    float3 normalVS = normalize(mul(input.normalWS, (float3x3) frameData.view));
    
    float ao = materialData.albedoFactor;
    float metallic = materialData.metallicFactor;
    float roughness = materialData.roughnessFactor;
    float3 aoRoughnessMetallic = float3(ao, roughness, metallic);
    
    float3 emissive = materialData.emissiveFactor;
    
    return PackGBuffer(albedoColor.xyz, normalize(normalVS), float4(emissive, materialData.emissiveFactor), aoRoughnessMetallic.g, aoRoughnessMetallic.b);

}