#pragma once
#include "../Utilities/Singleton.h"
#include "../Rendering/SceneViewport.h"

namespace Riley {

class Engine;
class Window;
class ImGuiLayer;

struct WindowEventData;
struct EngineInit;

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
    Editor(EngineInit const& init);
    ~Editor();
    void OnWindowEvent(WindowEventData const&);
    void Run();

  private:
    Engine* engine;
    ImGuiLayer* gui;
    entt::entity selected_entity = entt::null;
    bool isSceneFocused = false;
    SceneViewport sceneViewportData;

    std::array<bool, Flag_Count> window_flags = {true};

  private:
    void MenuBar(); 
    void Camera();
    void ListEntities();
    void Scene();
};
} // namespace Riley