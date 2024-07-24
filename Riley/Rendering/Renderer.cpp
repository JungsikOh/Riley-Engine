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
    shadowConstsGPU = new DXConstantBuffer<ShadowConsts>(m_device, true);
  }

  void Renderer::CreateRenderStates()
  {
    solidRS = new DXRasterizerState(m_device, CullCCWDesc());
    wireframeRS = new DXRasterizerState(m_device, WireframeDesc());
    cullNoneRS = new DXRasterizerState(m_device, CullNoneDesc());
    cullFrontRS = new DXRasterizerState(m_device, CullCWDesc());

    solidDSS = new DXDepthStencilState(m_device, DefaultDepthDesc());
    noneDepthDSS = new DXDepthStencilState(m_device, NoneDepthDesc());

    opaqueBS = new DXBlendState(m_device, OpaqueBlendStateDesc());
    additiveBS = new DXBlendState(m_device, AdditiveBlendStateDesc());
    alphaBS = new DXBlendState(m_device, AlphaBlendStateDesc());

    linearWrapSS
      = new DXSampler(m_device, SamplerDesc(DXFilter::MIN_MAG_MIP_LINEAR,
                                            DXTextureAddressMode::Wrap));
    linearClampSS
      = new DXSampler(m_device, SamplerDesc(DXFilter::MIN_MAG_MIP_LINEAR,
                                            DXTextureAddressMode::Clamp));

    DXSamplerDesc shadowPointDesc{};
    shadowPointDesc.filter = DXFilter::MIN_MAG_MIP_POINT;
    shadowPointDesc.addressU = shadowPointDesc.addressV
      = shadowPointDesc.addressW = DXTextureAddressMode::Border;
    shadowPointDesc.borderColor[0] = 1.0f;
    linearBorderSS = new DXSampler(m_device, shadowPointDesc);

    DXSamplerDesc comparisonSamplerDesc{};
    comparisonSamplerDesc.filter = DXFilter::COMPARISON_MIN_MAG_MIP_LINEAR;
    comparisonSamplerDesc.addressU = comparisonSamplerDesc.addressV
      = comparisonSamplerDesc.addressW = DXTextureAddressMode::Border;
    comparisonSamplerDesc.borderColor[0] = comparisonSamplerDesc.borderColor[1]
      = comparisonSamplerDesc.borderColor[2]
      = comparisonSamplerDesc.borderColor[3] = 1.0f;
    comparisonSamplerDesc.comparisonFunc = DXComparisonFunc::LessEqual;
    shadowLinearBorderSS = new DXSampler(m_device, comparisonSamplerDesc);
  }

  void Renderer::CreateDepthStencilBuffers(uint32 width, uint32 height)
  {
    if(hdrDSV != nullptr) SAFE_DELETE(hdrDSV);
    hdrDSV = new DXDepthStencilBuffer(m_device, width, height, true);
    if(depthMapDSV != nullptr) SAFE_DELETE(depthMapDSV);
    depthMapDSV = new DXDepthStencilBuffer(m_device, width, height, false);
    if(shadowDepthMapDSV != nullptr) SAFE_DELETE(shadowDepthMapDSV);
    shadowDepthMapDSV = new DXDepthStencilBuffer(
      m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, false, false);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    depthMapDSV->CreateSRV(m_device, &srvDesc);
    shadowDepthMapDSV->CreateSRV(m_device, &srvDesc);
  }

  void Renderer::CreateRenderTargets(uint32 width, uint32 height)
  {
    if(hdrRTV != nullptr) SAFE_DELETE(hdrRTV);
    hdrRTV = new DXRenderTarget(m_device, width, height,
                                DXFormat::R8G8B8A8_UNORM, hdrDSV);
    hdrRTV->CreateSRV(m_device, nullptr);
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

    shadowMapPass.attachmentDSV = shadowDepthMapDSV;
    shadowMapPass.width = SHADOW_MAP_SIZE;
    shadowMapPass.height = SHADOW_MAP_SIZE;
  }

  void Renderer::BindGlobals()
  {
    static bool called = false;
    if(!called)
      {
        // VS
        frameBufferGPU->Bind(m_context, DXShaderStage::VS, 0);
        objectConstsGPU->Bind(m_context, DXShaderStage::VS, 1);
        shadowConstsGPU->Bind(m_context, DXShaderStage::VS, 3);

        // PS
        frameBufferGPU->Bind(m_context, DXShaderStage::PS, 0);
        materialConstsGPU->Bind(m_context, DXShaderStage::PS, 1);
        lightConstsGPU->Bind(m_context, DXShaderStage::PS, 2);
        shadowConstsGPU->Bind(m_context, DXShaderStage::PS, 3);

        // Samplers
        linearWrapSS->Bind(m_context, 0, DXShaderStage::PS);
        linearClampSS->Bind(m_context, 1, DXShaderStage::PS);
        linearBorderSS->Bind(m_context, 2, DXShaderStage::PS);
        shadowLinearBorderSS->Bind(m_context, 3, DXShaderStage::PS);

        called = true;
      }
  }

  void Renderer::PassForward()
  {
    SetSceneViewport(forwardPass.width, forwardPass.height);

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

    solidRS->Bind(m_context);
    solidDSS->Bind(m_context, 0);

    PassSolid();
    PassForwardPhong();
  }

  void Renderer::PassSolid()
  {
    SetSceneViewport(forwardPass.width, forwardPass.height);
    hdrRTV->BindRenderTargetView(m_context);
    auto entity_view = m_reg.view<Mesh, Material, Transform, Light>();
    for(auto& entity : entity_view)
      {
        auto [mesh, material, transform, light]
          = entity_view.get<Mesh, Material, Transform, Light>(entity);

        ShaderManager::GetShaderProgram(material.shader)->Bind(m_context);
        objectConstsCPU.world = transform.currentTransform.Transpose();
        objectConstsCPU.worldInvTranspose
          = transform.currentTransform.Invert().Transpose();
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
    // opaqueBS->Bind(m_context, nullptr);
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
          lightConstsCPU.castShadows = 1;

          Matrix cameraView = m_camera->GetView();
          lightConstsCPU.position = Vector4::Transform(lightConstsCPU.position,
                                                       cameraView.Transpose());
          lightConstsCPU.direction = Vector4::Transform(
            lightConstsCPU.direction, cameraView.Transpose());
          lightConstsGPU->Update(m_context, lightConstsCPU,
                                 sizeof(lightConstsCPU));
        }

        PassShadowMapDirectional(lightData);
        solidRS->Bind(m_context);
        // Render Mesh
        {
          float clearBlack[4] = {0.0f, 0.2f, 0.0f, 0.0f};
          SetSceneViewport(forwardPass.width, forwardPass.height);
          hdrRTV->BindRenderTargetView(m_context);
          shadowDepthMapDSV->BindSRV(m_context, 0, DXShaderStage::PS);
          auto entityView
            = m_reg.view<Mesh, Material, Transform>(entt::exclude<Light>);
          for(auto& entity : entityView)
            {
              auto [mesh, material, transform]
                = entityView.get<Mesh, Material, Transform>(entity);

              ShaderManager::GetShaderProgram(material.shader)->Bind(m_context);

              objectConstsCPU.world = transform.currentTransform.Transpose();
              objectConstsCPU.worldInvTranspose
                = transform.currentTransform.Invert().Transpose();
              objectConstsGPU->Update(m_context, objectConstsCPU,
                                      sizeof(objectConstsCPU));

              materialConstsCPU.diffuse = material.diffuse;
              materialConstsCPU.albedoFactor = material.albedoFactor;
              materialConstsCPU.ambient = Vector3(0.5f, 0.1f, 0.1f);
              materialConstsGPU->Update(m_context, materialConstsCPU,
                                        sizeof(materialConstsCPU));

              mesh.Draw(m_context);
            }
        }
      }
    // opaqueBS->Unbind(m_context);
  }

  void Renderer::PassShadowMapDirectional(Light const& light)
  {
    // assert(light.type == LightType::Directional);
    SetSceneViewport(shadowMapPass.width, shadowMapPass.height);
    shadowDepthMapDSV->Clear(m_context, 1.0f, 0);
    ID3D11RenderTargetView* rtv[1] = {0};
    m_context->OMSetRenderTargets(0, rtv, shadowDepthMapDSV->GetDSV());

    // ShadowConstantBuffer Update about Light View
    {
      Vector3 lightDir = Vector3(light.direction);
      lightDir.Normalize();
      Vector3 lightPos = Vector3(light.position);
      Vector3 targetPos = lightPos + lightDir * light.range;

      Matrix lightViewRow
        = DirectX::XMMatrixLookAtLH(lightPos, targetPos, Vector3(1.0f, 0.0f, 0.0f));
      float fovAngle = 2.0f * acos(light.outer_cosine);
      Matrix lightProjRow = DirectX::XMMatrixPerspectiveFovLH(fovAngle, 1.0f, 1.0f, light.range);

      shadowConstsCPU.lightViewProj
        = (lightViewRow * lightProjRow).Transpose();
      shadowConstsCPU.lightView = lightViewRow.Transpose();
      shadowConstsCPU.shadow_map_size = SHADOW_MAP_SIZE;
      shadowConstsCPU.shadow_matrices[0]
        = shadowConstsCPU.lightViewProj * m_camera->GetView().Invert();
      shadowConstsGPU->Update(m_context, shadowConstsCPU,
                              sizeof(shadowConstsCPU));
    }

    // Render Mesh
    {
      auto entityView
        = m_reg.view<Mesh, Material, Transform>(entt::exclude<Light>);
      for(auto& entity : entityView)
        {
          auto [mesh, material, transform]
            = entityView.get<Mesh, Material, Transform>(entity);
          cullFrontRS->Bind(m_context);
          ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthMap)
            ->Bind(m_context);

          objectConstsCPU.world = transform.currentTransform.Transpose();
          objectConstsCPU.worldInvTranspose
            = transform.currentTransform.Invert().Transpose();
          objectConstsGPU->Update(m_context, objectConstsCPU,
                                  sizeof(objectConstsCPU));

          mesh.Draw(m_context);
        }
    }
  }

} // namespace Riley