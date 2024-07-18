#pragma once
#include "../Utilities/Event.h"
#include "../Utilities/LinkedList.h"
#include "Component.h"
#include "CoreTypes.h"
#include "Transform.h"

#include <cstdint>
#include <list>
#include <memory>

namespace Riley {

class Entity {
  public:
    std::string m_name;
    Event m_OnDestory;
    LinkedList<Component> m_components;

  public:
    Entity()
        : m_name(std::string("Entity")){};
    Entity(const std::string& _name) { m_name = _name; };
    ~Entity();

    void Update(const float& dt);

    Component* GetComponent(uint16 index);
    template <typename TComp> TComp* GetComponent();

    void AddComponent(Component* component);
    void RemoveComponent(Component* component);

    template <typename TComp> void RemoveComponent();
    template <typename TComp> bool TryGetComponent(TComp** component);
    template <typename TComp> bool HasComponent();
    bool HasComponent(Component* component);
};

class TestComponent : Component {
  public:
    void SetEntity(Entity* _entity) { m_entity = _entity; }
    Entity* GetEntity() { return m_entity; }

    void Update(const float&) {}
#ifndef NEDITOR
    void OnEditor() {}
#endif
};

} // namespace Riley