#include "Entity.h"

namespace Riley {

Entity::~Entity() {
    m_OnDestory();
    m_components.Iterate([](Component* comp) { comp->~Component(); });
}

void Entity::Update(const float& dt) {
    static float deltaTime = dt;
    m_components.Iterate([](Component* comp) { comp->Update(deltaTime); });
}

Component* Entity::GetComponent(uint16 index) { return m_components[index]; }

template <typename TComp> TComp* Entity::GetComponent() {
    return m_components.FindNodeByType<TComp>();
}

void Entity::AddComponent(Component* component) {
    m_components.AddFront(component);
}

void Entity::RemoveComponent(Component* component) {
    if (component == nullptr)
        return;
    if (!HasComponent(component))
        return;
    m_components.Remove(component);
    delete component;
}

template <typename TComp> void Entity::RemoveComponent() {
    TComp* component =
        m_components.Remove<TComp>(m_components.FindNodeByType<TComp>());
    component->~Component();
    delete component;
}

bool Entity::HasComponent(Component* component) {
    return m_components.FindeNodeFromPtr(component) != nullptr;
}

template <typename TComp> bool Entity::HasComponent() {
    return m_components.FindNodeByType<TComp>() != nullptr;
}

template <typename TComp> bool Entity::TryGetComponent(TComp** component) {
    return m_components.TryGetData<TComp>(component);
}

} // namespace Riley