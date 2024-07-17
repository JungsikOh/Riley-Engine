#include "Object.h"
#include "../Graphics/DXBuffer.h"

namespace Riley {
void Mesh::Draw(ID3D11DeviceContext* context) const { Draw(context, topology); }

void Mesh::Draw(ID3D11DeviceContext* context,
                D3D11_PRIMITIVE_TOPOLOGY topology) const {

    context->IASetPrimitiveTopology(topology);
    BindVertexBuffer(context, vertexBuffer, 0, 0);

    if (indexBuffer != nullptr) {
        BindIndexBuffer(context, indexBuffer);
        if (instanceBuffer != nullptr) {
            BindVertexBuffer(context, instanceBuffer, 1, 0);
        }
        context->DrawIndexedInstanced(indexCount, instanceCount, startIndexLoc,
                                      baseVertexLoc, startInstanceLoc);
    } else {
        if (instanceBuffer != nullptr) {
            BindVertexBuffer(context, instanceBuffer, 1, 0);
        }
        context->DrawInstanced(vertexCount, instanceCount, startVertexLoc,
                               startInstanceLoc);
    }
}

void Mesh::DeInit() {
    SAFE_DELETE(vertexBuffer);
    SAFE_DELETE(indexBuffer);
    SAFE_DELETE(instanceBuffer);
}
} // namespace Riley