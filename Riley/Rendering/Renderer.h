#pragma once
#include "../Core/Window.h"
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXConstantBuffer.h"
#include "../Graphics/DXDepthStencilBuffer.h"
#include "../Graphics/DXRenderTarget.h"
#include "../Graphics/DXRenderPass.h"
#include "Camera.h"
#include "ConstantBuffers.h"
#include "Components.h"
#include "SceneViewport.h"
#include "ShaderManager.h"
#include <memory>
#include <optional>
#include <entt.hpp>

namespace Riley
{

  class DXSampler;
  class DXBlendState;
  class DXRasterizerState;
  class DXDepthStencilState;

  class Camera;
  class Input;

  class Renderer
  {
  public:
    Renderer() = default;
    Renderer(Window* window, entt::registry& reg, ID3D11Device* device,
             ID3D11DeviceContext* context, IDXGISwapChain* swapChain,
             Camera* camera, uint32 width, uint32 height);
    ~Renderer();

    void Tick(Camera*);
    void Update(float dt);

    void
    SetSceneViewport(const float& width = 0.0f, const float& height = 0.0f,
                     const float& minDepth = 0.0f,
                     const float& maxDepth = 1.0f,
                     const float& topLeftX = 0.0f,
                     const float& topLeftY = 0.0f);

    void SetSceneViewport(SceneViewport const& viewport);

    void Render();

    void OnResize(uint32 width, uint32 height);
    void OnLeftMouseClicked();

    // Render Target Views
    std::vector<DXRenderTarget*> gbufferRTVs;
    DXRenderTarget* shadowDepthMapRTV;
    DXRenderTarget* hdrRTV;
    DXRenderTarget* postProcessRTV;

  private:
    uint32 m_width, m_height;

    /* Low Level APIs */
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swapChain;
    Window* m_window;

    /* App Level */
    entt::registry& m_reg;
    Camera* m_camera;
    SceneViewport m_currentSceneViewport;
    float m_currentDeltaTime;

    // Others
    ////////////////////////////
    DirectX::BoundingBox lightBoundingBox;

    // cbuffers
    FrameBufferConsts frameBufferCPU;
    DXConstantBuffer<FrameBufferConsts>* frameBufferGPU = nullptr;
    ObjectConsts objectConstsCPU;
    DXConstantBuffer<ObjectConsts>* objectConstsGPU = nullptr;
    MaterialConsts materialConstsCPU;
    DXConstantBuffer<MaterialConsts>* materialConstsGPU = nullptr;
    LightConsts lightConstsCPU;
    DXConstantBuffer<LightConsts>* lightConstsGPU = nullptr;
    ShadowConsts shadowConstsCPU;
    DXConstantBuffer<ShadowConsts>* shadowConstsGPU = nullptr;

    // Render States
    DXRasterizerState* solidRS;
    DXRasterizerState* wireframeRS;
    DXRasterizerState* cullNoneRS;
    DXRasterizerState* cullFrontRS;
    DXRasterizerState* depthBiasRS;

    DXDepthStencilState* solidDSS;
    DXDepthStencilState* noneDepthDSS;

    DXBlendState* opaqueBS;
    DXBlendState* additiveBS;
    DXBlendState* alphaBS;

    DXSampler* linearWrapSS;
    DXSampler* linearClampSS;
    DXSampler* linearBorderSS;
    DXSampler* shadowLinearBorderSS;

    // Depth Stencil Buffers(View)
    DXDepthStencilBuffer* hdrDSV;
    DXDepthStencilBuffer* depthMapDSV;
    DXDepthStencilBuffer* shadowDepthMapDSV;
    DXDepthStencilBuffer* shadowDepthCubeMapDSV;

    // Render Pass
    DXRenderPassDesc forwardPass;
    DXRenderPassDesc shadowMapPass;
    DXRenderPassDesc shadowCubeMapPass;
    DXRenderPassDesc postProcessPass;

  private:
    void CreateBackBufferResources();
    void CreateBuffers();
    void CreateSamplers();
    void CreateRenderStates();

    void CreateDepthStencilBuffers(uint32 width, uint32 height);
    void CreateRenderTargets(uint32 width, uint32 height);
    void CreateRenderPasses(uint32 width, uint32 height);

    void CreateGBuffer(uint32 width, uint32 height);

    void BindGlobals();

    void PassForward();
    void PassSolid();
    void PassForwardPhong();
    void PassShadowMapDirectional(Light const& light);
    void PassShadowMapSpot(Light const& light);
    void PassShadowMapPoint(Light const& light);
  };

} // namespace Riley