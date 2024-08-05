#pragma once
#include "../Graphics/DXRenderTarget.h"
#include "../Graphics/DXDepthStencilBuffer.h"
#include "../Rendering/ModelImporter.h"
#include "../Rendering/SceneViewport.h"
#include "../Utilities/Delegate.h"
#include "../Utilities/Singleton.h"
#include "LayerStack.h"

namespace Riley
{

class Renderer;
class Camera;

struct WindowEventData;
struct EngineInit
{
   bool vsync = false;
   Window* window = nullptr;
};

class Engine
{
   friend class Editor;

   public:
   explicit Engine(EngineInit const&);
   Engine() = delete;
   Engine(Engine const&) = delete;
   Engine(Engine&&) = delete;
   Engine& operator=(Engine const&) = delete;
   Engine& operator=(Engine&&) = delete;
   ~Engine();

   void OnWindowEvent(WindowEventData const&);
   void CreateSwapChainAndDevice();
   void CreateBackBufferResources(uint32 width, uint32 height);
   void ResizeBackbuffer(uint32 width, uint32 height);

   void Run();
   void Present();

   void SetSceneViewportData(std::optional<SceneViewport> viewportData);

   ID3D11Device* GetDevice();
   ID3D11DeviceContext* GetDeviceContext();
   Window* GetWindow();
   Camera* GetCamera();
   Renderer* GetRenderer();
   DXRenderTarget* GetBackbufferRTV();

   private:
   /* Low Level APIs */
   ID3D11Device* m_device = nullptr;
   ID3D11DeviceContext* m_context = nullptr;
   IDXGISwapChain* m_swapChain = nullptr;
   ID3D11Debug* m_debugLayer;

   /* App Level APIs */
   entt::registry m_registry;

   Window* window = nullptr;
   Renderer* renderer = nullptr;
   Camera* camera = nullptr;
   ModelImporter* modelImporter = nullptr;

   DXRenderTarget* backBufferRTV = nullptr;
   DXDepthStencilBuffer* backBufferDSV = nullptr;

   bool vsync;
   bool editor_active = true;
   SceneViewport sceneViewportData;

   private:
   void InitializeScene();
   void Update(float dt);
   void Render();
};
} // namespace Riley