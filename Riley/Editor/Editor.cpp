#include "Editor.h"
#include "../Core/Engine.h"
#include "../Core/Window.h"
#include "../Utilities/Delegate.h"
#include "../Rendering/SceneViewport.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Camera.h"
#include "ImGuiLayer.h"

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
   SAFE_DELETE(engine);

   gui->OnDetach();
   SAFE_DELETE(gui);
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
         engine->Run();
         engine->GetCamera()->SetAspectRatio(sceneViewportData.GetWidth() / sceneViewportData.GetHeight());
         engine->GetBackbufferRTV()->Clear(engine->GetDeviceContext(), clearDarkBlue);
         engine->GetBackbufferRTV()->BindRenderTargets(engine->GetDeviceContext());
      }
   else
      {
         engine->SetSceneViewportData(std::nullopt);
         engine->Run();
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
      ListEntities();
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
      ImGui::Image(engine->GetRenderer()->hdrRTV->SRV(), size);
      ImGui::GetForegroundDrawList()->AddRect(v_min, v_max, IM_COL32(255, 0, 0, 255));

      ImVec2 mouse_pos = ImGui::GetMousePos();
      sceneViewportData.m_mousePositionX = std::clamp(mouse_pos.x - v_min.x, 0.0f, size.x);
      sceneViewportData.m_mousePositionY = std::clamp(mouse_pos.y - v_min.y, 0.0f, size.y);
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

void Editor::ListEntities()
{
   //if (!window_flags[Flag_Entities])
   //   return;
   static bool isSelected = true;
   if (ImGui::Begin("Properties", &window_flags[Flag_Entities]))
      {
         int i = 0;
         auto aabbView = engine->m_registry.view<Material,Transform, AABB>();
         for (auto& entity : aabbView)
            {
               i++;
               auto [material, transform, aabb] = aabbView.get<Material, Transform, AABB>(entity);
               if (aabb.isDrawAABB && isSelected)
                  {
                     ImGui::PushID(i);
                     Matrix tr = transform.currentTransform;
                     Vector3 translation = tr.Translation();
                     tr.Translation(Vector3(0.0f));
                     ImGui::SliderFloat3("Position", &translation.x, -5.0f, 5.0f);
                     ImGui::SliderFloat3("Albedo Color", &material.diffuse.x, 0.0f, 1.0f);

                     aabb.boundingBox.Transform(aabb.boundingBox, transform.currentTransform.Invert());

                     transform.currentTransform = tr * Matrix::CreateTranslation(translation);
                     aabb.boundingBox.Transform(aabb.boundingBox, transform.currentTransform);
                     aabb.UpdateBuffer(engine->GetDevice());

                     isSelected = true;
                     ImGui::PopID();
                  }
            }
      }
   ImGui::End();
}

} // namespace Riley