#pragma once
#include "../Utilities/Event.h"
#include "Entity.h"
#include <cstdint>
#include <list>

namespace Riley {

class Scene {
  public:
    std::list<Entity*> m_entities;

  public:
    std::string m_name;

  public:
    Scene() : m_name(std::string("New Scene")){};
    Scene(const std::string& _name) : m_name(_name){};
    ~Scene();
    Entity* GetCurrentEntity();
    void Update(const float& dt);
    void Unload();
    void Load();
    void Start(){};
    void UpdateEditor();
    void AddEntity(Entity* entity);
    void RemoveEntity(Entity* entity);
    Entity* FindEntityByName(const std::string& name);
};

namespace SceneManager {
Scene* GetCurrentScene();

void AddScene(Scene* scene);
void LoadNewScene();
void LoadScene(uint8 index);
void LoadScene(const std::string& name);
} // namespace SceneManager

} // namespace Riley