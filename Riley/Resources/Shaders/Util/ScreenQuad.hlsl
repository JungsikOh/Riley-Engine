struct VSToPS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

VSToPS ScreenQuad(uint vertexId : SV_VertexID)
{
    int2 texcoord = int2(vertexId & 1, vertexId >> 1);
    VSToPS output;
    output.texcoord = float2(texcoord);
    output.posWorld = float4(2 * (texcoord.x - 0.5), -2 * (texcoord.y - 0.5), 0.0, 1.0);
    return output;
}