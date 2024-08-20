#include "../Common.hlsli"

Texture2D InputTex : register(t0);
RWTexture2D<float4> OutputTex : register(u0);

static const uint N = 1024;

groupshared float4 SharedHorizontalData[4 + N + 4];

[numthreads(N, 1, 1)]
void BlurX(uint3 gID : SV_GroupID,
                uint3 gtID : SV_GroupThreadID,
                uint3 dtID : SV_DispatchThreadID,
                uint gIdx : SV_GroupIndex)
{
    float4 input = InputTex.Load(dtID);
    SharedHorizontalData[gtID.x + 4] = input;
    
    if(gIdx == 0)
    {
        SharedHorizontalData[0] = InputTex.Load(dtID - int3(4, 0, 0));
        SharedHorizontalData[1] = InputTex.Load(dtID - int3(3, 0, 0));
        SharedHorizontalData[2] = InputTex.Load(dtID - int3(2, 0, 0));
        SharedHorizontalData[3] = InputTex.Load(dtID - int3(1, 0, 0));
    }
    
    if (gIdx == N - 1)
    {
        SharedHorizontalData[4 + N + 0] = InputTex.Load(dtID + int3(1, 0, 0));
        SharedHorizontalData[4 + N + 1] = InputTex.Load(dtID + int3(2, 0, 0));
        SharedHorizontalData[4 + N + 2] = InputTex.Load(dtID + int3(3, 0, 0));
        SharedHorizontalData[4 + N + 3] = InputTex.Load(dtID + int3(4, 0, 0));
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    int textureLoc = gtID.x;
    float4 blurredData = float4(0.0, 0.0, 0.0, 0.0);
    
    [unroll]
    for (int x = 0; x < 9; ++x)
    {
        blurredData += SharedHorizontalData[textureLoc + x] * gaussianWeights[x];
    }
    
    OutputTex[dtID.xy] = blurredData;
}