#include "Editor.h"
#include "../Core/Engine.h"
#include "../Core/Window.h"
#include "../Math/BoundingVolume.h"
#include "../Math/MatrixMath.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/SceneViewport.h"
#include "../Utilities/Delegate.h"
#include "ImGuiLayer.h"

#include <functional>
#include <optional>

namespace Riley
{
Editor::Editor(EngineInit const& init)
{
    engine = new Engine(init);
    gui = new ImGuiLayer(engine);

    gui->OnAttach();
}

Editor::~Editor()
{
    gui->OnDetach();
    SAFE_DELETE(gui);
    SAFE_DELETE(engine);
}

void Editor::OnWindowEvent(WindowEventData const& msg_data)
{
    engine->OnWindowEvent(msg_data);
    gui->HandleWindowMessage(msg_data);
}

void Editor::Run()
{
    static constexpr float clearDarkBlue[4] = {0.0f, 0.21f, 0.38f, 0.0f};
    static bool started = false;

    if (started)
    {
        engine->SetSceneViewportData(sceneViewportData);
        engine->Run(renderSetting);
        engine->GetCamera()->SetAspectRatio(sceneViewportData.GetWidth() / sceneViewportData.GetHeight());
        engine->GetBackbufferRTV()->Clear(engine->GetDeviceContext(), clearDarkBlue);
        engine->GetBackbufferRTV()->BindRenderTargets(engine->GetDeviceContext());
    }
    else
    {
        engine->SetSceneViewportData(std::nullopt);
        engine->Run(renderSetting);
        engine->GetBackbufferRTV()->Clear(engine->GetDeviceContext(), clearDarkBlue);
        engine->GetBackbufferRTV()->BindRenderTargets(engine->GetDeviceContext());
        started = true;
    }
    gui->Begin();
    {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        MenuBar();
        Camera();
        Scene();
        RenderSetting();
        ListEntities();
        Properties();
    }
    gui->End();
    engine->Present();
}

void Editor::MenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        ImGui::EndMainMenuBar();
    }
}

void Editor::Camera()
{
    // if (!window_flags[Flag_Camera])
    //     return;
    auto camera = engine->GetCamera();
    if (ImGui::Begin("Camera", &window_flags[Flag_Camera]))
    {
        if (ImGui::CollapsingHeader("Camera"))
        {
            Vector3 cam_pos = camera->Position();
            float pos[3] = {cam_pos.x, cam_pos.y, cam_pos.z};
            ImGui::SliderFloat3("Position", pos, 0.0f, 10000.0f);
            camera->SetPosition(Vector3(pos));
            float near_plane = camera->Near(), far_plane = camera->Far();
            float _fov = camera->Fov(), _ar = camera->AspectRatio();
            ImGui::SliderFloat("Near Plane", &near_plane, 0.005f, 2.0f);
            ImGui::SliderFloat("Far Plane", &far_plane, 10.0f, 5000.0f);
            ImGui::SliderFloat("FOV", &_fov, 0.01f, 120.0f);
            camera->SetNearAndFar(near_plane, far_plane);
            camera->SetFov(_fov);
            float color[3] = {camera->GetGlobalAmbient().x, camera->GetGlobalAmbient().y, camera->GetGlobalAmbient().z};
            ImGui::ColorEdit3("GlobalAmbient", color);
            camera->SetGlobalAmbient(Vector4(color[0], color[1], color[2], 1.0f));
        }
    }
    ImGui::End();
}

void Editor::Scene()
{
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
        ImGui::Image(engine->renderer->GetOffScreenRTV()->SRV(), size);
        ImGui::GetForegroundDrawList()->AddRect(v_min, v_max, IM_COL32(255, 0, 0, 255));

        ImVec2 mouse_pos = ImGui::GetMousePos();
        sceneViewportData.m_mousePositionX = mouse_pos.x - v_min.x;
        sceneViewportData.m_mousePositionY = mouse_pos.y - v_min.y;
        sceneViewportData.isViewportFocused = isSceneFocused;
        sceneViewportData.SetTopLeftX(v_min.x);
        sceneViewportData.SetTopLeftY(v_min.y);
        sceneViewportData.SetWidth(size.x);
        sceneViewportData.SetHeight(size.y);
        sceneViewportData.m_widthImGui = size.x;
        sceneViewportData.m_heightImGui = size.y;
    }
    ImGui::End();
}

void Editor::RenderSetting()
{
    ImGui::Begin("Postprocessing");
    {
        if (ImGui::TreeNode("AO"))
        {
            const char* aoTypes[] = {"None", "SSAO"};
            static int currentAoType = 1;
            const char* comboLabel = aoTypes[currentAoType];
            if (ImGui::BeginCombo("AO", comboLabel, 0))
            {
                for (int n = 0; n < IM_ARRAYSIZE(aoTypes); ++n)
                {
                    const bool isSelected = (currentAoType == n);
                    if (ImGui::Selectable(aoTypes[n], isSelected))
                        currentAoType = n;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            renderSetting.ao = static_cast<AmbientOcclusion>(currentAoType);
            if (renderSetting.ao == AmbientOcclusion::SSAO)
            {
                ImGui::SliderFloat("Power", &renderSetting.ssaoPower, 1.0f, 16.0f);
                ImGui::SliderFloat("Radius", &renderSetting.ssaoRadius, 0.05f, 4.0f);
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void Editor::ListEntities()
{
    // if (!window_flags[Flag_Entities])
    //     return;
    auto all_entities = engine->m_registry.view<Tag>();
    if (ImGui::Begin("Entities", &window_flags[Flag_Entities]))
    {
        std::function<void(entt::entity, bool)> ShowEntity;
        ShowEntity = [&](entt::entity e, bool firstIteration) {
            Relationship* relationship = engine->m_registry.try_get<Relationship>(e);
            if (firstIteration && relationship && relationship->parent != entt::null)
                return;
            auto& tag = all_entities.get<Tag>(e);

            ImGuiTreeNodeFlags flags = ((selected_entity == e) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

            bool opened = ImGui::TreeNodeEx(tag.name.c_str(), flags);

            if (ImGui::IsItemClicked())
            {
                auto aabb = engine->m_registry.try_get<AABB>(selected_entity);
                if (aabb)
                    aabb->isDrawAABB = false;
                if (e == selected_entity)
                {
                    selected_entity = entt::null;
                }
                else
                {
                    selected_entity = e;
                    auto aabb = engine->m_registry.try_get<AABB>(selected_entity);
                    if (aabb)
                        aabb->isDrawAABB = true;
                }
            }

            if (opened)
            {
                if (relationship)
                {
                    for (size_t i = 0; i < relationship->childrenCount; ++i)
                    {
                        ShowEntity(relationship->children[i], false);
                    }
                }
                ImGui::TreePop();
            }
        };
        for (auto e : all_entities)
        {
            ShowEntity(e, true);
        }
    }
    ImGui::End();
}

void Editor::Properties()
{
    if (ImGui::Begin("Properties", &window_flags[Flag_Entities]))
    {
        if (selected_entity != entt::null)
        {
            auto tag = engine->m_registry.try_get<Tag>(selected_entity);
            if (tag)
            {
                char buffer[256];
                memset(buffer, 0, sizeof(buffer));
                std::strncpy(buffer, tag->name.c_str(), sizeof(buffer));
                if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
                    tag->name = std::string(buffer);
            }

            auto light = engine->m_registry.try_get<Light>(selected_entity);
            if (light && ImGui::CollapsingHeader("Light", 1))
            {
                if (light->type == LightType::Directional)
                    ImGui::Text("Directional Light");
                else if (light->type == LightType::Spot)
                    ImGui::Text("Spot Light");
                else if (light->type == LightType::Point)
                    ImGui::Text("Point Light");

                Vector4 lightColor = light->color, lightDirection = light->direction, lightPosition = light->position;
                float color[3] = {lightColor.x, lightColor.y, lightColor.z};
                ImGui::ColorEdit3("Light Color", color);
                light->color = Vector4(color[0], color[1], color[2], 1.0f);

                ImGui::SliderFloat("Light Energy", &light->energy, 0.0f, 50.0f);

                if (light->type == LightType::Directional || light->type == LightType::Spot)
                {
                    float direction[3] = {lightDirection.x, lightDirection.y, lightDirection.z};
                    ImGui::SliderFloat3("Light Direction", direction, -1.0f, 1.0f);
                    light->direction = Vector4(direction[0], direction[1], direction[2], 0.0f);
                    if (light->type == LightType::Directional)
                    {
                        light->position = -light->direction * 1e3;
                    }
                }

                if (light->type == LightType::Spot || light->type == LightType::Point)
                {
                    float position[3] = {lightPosition.x, lightPosition.y, lightPosition.z};
                    ImGui::SliderFloat3("Light Position", position, -10.0f, 10.0f);
                    light->position = Vector4(position[0], position[1], position[2], 1.0f);
                }
            }

            auto transform = engine->m_registry.try_get<Transform>(selected_entity);
            if(light) transform->currentTransform = Matrix::CreateTranslation(Vector3(light->position));

            if (!light && transform && ImGui::CollapsingHeader("Transform"))
            {
                Matrix tr = transform->currentTransform;
                Vector3 translation = tr.Translation();
                Quaternion q = Quaternion::CreateFromRotationMatrix(ExtractRoationMatrix(tr));
                Vector3 rotation = q.ToEuler();
                Vector3 scale = ExtractScaleFromMatrix(tr);

                ImGui::SliderFloat3("Position", &translation.x, -5.0f, 5.0f);
                ImGui::SliderFloat3("Rotate", &rotation.x, -1.0f, 1.0f);
                ImGui::SliderFloat3("Scale", &scale.x, 0.1f, 10.0f);

                tr = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(Quaternion::CreateFromYawPitchRoll(rotation)) *
                     Matrix::CreateTranslation(translation);

                auto aabb = engine->m_registry.try_get<AABB>(selected_entity);
                if (aabb)
                {
                    aabb->boundingBox = aabb->orginalBox;
                    aabb->boundingBox.Transform(aabb->boundingBox, tr);
                    aabb->UpdateBuffer(engine->GetDevice());
                }
                transform->currentTransform = tr;
            }
        }
        ImGui::End();
    }
}

} // namespace Riley