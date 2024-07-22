#include "Renderer.h"
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXShaderCompiler.h"
#include "../Graphics/DXShaderProgram.h"
#include "../Graphics/DXStates.h"
#include "../Math/ComputeVectors.h"
#include "../Math/MathTypes.h"
#include "ModelImporter.h"
#include "spdlog\spdlog.h"

namespace Riley
{

  constexpr uint32 SHADOW_MAP_SIZE = 2048;
  constexpr uint32 SHADOW_CUBE_SIZE = 512;

  Renderer::Renderer(Window* window, entt::registry& reg, ID3D11Device* device,
                     ID3D11DeviceContext* context, IDXGISwapChain* swapChain,
                     Camera* camera, uint32 width, uint32 height)
      : m_window(window), m_reg(reg), m_camera(camera), m_device(device),
        m_context(context), m_swapChain(swapChain), m_width(width),
        m_height(height)
  {
    ShaderManager::Initialize(m_device);
    CreateBuffers();
    CreateRenderStates();

    CreateDepthStencilBuffers(m_width, m_height);
    CreateRenderTargets(m_width, m_height);
    CreateRenderPasses(m_width, m_height);

    SetSceneViewport(m_width, m_height);

    // Logger
    spdlog::info("Renderer init complete {:f}s", timer.MarkInSeconds());
    timer.Mark();
  }

  Renderer::~Renderer()
  {
    SAFE_DELETE(frameBufferGPU);
    SAFE_DELETE(objectConstsGPU);
  }

  void Renderer::Update(float dt) { m_currentDeltaTime = dt; }

  void Renderer::Render() { PassForward(); }

  void Renderer::OnResize(uint32 width, uint32 height)
  {
    if((m_width != width || m_height != height) && width > 0 && height > 0)
      {
        m_width = width;
        m_height = height;
        CreateDepthStencilBuffers(width, height);
        CreateRenderTargets(width, height);
        CreateRenderPasses(width, height);
      }
  }

  void Renderer::Tick(Camera* camera)
  {
    BindGlobals();

    m_camera = camera;
    frameBufferCPU.globalAmbient = Vector4(0.2f, 0.3f, 0.8f, 1.0f);

    static uint32 frameIdx = 0;
    float jitterX = 0.0f, jitterY = 0.0f;

    frameBufferCPU.cameraNear = m_camera->Near();
    frameBufferCPU.cameraFar = m_camera->Far();
    frameBufferCPU.cameraJitterX = jitterX;
    frameBufferCPU.cameraJitterY = jitterY;
    frameBufferCPU.cameraPosition
      = Vector4(m_camera->Position().x, m_camera->Position().y,
                m_camera->Position().z, 1.0f);
    frameBufferCPU.view = m_camera->GetView();
    frameBufferCPU.proj = m_camera->GetProj();
    frameBufferCPU.viewProj = m_camera->GetViewProj();
    frameBufferCPU.invView = m_camera->GetView().Invert();
    frameBufferCPU.invProj = m_camera->GetProj().Invert();
    frameBufferCPU.invViewProj = m_camera->GetViewProj().Invert();
    frameBufferCPU.screenResolutionX = m_currentSceneViewport.GetWidth();
    frameBufferCPU.screenResolutionY = m_currentSceneViewport.GetHeight();

    frameBufferGPU->Update(m_context, frameBufferCPU, sizeof(frameBufferCPU));

    // Set for next frame
    frameBufferCPU.prevView = m_camera->GetView();
    frameBufferCPU.prevProj = m_camera->GetProj();
    frameBufferCPU.prevViewProj = m_camera->GetViewProj();
    ++frameIdx;
  }

  void Renderer::SetSceneViewport(const float& width, const float& height,
                                  const float& minDepth, const float& maxDepth,
                                  const float& topLeftX, const float& topLeftY)
  {
    m_currentSceneViewport.SetWidth(width);
    m_currentSceneViewport.SetHeight(height);
    m_currentSceneViewport.SetMinDepth(minDepth);
    m_currentSceneViewport.SetMaxDepth(maxDepth);
    m_currentSceneViewport.SetTopLeftX(topLeftX);
    m_currentSceneViewport.SetTopLeftY(topLeftY);
    m_currentSceneViewport.Bind(m_context);
  }

  void Renderer::SetSceneViewport(SceneViewport const& viewport)
  {
    m_currentSceneViewport = viewport;
    m_currentSceneViewport.Bind(m_context);
  }

  void Renderer::CreateBuffers()
  {
    frameBufferGPU = new DXConstantBuffer<FrameBufferConsts>(m_device, true);
    objectConstsGPU = new DXConstantBuffer<ObjectConsts>(m_device, true);
    materialConstsGPU = new DXConstantBuffer<MaterialConsts>(m_device, true);
    lightConstsGPU = new DXConstantBuffer<LightConsts>(m_device, true);
  }

  void Renderer::CreateRenderStates()
  {
    solidRS = new DXRasterizerState(m_device, CullCCWDesc());
    wireframeRS = new DXRasterizerState(m_device, WireframeDesc());
    cullNoneRS = new DXRasterizerState(m_device, CullNoneDesc());

    solidDSS = new DXDepthStencilState(m_device, DefaultDepthDesc());
    noneDepthDSS = new DXDepthStencilState(m_device, NoneDepthDesc());

    opaqueBS = new DXBlendState(m_device, OpaqueBlendStateDesc());
    additiveBS = new DXBlendState(m_device, AdditiveBlendStateDesc());
    alphaBS = new DXBlendState(m_device, AlphaBlendStateDesc());
  }

  void Renderer::CreateDepthStencilBuffers(uint32 width, uint32 height)
  {
    if(hdrDSV != nullptr) SAFE_DELETE(hdrDSV);
    hdrDSV = new DXDepthStencilBuffer(m_device, width, height, true);

    // depthMapDSV = new DXDepthStencilBuffer(m_device, width, height, false);
    // shadowDepthMapDSV =
    //     new DXDepthStencilBuffer(m_device, width, height, false);
  }

  void Renderer::CreateRenderTargets(uint32 width, uint32 height)
  {
    if(hdrRTV != nullptr) SAFE_DELETE(hdrRTV);
    hdrRTV = new DXRenderTarget(m_device, width, height,
                                DXFormat::R8G8B8A8_UNORM, hdrDSV);
    hdrRTV->CreateSRV(m_device, nullptr);
    // depthMapRTV = new DXRenderTarget(m_device, width, height,
    //                                  DXFormat::R8G8B8A8_UNORM, depthMapDSV);
    // shadowDepthMapRTV =
    //     new DXRenderTarget(m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
    //                        DXFormat::R8G8B8A8_UNORM, depthMapDSV);
  }

  void Renderer::CreateRenderPasses(uint32 width, uint32 height)
  {
    static constexpr float clearBlack[4] = {0.0f, 0.2f, 0.0f, 0.0f};

    forwardPass.attachmentRTVs.clear();
    forwardPass.attachmentRTVs.push_back(hdrRTV);
    forwardPass.clearColor = clearBlack;
    forwardPass.attachmentDSV = hdrDSV;
    forwardPass.width = width;
    forwardPass.height = height;

    //postProcessPass.attachmentRTVs.clear();
    //postProcessPass.attachmentRTVs.push_back(m_backBufferRTV);
    //postProcessPass.clearColor = clearBlack;
    //postProcessPass.attachmentDSV = m_backBufferDepthStencil;
    //postProcessPass.width = width;
    //postProcessPass.height = height;
  }

  void Renderer::BindGlobals()
  {
    static bool called = false;
    if(!called)
      {
        // VS
        frameBufferGPU->Bind(m_context, DXShaderStage::VS, 0);
        objectConstsGPU->Bind(m_context, DXShaderStage::VS, 1);

        // PS
        frameBufferGPU->Bind(m_context, DXShaderStage::PS, 0);
        materialConstsGPU->Bind(m_context, DXShaderStage::PS, 1);
        lightConstsGPU->Bind(m_context, DXShaderStage::PS, 2);

        called = true;
      }
  }

  void Renderer::PassForward()
  {
    SetSceneViewport(forwardPass.width, forwardPass.height);

    std::vector<ID3D11RenderTargetView*> rtvs;
    // for(auto& rtv : postProcessPass.attachmentRTVs)
    //   {
    //     rtv->Clear(m_context, postProcessPass.clearColor);
    //     rtvs.push_back(rtv->GetRTV());
    //   }
    // postProcessPass.attachmentDSV->Clear(m_context, 1.0f, 0);
    // m_context->OMSetRenderTargets(rtvs.size(), rtvs.data(),
    //                               postProcessPass.attachmentDSV->GetDSV());
    static constexpr float clearBlack[4] = {0.0f, 0.2f, 0.0f, 0.0f};
    hdrRTV->Clear(m_context, clearBlack);
    hdrDSV->Clear(m_context, 1.0f, 0);
    hdrRTV->BindRenderTargetView(m_context);

    solidRS->Bind(m_context);
    solidDSS->Bind(m_context, 0);

    PassSolid();
    PassForwardPhong();
  }

  void Renderer::PassSolid()
  {
    auto entity_view = m_reg.view<Mesh, Material, Transform, Light>();
    for(auto& entity : entity_view)
      {
        auto [mesh, material, transform, light]
          = entity_view.get<Mesh, Material, Transform, Light>(entity);

        ShaderManager::GetShaderProgram(material.shader)->Bind(m_context);
        objectConstsCPU.world = transform.currentTransform.Transpose();
        objectConstsCPU.worldInvTranspose
          = transform.currentTransform.Invert();
        objectConstsGPU->Update(m_context, objectConstsCPU,
                                sizeof(objectConstsCPU));

        materialConstsCPU.diffuse = material.diffuse;
        materialConstsCPU.albedoFactor = material.albedoFactor;
        materialConstsGPU->Update(m_context, materialConstsCPU,
                                  sizeof(materialConstsCPU));

        mesh.Draw(m_context);
      }
  }

  void Renderer::PassForwardPhong()
  {
    opaqueBS->Bind(m_context, nullptr);
    auto lightView = m_reg.view<Light>();
    for(auto& e : lightView)
      {
        auto lightData = lightView.get<Light>(e);
        if(!lightData.active) continue;
        // Update Cbuffer
        {
          lightConstsCPU.position = lightData.position;
          lightConstsCPU.direction = lightData.direction;
          lightConstsCPU.range = lightData.range;
          lightConstsCPU.lightColor = lightData.color;
          lightConstsCPU.type = static_cast<int32>(lightData.type);
          lightConstsCPU.innerCosine = lightData.inner_cosine;
          lightConstsCPU.outerCosine = lightData.outer_cosine;

          Matrix cameraView = m_camera->GetView();
          lightConstsCPU.position
            = Vector4::Transform(lightConstsCPU.position, cameraView);
          lightConstsCPU.direction
            = Vector4::Transform(lightConstsCPU.direction, cameraView);
          lightConstsGPU->Update(m_context, lightConstsCPU,
                                 sizeof(lightConstsCPU));

          // Render Mesh
          {
            auto entityView
              = m_reg.view<Mesh, Material, Transform>(entt::exclude<Light>);
            for(auto& entity : entityView)
              {
                auto [mesh, material, transform]
                  = entityView.get<Mesh, Material, Transform>(entity);

                ShaderManager::GetShaderProgram(material.shader)
                  ->Bind(m_context);
                objectConstsCPU.world = transform.currentTransform.Transpose();
                objectConstsCPU.worldInvTranspose
                  = transform.currentTransform.Transpose().Invert();
                objectConstsGPU->Update(m_context, objectConstsCPU,
                                        sizeof(objectConstsCPU));

                materialConstsCPU.diffuse = material.diffuse;
                materialConstsCPU.albedoFactor = material.albedoFactor;
                materialConstsCPU.ambient = Vector3(0.4f, 0.1f, 0.1f);
                materialConstsGPU->Update(m_context, materialConstsCPU,
                                          sizeof(materialConstsCPU));

                mesh.Draw(m_context);
              }
          }
        }
      }
    opaqueBS->Unbind(m_context);
  }

} // namespace Riley