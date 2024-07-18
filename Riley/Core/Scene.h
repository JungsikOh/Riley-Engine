#pragma once
#include <entt.hpp>

/**
 * registry is bascally a container for all of these components and entities.
 * 
 */

namespace Riley {
class Scene {
  public:
    Scene();
    ~Scene();

  private:
    entt::registry m_registry;
};
} // namespace Riley