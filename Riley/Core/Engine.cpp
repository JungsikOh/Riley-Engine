#include "Engine.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/ShaderManager.h"
#include "../Utilities/Timer.h"
#include "Input.h"
#include "ECS.h"
#include "Window.h"
#include <iostream>
#include <optional>
#include <tuple>

// std::ignore
// std::tuple로 반환되는 결과를 std::tie로 연결해서 받을 때 사용하지 않는 값을
// 처리하기 위함. std::tie(std::ignore, inserted) = set_of_str.insert("Test")
// 이런식으로 사용.

namespace Riley {

static const CameraParameters ParseCameraParam() {
    CameraParameters cp;
    cp.fov = 90.0f;
    cp.aspectRatio = 16.0f / 9.0f;
    cp.nearPlane = 0.05f;
    cp.farPlane = 50.0f;
    cp.transform = Transform(Vector3(0.5f, 0.0f, -1.0f),
                             Quaternion(Quaternion::CreateFromAxisAngle(
                                 Vector3(-1.0f, 0.0f, 0.0f), 0.0f)));
    return cp;
}

Engine::Engine(EngineInit const& init)
    : window(init.window), vsync{init.vsync} {

    CameraParameters cp = ParseCameraParam();
    camera = new Camera(cp);
    renderer = new Renderer(window, camera, window->Width(), window->Height());

    InputEvents& inputEvents = g_Input.GetInputEvents();
    std::ignore = inputEvents.window_resized_event.AddMember(
        &Renderer::OnResize, *renderer);
    // std::ignore = inputEvents.left_mouse_clicked.Add(
    //     [this](int32 mx, int32 my) { renderer->OnLeftMouseClicked(); });
    std::ignore =
        inputEvents.window_resized_event.AddMember(&Camera::OnResize, *camera);
}

Engine::~Engine() {
    SAFE_DELETE(renderer);
    SAFE_DELETE(camera);
}

void Engine::OnWindowEvent(WindowEventData const& data) {
    g_Input.OnWindowEvent(data);
}

void Engine::Run() {
    static RileyTimer timer;
    float const dt = timer.MarkInSeconds();

    g_Input.Tick();
    if (true) {
        Update(dt);
        Render();
    }
}

void Engine::Update(float dt) {
    camera->Tick(dt);
    renderer->Tick(camera);
    renderer->Update(dt);
}

void Engine::Render() {
    renderer->Render();
    renderer->Present(true);
}

} // namespace Riley