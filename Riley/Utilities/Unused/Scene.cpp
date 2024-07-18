#include "Scene.h"
#include "Entity.h"

namespace Riley {

Scene::~Scene() { Unload(); }

void Scene::Update(const float& dt) {
    for (auto& entity : m_entities)
        entity->Update(dt);
}

static Entity* currEntity = nullptr;

Entity* Scene::GetCurrentEntity() { return currEntity; }

static void AddComponent() {
    if (!currEntity)
        return;

    TestComponent* _component = new TestComponent();
    currEntity->AddComponent((Component*)_component);
    _component->SetEntity(currEntity);
}

Entity* Scene::FindEntityByName(const std::string& name) {
    for (auto& entity : m_entities) {
        if (entity->m_name == name)
            return entity;
    }
}

void Scene::Load() {
}

void Scene::Unload() {
    for (auto& entity : m_entities) {
        entity->~Entity();
    }
    m_entities.clear();
}

void Scene::AddEntity(Entity* entity) { m_entities.push_back(entity); }
void Scene::RemoveEntity(Entity* entity) { m_entities.remove(entity); }

namespace SceneManager {

std::list<Scene*> scenes;
Scene* currentScene;

Scene* GetCurrentScene() { return currentScene; }

void AddScene(Scene* scene) { scenes.push_back(scene); }
void LoadNewScene() {
    scenes.push_back(new Scene());
    currentScene = scenes.back();
}
void LoadScene(uint8 index) {
    if (currentScene) {
        currentScene->Unload();
    }
    auto iter = scenes.begin();
    for (uint8 i = 0; index < i; i++, ++iter);
    currentScene = *iter;
    currentScene->Load();
}

void LoadScene(const std::string& name) {
    auto iter = scenes.begin();
    for (auto& scene : scenes) {
        if (scene->m_name == name) {
            currentScene = scene;
            scene->Load();
            break;
        }
    }
}
} // namespace SceneManager

} // namespace Riley