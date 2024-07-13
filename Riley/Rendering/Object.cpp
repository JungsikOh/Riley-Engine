#include "Object.h"
#include "../Graphics/DXBuffer.h"

namespace Riley {
void Mesh::Draw(ID3D11DeviceContext* context) const { Draw(context, topology); }

void Mesh::Draw(ID3D11DeviceContext* context,
                D3D11_PRIMITIVE_TOPOLOGY topology) const {

    context->IASetPrimitiveTopology(topology);
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexBufferDesc.stride,
                                0);

    if (indexBuffer != nullptr) {
        DXGI_FORMAT dxgiFormat = ConvertDXFormat(indexBufferDesc.format);
        context->IASetIndexBuffer(indexBuffer, dxgiFormat, 0);
        if (instanceBuffer != nullptr) {
            context->IASetVertexBuffers(1, 1, &instanceBuffer,
                                        &vertexBufferDesc.stride, 0);
        }
        context->DrawIndexedInstanced(indexCount, instanceCount, startIndexLoc,
                                      baseVertexLoc, startInstanceLoc);
    } else {
        if (instanceBuffer != nullptr) {
            context->IASetVertexBuffers(1, 1, &instanceBuffer,
                                        &vertexBufferDesc.stride, 0);
        }
        context->DrawInstanced(vertexCount, instanceCount, startVertexLoc,
                               startInstanceLoc);
    }
}

void Mesh::DeInit() {
    SAFE_RELEASE(vertexBuffer);
    SAFE_RELEASE(indexBuffer);
    SAFE_RELEASE(instanceBuffer);
}
} // namespace Riley