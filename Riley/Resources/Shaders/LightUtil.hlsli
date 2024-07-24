#ifndef __LIGHT_UTILS__
#define __LIGHT_UTILS__

#include "Common.hlsli"

float DoAttenuation(float distance, float range)
{
    // 1. Gihbud Source
    //float att = saturate(1.0f - (distance * distance / (range * range)));
    
    // 2. LearnOpenGL
    float att = 1.0 / (distance * distance);
    
    return att;
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
    float NdotH = max(0.0001f, dot(N, H));
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

#endif