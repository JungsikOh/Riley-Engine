#include "Common.hlsli"
#include "LightUtil.hlsli"

struct VSInput
{
    float3 posModel : POSITION; //모델 좌표계의 위치 position
    float3 normalModel : NORMAL0; // 모델 좌표계의 normal    
    float2 texcoord : TEXCOORD0;
    float3 tangentModel : TANGENT0;
    float3 bitangentModel : BITANGENT0;
};

struct PSInput
{
    float4 posProj : SV_POSITION; // Screen position
    float4 posWorld : position1;
    float3 normalWorld : NORMAL0;
    float3 normalView : NORMAL1;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
    float3 bitangentWorld : BITANGENT0;
};

PSInput PhongVS(VSInput input)
{
    PSInput output;
    
    float4 pos = float4(input.posModel, 1.0);
    pos = mul(pos, meshData.world);
    output.posWorld = pos;
    output.posProj = mul(pos, frameData.viewProj);
    
    float3 normalWorld = normalize(mul(input.normalModel, (float3x3) meshData.worldInvTranspose));
    output.normalView = mul(normalWorld, (float3x3) transpose(frameData.invView));
    output.tangentWorld = mul(input.tangentModel, (float3x3) meshData.world);
    output.bitangentWorld = mul(input.bitangentModel, (float3x3) meshData.world);
    output.normalWorld = normalWorld;
    output.texcoord = input.texcoord;

    return output;
}

float ShadowCalculation(LightData light, float4 posWorld, Texture2D shadowMap)
{
    float shadow = 0.0;
    if (light.castShadows)
    {
        if (light.type & DIRECTIONAL_LIGHT)
        {
            // light의 inputTexcoord를 찾아야한다.
            // 빛에서 볼 때를 기준으로 값을 정한다.
            // 1. Light Proj -> Screen(Clip)
            float4 lightScreen = mul(posWorld, shadowData.lightViewProj);
            lightScreen /= lightScreen.w; // Clip -> NDC
            lightScreen.y *= -1; // texcoord는 y축 방향이 다르므로
        
            // 2. [-1, 1] -> [0, 1]
            lightScreen *= 0.5;
            lightScreen += 0.5;
        
            float2 lightTex = float2(lightScreen.x, lightScreen.y);
    
            float shadow = 0.0;
            shadow += shadowMap.SampleCmpLevelZero(ShadowSampler, lightTex.xy, lightScreen.z).r;
        }
        return shadow;
    }
}

struct PSOutput
{
    float4 pixelColor : SV_Target0;
};

PSOutput PhongPS(PSInput input)
{
    PSOutput output;
    
    float4 viewPosition = mul(input.posWorld, frameData.view);
    float4 worldPosition = input.posWorld;
    float3 V = normalize(0.0.xxx - viewPosition.xyz);
    //float3 V = normalize(frameData.cameraPosition.xyz - input.posWorld.xyz);
    
    float3 ambient = materialData.ambient;
    
    LightingResult Lo;
    //float shadowFactor = ShadowCalculation(lightData, input.posWorld, shadowMap);
    
    switch (lightData.type)
    {
        case DIRECTIONAL_LIGHT:
            Lo = DoDirectionalLight(lightData, 2.0, V, input.normalWorld);
            break;
        case POINT_LIGHT:
            Lo = DoPointLight(lightData, 2.0, V, viewPosition.xyz, input.normalView);
            break;
        case SPOT_LIGHT:
            Lo = DoSpotLight(lightData, 2.0, V, viewPosition.xyz, input.normalView);
            break;
    }
    
    output.pixelColor = float4(ambient + (Lo.diffuse + Lo.specular), 1.0);
    return output;
}