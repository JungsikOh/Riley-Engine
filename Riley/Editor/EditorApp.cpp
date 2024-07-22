#include "Editor.h"
#include "../Rendering/Renderer.h"
#include "../Utilities/StringUtil.h"
#include "spdlog\spdlog.h"

namespace Riley {
Editor::Editor(EditorInit const& init) {
    engine = new Engine(init.engineInit);
    gui = new GUI(engine->GetDevice(), engine->GetDeviceContext(),
                  init.engineInit.window);
}

Editor::~Editor() {
    SAFE_DELETE(engine);
    SAFE_DELETE(gui);
}

void Editor::OnWindowEvent(WindowEventData const& msg_data) {
    engine->OnWindowEvent(msg_data);
    gui->HandleWindowMessage(msg_data);
}

void Editor::Run() {
    if (gui->IsVisible()) {
        engine->SetSceneViewportData(sceneViewportData);
        engine->Run();
        gui->Begin();
        {
            MenuBar();
            Camera();
            Scene();
        }
        gui->End();
        engine->Present();
    } else {
        engine->SetSceneViewportData(std::nullopt);
        engine->Run();
        engine->Present();
    }
}

void Editor::MenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Windows")) {
            if (ImGui::MenuItem("Profiler", 0, window_flags[Flag_Profiler]))
                window_flags[Flag_Profiler] = !window_flags[Flag_Profiler];
            if (ImGui::MenuItem("Log", 0, window_flags[Flag_Log]))
                window_flags[Flag_Log] = !window_flags[Flag_Log];
            if (ImGui::MenuItem("Camera", 0, window_flags[Flag_Camera]))
                window_flags[Flag_Camera] = !window_flags[Flag_Camera];
            if (ImGui::MenuItem("Entities", 0, window_flags[Flag_Entities]))
                window_flags[Flag_Entities] = !window_flags[Flag_Entities];
            if (ImGui::MenuItem("Add Entities", 0,
                                window_flags[Flag_AddEntities]))
                window_flags[Flag_AddEntities] =
                    !window_flags[Flag_AddEntities];

            ImGui::EndMenu();
        }
    }
}

void Editor::Camera() {
    if (!window_flags[Flag_Camera])
        return;
    auto& camera = *engine->camera;
    if (ImGui::Begin("Camera", &window_flags[Flag_Camera])) {
        Vector3 cam_pos = camera.Position();
        float pos[3] = {cam_pos.x, cam_pos.y, cam_pos.z};
        ImGui::SliderFloat3("Position", pos, 0.0f, 10000.0f);
        camera.SetPosition(Vector3(pos));
        float near_plane = camera.Near(), far_plane = camera.Far();
        float _fov = camera.Fov(), _ar = camera.AspectRatio();
        ImGui::SliderFloat("Near Plane", &near_plane, 0.0f, 2.0f);
        ImGui::SliderFloat("Far Plane", &far_plane, 10.0f, 5000.0f);
        ImGui::SliderFloat("FOV", &_fov, 0.01f, 1.5707f);
        camera.SetNearAndFar(near_plane, far_plane);
        camera.SetFov(_fov);
    }
    ImGui::End();
}

void Editor::Scene() {
    ImGui::Begin("Scene");
    {
        ImVec2 v_min = ImGui::GetWindowContentRegionMin();
        ImVec2 v_max = ImGui::GetWindowContentRegionMax();
        v_min.x += ImGui::GetWindowPos().x;
        v_min.y += ImGui::GetWindowPos().y;
        v_max.x += ImGui::GetWindowPos().x;
        v_max.y += ImGui::GetWindowPos().y;
        ImVec2 size(v_max.x - v_min.x, v_max.y - v_min.y);

        isSceneFocused = ImGui::IsWindowFocused();
        ImVec2 mouse_pos = ImGui::GetMousePos();
        sceneViewportData.m_mousePositionX = mouse_pos.x;
        sceneViewportData.m_mousePositionY = mouse_pos.y;
        sceneViewportData.isViewportFocused = isSceneFocused;
        sceneViewportData.m_topLeftX = v_min.x;
        sceneViewportData.m_topLeftY = v_min.y;
        sceneViewportData.m_width = size.x;
        sceneViewportData.m_height = size.y;
    }
    ImGui::End();
}

} // namespace Riley