#pragma once
#include "Input.h"
#include <memory>
#include <optional>

namespace Riley {
struct WindowEventData;

struct EngineInit {
    bool vsync = false;
    Window* window = nullptr;
};

class Engine {
    friend class Editor;

  public:
    // = delete를 통해서 복사할당을 하는 것을 사전조치
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

    bool vsync;
    bool editor_active = true;

  private:
    void InitializeScene();
    void Update(float dt);
    void Render();
    void SetSceneViewportData();
};

} // namespace Riley