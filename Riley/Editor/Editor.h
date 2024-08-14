#pragma once
#include "../Utilities/Singleton.h"
#include "../Rendering/SceneViewport.h"
#include "../Rendering/RenderSetting.h"

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

    void SetSelectedEntity();

  private:
    Engine* engine;
    ImGuiLayer* gui;
    entt::entity selected_entity = entt::null;
    RenderSetting renderSetting;
    bool isSceneFocused = false;
    SceneViewport sceneViewportData;

    std::array<bool, Flag_Count> window_flags = {true};

  private:
    void MenuBar(); 
    void Camera();
    void ListEntities();
    void Properties();
    void RenderSetting();
    void Scene();
};
} // namespace Riley