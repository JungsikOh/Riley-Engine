#pragma once
#include "../Core/ECS.h"
#include "../Core/Window.h"
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXConstantBuffer.h"
#include "../Graphics/DXDepthStencilBuffer.h"
#include "../Graphics/DXRenderTarget.h"
#include "Camera.h"
#include "ConstantBuffers.h"
#include "Model.h"
#include "SceneViewport.h"
#include "ShaderManager.h"
#include <memory>
#include <optional>

namespace Riley {

class DXSampler;
class DXBlendState;
class DXRasterizerState;
class DXDepthStencilState;

class Camera;
class Input;

class Renderer {
  public:
    Renderer() = default;
    Renderer(Window* window, Camera* camera, uint32 width, uint32 height);
    ~Renderer();

    void Tick(Camera*);
    void Update(float dt);

    void SetSceneViewport(const float& width = 0.0f, const float& height = 0.0f,
                          const float& minDepth = 0.0f,
                          const float& maxDepth = 1.0f,
                          const float& topLeftX = 0.0f,
                          const float& topLeftY = 0.0f);
    void SetScene(Scene const* scene);
    void Render();
    void Present(bool vsync);

    void OnResize(uint32 width, uint32 height);
    void OnLeftMouseClicked();

    Model test;

  private:
    uint32 m_width, m_height;

    /* Low Level APIs */
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swapChain;
    DXRenderTarget* m_backBufferRTV;
    DXDepthStencilBuffer* m_backBufferDepthStencil;
    Window* m_window;

    Mesh m_mesh;

    /* App Level */
    Scene* m_scene;
    Camera* m_camera;
    SceneViewport m_currentSceneViewport;
    float m_currentDeltaTime;

    DXRasterizerState* m_solidRS;
    DXDepthStencilState* m_solidDSS;

    // cbuffers
    FrameBufferConsts frameBufferCPU;
    DXConstantBuffer<FrameBufferConsts>* frameBufferGPU = nullptr;
    ObjectConsts objectConstsCPU;
    DXConstantBuffer<ObjectConsts>* objectConstsGPU = nullptr;
    MaterialConsts materialConstsCPU;
    DXConstantBuffer<MaterialConsts>* materialConstsGPU = nullptr;

  private:
    void CreateSwapChainAndDevice();
    void CreateBackBufferResources();
    void CreateBuffers();
    void CreateSamplers();
    void CreateRenderStates();

    void BindGlobals();
    void PassSolid();
};

} // namespace Riley