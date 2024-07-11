#pragma once
#include "../Utilities/Event.h"
#include "Transform.h"
#include "Entity.h"
#include <iostream>
#include <string>

namespace Riley {

class Entity;

class Component {
  public:
    std::string m_name;
    Event m_OnDestroyed;
    Entity* m_entity;
    Transform* m_transform;

  public:
    Component()
        : m_name(std::string(typeid(*this).name())),
          m_transform(new Transform()) {}
    Component(const std::string& _name) : m_name(_name) {}

    virtual void SetEntity(Entity* entity) = 0;
    virtual Entity* GetEntity() = 0;

    // virtual ~Component() { OnDestroyed() ; };
    virtual void Update(const float& deltaTime) = 0;
#ifndef NEDITOR
    virtual void OnEditor() = 0;
#endif
};

} // namespace Riley