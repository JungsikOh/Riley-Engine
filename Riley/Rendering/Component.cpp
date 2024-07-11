#include "Component.h"

namespace Riley {
void Mesh::Draw(ID3D11DeviceContext* context) const { Draw(context, topology); }
void Mesh::Draw(ID3D11DeviceContext* context,
                D3D11_PRIMITIVE_TOPOLOGY topology) const {
    context->IASetPrimitiveTopology(topology);
}
} // namespace Riley