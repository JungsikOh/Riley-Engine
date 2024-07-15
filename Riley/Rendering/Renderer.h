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
    Renderer(Window* window, uint32 width, uint32 height);
    ~Renderer();

    void Tick(Camera const*);
    void Update(float dt);

    void SetSceneViewport(const float& width, const float& height,
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

    /* App Level */
    Scene* m_scene;
    Camera const* m_camera;
    SceneViewport m_currentSceneViewport;
    float m_currentDeltaTime;

    ID3D11RasterizerState* solidRS;
    ID3D11DepthStencilState* solidDSS;

    // cbuffers
    FrameBufferConsts frameBufferCPU;
    DXConstantBuffer<FrameBufferConsts>* frameBufferGPU = nullptr;
    ObjectConsts objectConstsCPU;
    DXConstantBuffer<ObjectConsts>* objectConstsGPU = nullptr;
    MaterialConsts materialConstsCPU;
    DXConstantBuffer<MaterialConsts>* materialConstsGPU = nullptr;

  private:
    void InitDirectX(); // init swapChain and backbuffer
    void CreateBuffers();
    void CreateSamplers();
    void CreateRenderStates();

    void BindGlobals();
    void PassSolid();
};

} // namespace Riley