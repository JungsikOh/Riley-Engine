#include "../Common.hlsli"

struct VSInput
{
    float3 posModel : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VSToPS
{
    float4 posProj : SV_Position;
    float2 texcoord : TEXCOORD0;
};

VSToPS SunVS(VSInput input)
{
    VSToPS output;
    
    float4x4 modelMatrix = meshData.world;
    modelMatrix[3][0] += frameData.invView[3][0];
    modelMatrix[3][1] += frameData.invView[3][1];
    modelMatrix[3][2] += frameData.invView[3][2];
    
    float4x4 modelView = mul(modelMatrix, frameData.view);
    // Delete effecting for the Rotation and Scale value
    modelView[0][0] = 1;
    modelView[0][1] = 0;
    modelView[0][2] = 0;
    
    modelView[1][0] = 0;
    modelView[1][1] = 1;
    modelView[1][2] = 0;
    
    modelView[2][0] = 0;
    modelView[2][1] = 0;
    modelView[2][2] = 1;
    
    float4 viewPosition = mul(float4(input.posModel, 1.0), modelView);
    float4 clipPosition = mul(viewPosition, frameData.proj);
    output.posProj = float4(clipPosition.xy, clipPosition.w - 0.001f, clipPosition.w);
    output.texcoord = input.texcoord;
    
    return output;
}

Texture2D SunTex : register(t0);

float4 SunPS(VSToPS input) : SV_Target
{
    float4 texColor = SunTex.Sample(LinearWrapSampler, input.texcoord);
    return texColor;
}