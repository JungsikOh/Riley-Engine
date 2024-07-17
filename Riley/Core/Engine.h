#pragma once
#include "../Utilities/Delegate.h"
#include "Input.h"
#include <memory>
#include <optional>

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

    void Run();
    void Present();

  private:
    Window* window = nullptr;
    Renderer* renderer = nullptr;
    Camera* camera = nullptr;

    bool vsync;
    bool editor_active = true;

  private:
    void InitializeScene();
    void Update(float dt);
    void Render();
    void SetSceneViewportData();
};

} // namespace Riley