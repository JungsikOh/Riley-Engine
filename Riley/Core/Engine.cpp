#include "Engine.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/ShaderManager.h"
#include "../Utilities/Timer.h"
#include "Input.h"
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
    cp.position = Vector3(0.0f, 0.0f, -1.0f);
    cp.lootAt = Vector3(0.0f, 0.0f, 1.0f);

    return cp;
}

Engine::Engine(EngineInit const& init)
    : window(init.window), vsync{init.vsync} {
    
    CreateSwapChainAndDevice();
    CameraParameters cp = ParseCameraParam();
    camera = new Camera(cp);
    renderer = new Renderer(window, m_registry, m_device, m_context, m_swapChain, camera, window->Width(), window->Height());
    modelImporter = new ModelImporter(m_device, m_registry);

    InputEvents& inputEvents = g_Input.GetInputEvents();
    std::ignore = inputEvents.window_resized_event.AddMember(
        &Renderer::OnResize, *renderer);
    // std::ignore = inputEvents.left_mouse_clicked.Add(
    //     [this](int32 mx, int32 my) { renderer->OnLeftMouseClicked(); });
    std::ignore =
        inputEvents.window_resized_event.AddMember(&Camera::OnResize, *camera);

    // testing Add entity
    modelImporter->LoadSquare(Vector3(0.3f, 0.2f, 0.0f), 0.2f);
}

Engine::~Engine() {
    SAFE_DELETE(renderer);
    SAFE_DELETE(camera);
    SAFE_DELETE(modelImporter);

    SAFE_RELEASE(m_swapChain);
    SAFE_RELEASE(m_device);
    SAFE_RELEASE(m_context);
}

void Engine::OnWindowEvent(WindowEventData const& data) {
    g_Input.OnWindowEvent(data);
}

void Engine::CreateSwapChainAndDevice() {
    /* Initalize swapChain */
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = window->Width();
    sd.BufferDesc.Height = window->Height();
    sd.BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.OutputWindow = static_cast<HWND>(window->Handle());
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = 0;

    UINT swapChainCreateFlags = 0;
#if defined(_DEBUG)
    swapChainCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;

    HR(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                     swapChainCreateFlags, featureLevels, 1,
                                     D3D11_SDK_VERSION, &sd, &m_swapChain,
                                     &m_device, &featureLevel, &m_context));
}

void Engine::Run() {
    static RileyTimer timer;
    float const dt = timer.MarkInSeconds();

    g_Input.Tick();
    if (true) {
        Update(dt);
        Render();
        Present();
    }
}

void Engine::Present() {
    if (m_swapChain != nullptr) {
        m_swapChain->Present(1, 0);
    }
}

void Engine::Update(float dt) {
    camera->Tick(dt);
    renderer->Tick(camera);
    renderer->Update(dt);
}

void Engine::Render() {
    renderer->Render();
}

} // namespace Riley