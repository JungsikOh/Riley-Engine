#pragma once
#include "../Core/Engine.h"
#include "../Core/Window.h"
#include "../Rendering/SceneViewport.h"
#include "GUI.h"
#include "imgui.h"
#include "entt.hpp"
#include <memory>
#include <array>

namespace Riley {
struct Material;
struct WindowEventData;

struct EditorInit {
    EngineInit engineInit;
};

class Editor {
    enum {
        Flag_Profiler,
        Flag_Camera,
        Flag_Log,
        Flag_Entities,
        Flag_AddEntities,
        Flag_Count
    };

  public:
    explicit Editor(EditorInit const& init);
    ~Editor();
    void OnWindowEvent(WindowEventData const&);
    void Run();

  private:
    Engine* engine;
    GUI* gui;
    entt::entity selected_entity = entt::null;
    bool isSceneFocused = false;
    SceneViewport sceneViewportData;

    std::array<bool, Flag_Count> window_flags = {false};

    private:
    void HandleInput();
    void MenuBar();
    void Camera();
    void Scene();
};

} // namespace Riley