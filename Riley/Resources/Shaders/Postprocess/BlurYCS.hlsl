#include "../Common.hlsli"

Texture2D InputTex : register(t0);
RWTexture2D<float4> OutputTex : register(u0);

static const uint N = 1024;

groupshared float4 SharedHorizontalData[4 + N + 4]; // 앞뒤를 참조하기 위해 추가 공간을 할당

[numthreads(1, N, 1)]
void BlurY(uint3 gID : SV_GroupID,
                uint3 gtID : SV_GroupThreadID,
                uint3 dtID : SV_DispatchThreadID,
                uint gIdx : SV_GroupIndex)
{
    float4 input = InputTex.Load(dtID);
    SharedHorizontalData[gtID.y + 4] = input;
    
    if (gIdx == 0)
    {
        SharedHorizontalData[0] = InputTex.Load(dtID - int3(0, 4, 0));
        SharedHorizontalData[1] = InputTex.Load(dtID - int3(0, 3, 0));
        SharedHorizontalData[2] = InputTex.Load(dtID - int3(0, 2, 0));
        SharedHorizontalData[3] = InputTex.Load(dtID - int3(0, 1, 0));
    }
    
    if (gIdx == N - 1)
    {
        SharedHorizontalData[4 + N + 0] = InputTex.Load(dtID + int3(0, 1, 0));
        SharedHorizontalData[4 + N + 1] = InputTex.Load(dtID + int3(0, 2, 0));
        SharedHorizontalData[4 + N + 2] = InputTex.Load(dtID + int3(0, 3, 0));
        SharedHorizontalData[4 + N + 3] = InputTex.Load(dtID + int3(0, 4, 0));
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    int textureLoc = gtID.y;
    float4 blurredData = float4(0.0, 0.0, 0.0, 0.0);
    
    [unroll]
    for (int y = 0; y < 9; ++y)
    {
        blurredData += SharedHorizontalData[textureLoc + y] * gaussianWeights[y];
    }
    
    OutputTex[dtID.xy] = blurredData;
}