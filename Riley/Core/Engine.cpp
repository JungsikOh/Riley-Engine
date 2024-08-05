#include "../Rendering/Camera.h"
#include "../Rendering/Components.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/ShaderManager.h"
#include "../Rendering/RenderSetting.h"
#include "../Utilities/Timer.h"
#include "Engine.h"
#include "Window.h"

// std::ignore
// std::tuple·Î ¹ÝÈ¯µÇ´Â °á°ú¸¦ std::tie·Î ¿¬°áÇØ¼­ ¹ÞÀ» ¶§ »ç¿ëÇÏÁö ¾Ê´Â °ªÀ»
// Ã³¸®ÇÏ±â À§ÇÔ. std::tie(std::ignore, inserted) = set_of_str.insert("Test")
// ÀÌ·±½ÄÀ¸·Î »ç¿ë.

namespace Riley
{

static const CameraParameters ParseCameraParam()
{
   CameraParameters cp;
   cp.fov = 90.0f;
   cp.aspectRatio = 16.0f / 9.0f;
   cp.nearPlane = 0.05f;
   cp.farPlane = 50.0f;
   cp.position = Vector3(0.0f, 0.0f, -1.0f);
   cp.lootAt = Vector3(0.0f, 0.0f, 1.0f);
   cp.sensitivity = 0.3f;

   return cp;
}

Engine::Engine(EngineInit const& init)
    : window(init.window)
    , vsync{init.vsync}
{
   CreateSwapChainAndDevice();
   CreateBackBufferResources(window->Width(), window->Height());

   CameraParameters cp = ParseCameraParam();
   camera = new Camera(cp);
   renderer = new Renderer(m_registry, m_device, m_context, camera, window->Width(), window->Height());
   modelImporter = new ModelImporter(m_device, m_registry);

   InputEvents& inputEvents = g_Input.GetInputEvents();
   std::ignore = inputEvents.window_resized_event.AddMember(&Engine::ResizeBackbuffer, *this);
   std::ignore = inputEvents.window_resized_event.AddMember(&Renderer::OnResize, *renderer);
   std::ignore = inputEvents.left_mouse_clicked.Add([this](uint32 mx, uint32 my) {
      renderer->OnLeftMouseClicked(mx, my);
   });
   std::ignore = inputEvents.window_resized_event.AddMember(&Camera::OnResize, *camera);

   // testing Add entity
   modelImporter->LoadBox(Vector3(0.3f, 0.2f, 0.0f), 0.2f);
   modelImporter->LoadBox(Vector3(-0.3f, -0.1f, 0.0f), 0.4f);
   modelImporter->LoadSqhere(Vector3(0.2f, 0.05f, 0.3f), 0.3f, 40, 20);
   modelImporter->LoadSqhere(Vector3(0.2f, 0.05f, 1.3f), 0.5f, 40, 20);
   modelImporter->LoadSquare(Vector3(0.0f, -1.0f, 0.0f), 2.0f, 90.0f);

   Light light;
   light.position = Vector4(0.5f, 0.2f, 0.5f, 1.0f);
   light.color = Vector4(0.7f, 0.7f, 0.7f, 0.0f);
   light.direction = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
   light.energy = 0.3f;
   light.type = LightType::Directional;
   modelImporter->LoadLight(light, LightMesh::Cube, 0.03f);

   light.position = Vector4(0.6f, 1.8f, 0.02f, 1.0f);
   light.direction = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
   light.color = Vector4(1.0f, 0.2f, 1.0f, 0.0f);
   light.energy = 1.0f;
   light.range = 100.0f;
   light.type = LightType::Spot;
   modelImporter->LoadLight(light, LightMesh::Cube, 0.03f);

   light.position = Vector4(0.0f, 1.8f, 0.6f, 1.0f);
   light.direction = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
   light.color = Vector4(1.0f, 1.0f, 0.2f, 0.0f);
   light.energy = 1.0f;
   light.range = 100.0f;
   light.type = LightType::Point;
   modelImporter->LoadLight(light, LightMesh::Cube, 0.03f);
}

Engine::~Engine()
{
   SAFE_DELETE(camera);
   SAFE_DELETE(modelImporter);
   m_registry.clear();

   SAFE_DELETE(renderer);
   SAFE_DELETE(backBufferDSV);
   SAFE_DELETE(backBufferRTV);

   SAFE_RELEASE(m_swapChain);
   SAFE_RELEASE(m_context);
   SAFE_RELEASE(m_device);

   m_debugLayer->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
   SAFE_RELEASE(m_debugLayer);
}

void Engine::OnWindowEvent(WindowEventData const& data)
{
   g_Input.OnWindowEvent(data);
}

void Engine::CreateSwapChainAndDevice()
{
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

   const D3D_FEATURE_LEVEL featureLevels[2] = {D3D_FEATURE_LEVEL_11_0, // ´õ ³ôÀº ¹öÀüÀÌ ¸ÕÀú ¿Àµµ·Ï ¼³Á¤
                                               D3D_FEATURE_LEVEL_9_3};
   D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;

   HR(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, swapChainCreateFlags, featureLevels, 1,
                                    D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, &featureLevel, &m_context));

   HRESULT hr = m_device->QueryInterface(__uuidof(ID3D11Debug),
                                         reinterpret_cast<void**>(&m_debugLayer)); // Use device->QueryInterface if not using ComPtr
   if (SUCCEEDED(hr))
      {
         OutputDebugString(L"Debug layer successfully enabled.\n");
         //Microsoft::WRL::ComPtr<ID3D11InfoQueue> infoQueue;
         //if (SUCCEEDED(m_device->QueryInterface(infoQueue.GetAddressOf())))
         //   {
         //      infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
         //      infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
         //   }
      }
   else
      {
         OutputDebugString(L"Failed to enable debug layer.\n");
      }
}

void Engine::CreateBackBufferResources(uint32 width, uint32 height)
{
   SAFE_DELETE(backBufferDSV);
   SAFE_DELETE(backBufferRTV);
   m_context->Flush();
   m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

   ID3D11Texture2D* backBuffer = nullptr;
   ID3D11RenderTargetView* rtv = nullptr;
   HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer));
   HR(m_device->CreateRenderTargetView(backBuffer, nullptr, &rtv));

   backBufferDSV = new DXDepthStencilBuffer(m_device, width, height);
   backBufferRTV = new DXRenderTarget(m_device, rtv, backBufferDSV);
}

void Engine::ResizeBackbuffer(uint32 width, uint32 height)
{
   if (m_swapChain != nullptr)
      {
         CreateBackBufferResources(width, height);
      }
}

void Engine::Run(RenderSetting& _setting)
{
   static RileyTimer timer;
   float const dt = timer.MarkInSeconds();

   g_Input.Tick();
   if (true)
      {
         Update(dt);
         Render(_setting);
      }
}

void Engine::Present()
{
   if (m_swapChain != nullptr)
      {
         m_swapChain->Present(1, 0);
      }
}

ID3D11Device* Engine::GetDevice()
{
   return m_device;
}
ID3D11DeviceContext* Engine::GetDeviceContext()
{
   return m_context;
}
Window* Engine::GetWindow()
{
   return window;
}
Camera* Engine::GetCamera()
{
   return camera;
}
Renderer* Engine::GetRenderer()
{
   return renderer;
}
DXRenderTarget* Engine::GetBackbufferRTV()
{
   return backBufferRTV;
}

void Engine::Update(float dt)
{
   camera->Tick(dt);
   renderer->Tick(camera);
   renderer->SetSceneViewport(sceneViewportData);
   renderer->Update(dt);
}

void Engine::Render(RenderSetting& _setting)
{
   renderer->Render(_setting);
}

void Engine::SetSceneViewportData(std::optional<SceneViewport> viewportData)
{
   if (viewportData.has_value())
      {
         editor_active = true;
         sceneViewportData = viewportData.value();
      }
   else
      {
         editor_active = false;
         sceneViewportData.isViewportFocused = true;
         sceneViewportData.m_mousePositionX = g_Input.GetMousePositionX();
         sceneViewportData.m_mousePositionY = g_Input.GetMousePositionY();

         sceneViewportData.m_topLeftX = static_cast<float>(window->PositionX());
         sceneViewportData.m_topLeftY = static_cast<float>(window->PositionY());
         sceneViewportData.m_width = static_cast<float>(window->Width());
         sceneViewportData.m_height = static_cast<float>(window->Height());
      }
}

} // namespace Riley