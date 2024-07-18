#include "Components.h"
#include "../Graphics/DXBuffer.h"

namespace Riley {
void Mesh::Draw(ID3D11DeviceContext* context) const { Draw(context, topology); }

void Mesh::Draw(ID3D11DeviceContext* context,
                D3D11_PRIMITIVE_TOPOLOGY topology) const {

    context->IASetPrimitiveTopology(topology);
    BindVertexBuffer(context, vertexBuffer.get(), 0, 0);

    if (indexBuffer != nullptr) {
        BindIndexBuffer(context, indexBuffer.get());
        if (instanceBuffer != nullptr) {
            BindVertexBuffer(context, instanceBuffer.get(), 1, 0);
        }
        context->DrawIndexedInstanced(indexCount, instanceCount, startIndexLoc,
                                      baseVertexLoc, startInstanceLoc);
    } else {
        if (instanceBuffer != nullptr) {
            BindVertexBuffer(context, instanceBuffer.get(), 1, 0);
        }
        context->DrawInstanced(vertexCount, instanceCount, startVertexLoc,
                               startInstanceLoc);
    }
}

} // namespace Riley