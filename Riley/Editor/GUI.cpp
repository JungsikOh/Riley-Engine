#include "GUI.h"
#include "../Core/Paths.h"
#include "../Core/Window.h"
//#include "ImGui\ImGuizmo.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Riley {
GUI::GUI(ID3D11Device* device, ID3D11DeviceContext* context, Window* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigWindowsResizeFromEdges = true;
    io.ConfigViewportsNoTaskBarIcon = true;

    ImGui_ImplWin32_Init(window->Handle());
    ImGui_ImplDX11_Init(device, context);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.ScrollbarSize = 20.0f;
    style.FramePadding = ImVec2(5, 5);
    style.ItemSpacing = ImVec2(6, 5);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.WindowRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.PopupRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.Alpha = 1.0f;
}

GUI::~GUI() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void GUI::Begin() const {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void GUI::End() const {
    ImGui::Render();
    if (isVisible) {
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void GUI::HandleWindowMessage(WindowEventData const& data) const {
    ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(data.handle), data.msg,
                                   data.wparam, data.lparam);
}

void GUI::ToggleVisibility() { isVisible = !isVisible; }
bool GUI::IsVisible() const { return isVisible; }

} // namespace Riley