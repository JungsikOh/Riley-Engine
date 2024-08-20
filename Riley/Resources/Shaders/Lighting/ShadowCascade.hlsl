#include "../Common.hlsli"

struct VSInput
{
    float3 posModel : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VSToGS
{
    float4 posWorld : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

VSToGS ShadowCascadeVS(VSInput input)
{
    VSToGS output;
    output.posWorld = mul(float4(input.posModel, 1.0), meshData.world);
    output.texcoord = input.texcoord;
    
    return output;
}

struct GSToPS
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    uint layer : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3 * CASCADE_COUNT)]
void ShadowCascadeGS(triangle VSToGS input[3], inout TriangleStream<GSToPS> triStream)
{
    for (int face = 0; face < CASCADE_COUNT; ++face)
    {
        GSToPS output;
        output.layer = face;
        for (int i = 0; i < 3; ++i)
        {
            output.posProj = mul(input[i].posWorld, shadowData.shadowCascadeMapViewProj[face]);
            output.texcoord = input[i].texcoord;
            triStream.Append(output);
        }
        triStream.RestartStrip();
    }
}

void ShadowCascadePS(GSToPS input)
{
}