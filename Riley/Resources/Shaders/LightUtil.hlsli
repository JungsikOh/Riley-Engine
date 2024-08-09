#ifndef __LIGHT_UTILS__
#define __LIGHT_UTILS__

#include "Common.hlsli"

float DoAttenuation(float distance, float range)
{
    // 1. Gihbud Source
    float att = saturate(1.0f - (distance * distance / (range * range)));
    
    // 2. LearnOpenGL
    //float att = 1.0 / (distance * distance);
    
    return att * att;
}

/////////////////
// Blinn-Phong //
/////////////////

struct LightingResult
{
    float3 diffuse;
    float3 specular;
};

float3 DoDiffuse(LightData light, float3 L, float3 N)
{
    float NdotL = max(0.0f, dot(N, L));
    return (light.lightColor.rgb * NdotL);
}

float3 DoSpecular(LightData light, float shininess, float3 L, float3 N, float3 V)
{
    float3 H = normalize(L + V);
    float NdotH = max(0.0f, dot(N, H));
    return (light.lightColor.rgb * pow(NdotH, shininess));
}

LightingResult DoPointLight(LightData light, float shininess, float3 V, float3 P, float3 N)
{    
    float3 L = light.position.xyz - P;
    float distance = length(L);
    L = L / distance;
    
    N = normalize(N);
    float attenuation = DoAttenuation(distance, light.range);
    
    LightingResult result;
    result.diffuse = DoDiffuse(light, L, N) * attenuation;
    result.specular = DoSpecular(light, shininess, V, L, N) * attenuation;
    return result;
}

LightingResult DoDirectionalLight(LightData light, float shininess, float3 V, float3 N)
{
    LightingResult result;
    
    N = normalize(N);
    float3 L = -light.direction.xyz;
    L = normalize(L);
    
    result.diffuse = DoDiffuse(light, L, N);
    result.specular = DoSpecular(light, shininess, V, L, N);
    return result;
}

LightingResult DoSpotLight(LightData light, float shininess, float3 V, float3 P, float3 N)
{
    LightingResult result;
    
    float3 L = light.position.xyz - P;
    float distance = length(L);
    L = L / distance;
    
    N = normalize(N);
    float attenuation = DoAttenuation(distance, light.range);
    float3 lightDir = normalize(light.direction.xyz);
    
    // https://learnopengl.com/Lighting/Light-casters
    float cosAngle = dot(-lightDir, L);
    float conAtt = saturate((cosAngle - light.outerCosine) / (light.innerCosine - light.outerCosine));
    conAtt *= conAtt;
    
    result.diffuse = DoDiffuse(light, L, N) * attenuation * conAtt;
    result.specular = DoSpecular(light, shininess, V, L, N) * attenuation * conAtt;
    return result;
}

/////////////////
// PBR //
/////////////////

static const float PI = 3.141592653;
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(0.0f, dot(N, H));
    float NdotH2 = NdotH * NdotH;
    
    float numerator = a2;
    float denominator = PI * pow((NdotH2 * (a2 - 1.0f) + 1.0f), 2);
    
    return numerator / denominator;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    
    float numerator = NdotV;
    float denominator = NdotV * (1.0f - k) + k;
    
    return numerator / denominator;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(max(1.0f - cosTheta, 0.0f), 5.0f);
}

float3 FreselSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(max(1.0f - cosTheta, 0.0f), 5.0f);
}

float3 DoPointLightPBR(LightData light, float3 positionVS, float3 normalVS, float3 V, float3 albedo, float metallic, float roughness)
{
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    float3 L = normalize(light.position.xyz - positionVS);
    float3 H = normalize(L + V);
    float distance = length(light.position.xyz - positionVS);
    float attenuation = DoAttenuation(distance, light.range);
    float3 radiance = light.lightColor * attenuation * light.range;
    
    float NDF = DistributionGGX(normalVS, H, roughness);
    float G = GeometrySmith(normalVS, V, L, roughness);
    float3 F = FresnelSchlick(clamp(dot(H, V), 0.0f, 1.0f), F0);
    
    float3 numerator = NDF * F * G;
    float denominator = 4.0f * max(0.0f, dot(normalVS, V)) * max(0.0f, dot(normalVS, L)) + 1e-5;

    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;
    float NdotL = max(0.0f, dot(normalVS, L));
    float3 Lo = (kD * albedo / PI + numerator / denominator) * radiance * NdotL;
    
    return Lo;
}

float3 DoDirectinoalLightPBR(LightData light, float3 positionVS, float3 normalVS, float3 V, float3 albedo, float metallic, float roughness)
{
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    float3 L = normalize(-light.direction.xyz);
    float3 H = normalize(L + V);
    
    float3 radiance = light.lightColor.xyz;
    
    float NDF = DistributionGGX(normalVS, H, roughness);
    float G = GeometrySmith(normalVS, V, L, roughness);
    float3 F = FresnelSchlick(clamp(dot(H, V), 0.0f, 1.0f), F0);
    
    float3 numerator = NDF * F * G;
    float denominator = 4.0f * max(0.0f, dot(normalVS, V)) * max(0.0f, dot(normalVS, L)) + 1e-5;

    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;
    float NdotL = max(0.0f, dot(normalVS, L));
    float3 Lo = (kD * albedo / PI + (numerator / denominator)) * radiance * NdotL;
    
    return Lo;
}

float3 DoSpotLightPBR(LightData light, float3 positionVS, float3 normalVS, float3 V, float3 albedo, float metallic, float roughness)
{
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    float3 L = normalize(light.position.xyz - positionVS);
    float3 H = normalize(L + V);
    float distance = length(light.position.xyz - positionVS);
    float attenuation = DoAttenuation(distance, light.range);
    
    // https://learnopengl.com/Lighting/Light-casters
    float3 lightDir = normalize(light.direction.xyz);
    float cosAngle = dot(-lightDir, L);
    float conAtt = saturate((cosAngle - light.outerCosine) / (light.innerCosine - light.outerCosine));
    conAtt *= conAtt;
    
    float3 radiance = light.lightColor * attenuation * conAtt;
    
    float NDF = DistributionGGX(normalVS, H, roughness);
    float G = GeometrySmith(normalVS, V, L, roughness);
    float3 F = FresnelSchlick(clamp(dot(H, V), 0.0f, 1.0f), F0);
    
    float3 numerator = NDF * F * G;
    float denominator = 4.0f * max(0.0f, dot(normalVS, V)) * max(0.0f, dot(normalVS, L)) + 1e-5;

    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;
    float NdotL = max(0.0f, dot(normalVS, L));
    float3 Lo = (kD * albedo / PI + (numerator / denominator)) * radiance * NdotL;
    
    return Lo;
}

#endif