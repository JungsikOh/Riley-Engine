#pragma once
#include "../Rendering/ModelImporter.h"
#include "../Utilities/Delegate.h"
#include "Input.h"
#include <iostream>
#include <memory>
#include <optional>
#include <entt.hpp>

namespace Riley {

class Renderer;
class Camera;

struct WindowEventData;
struct EngineInit {
    bool vsync = false;
    Window* window = nullptr;
};

class Engine {
    friend class Editor;

  public:
    explicit Engine(EngineInit const&);
    Engine(Engine const&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine const&) = delete;
    Engine& operator=(Engine&&) = delete;
    ~Engine();

    void OnWindowEvent(WindowEventData const&);
    void CreateSwapChainAndDevice();

    void Run();
    void Present();

  private:
    /* Low Level APIs */
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;

    entt::registry m_registry;

    Window* window = nullptr;
    Renderer* renderer = nullptr;
    Camera* camera = nullptr;
    ModelImporter* modelImporter = nullptr;

    bool vsync;
    bool editor_active = true;

  private:
    void InitializeScene();
    void Update(float dt);
    void Render();
    void SetSceneViewportData();
};

} // namespace Riley