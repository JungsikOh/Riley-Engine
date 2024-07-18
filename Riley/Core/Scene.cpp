#include "Scene.h"
#include "../Math/MathTypes.h"

// entt::entity == uint32
namespace Riley {

static void DoMath(const Matrix& transform) {}

Scene::Scene() {

    struct MeshComponent {
        Matrix Transform;
        operator Matrix&() { return Transform; }
        operator const Matrix&() const { return Transform; }
    };

    struct TransformComponent {
        Matrix Transform;
        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const Matrix& transform) : Transform(transform) {}

        operator Matrix&() { return Transform; }
        operator const Matrix&() const { return Transform; }
    };

    entt::entity entity = m_registry.create();
    auto& transform = m_registry.emplace<TransformComponent>(entity, Matrix());

    auto view = m_registry.view<TransformComponent>();
    for (auto e : view) {
        auto& transform = view.get<TransformComponent>(e);
        TransformComponent& transform2 =
            m_registry.get<TransformComponent>(e);
    }

    auto group = m_registry.group<TransformComponent>(entt::get<MeshComponent>);
    for (auto e : group) {
        auto [transform, mesh] =
            group.get<TransformComponent, MeshComponent>(e);
    }
};
Scene::~Scene(){};

} // namespace Riley