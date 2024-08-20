#include "Renderer.h"
#include "../Editor/Editor.h"
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXScopedAnnotation.h"
#include "../Graphics/DXShaderCompiler.h"
#include "../Graphics/DXShaderProgram.h"
#include "../Graphics/DXStates.h"
#include "../Math/CalcLightFrustum.h"
#include "../Math/ComputeVectors.h"
#include "Camera.h"
#include "ModelImporter.h"
#include <random>

namespace Riley
{

Renderer::Renderer(entt::registry& reg, ID3D11Device* device, ID3D11DeviceContext* context, ID3DUserDefinedAnnotation* pAnnotation,
                   Camera* camera, uint32 width, uint32 height)
    : m_reg(reg), m_camera(camera), m_device(device), m_context(context), m_annotation(*pAnnotation), m_width(width), m_height(height)
{
    ShaderManager::Initialize(m_device);
    timer.Mark();

    CreateBuffers();
    RI_TRACE("Create Buffers {:f}s", AppTimer.ElapsedInSeconds());
    AppTimer.Mark();
    CreateOtherResources();
    CreateRenderStates();

    CreateDepthStencilBuffers(m_width, m_height);
    CreateRenderTargets(m_width, m_height);
    CreateRenderPasses(m_width, m_height);

    /*SetSceneViewport(static_cast<float>(m_width), static_cast<float>(m_height));*/

    RI_CORE_INFO("Renderer init complete {:f}s", timer.MarkInSeconds());
    timer.Mark();
}

Renderer::~Renderer()
{
    forwardPass.Destroy();
    gbufferPass.Destroy();
    shadowMapPass.Destroy();
    shadowCubeMapPass.Destroy();
    ShaderManager::Destroy();

    SAFE_DELETE(ssaoNoiseTex);

    SAFE_DELETE(frameBufferGPU);
    SAFE_DELETE(objectConstsGPU);
    SAFE_DELETE(materialConstsGPU);
    SAFE_DELETE(lightConstsGPU);
    SAFE_DELETE(shadowConstsGPU);
    SAFE_DELETE(entityIdConstsGPU);
    SAFE_DELETE(postProcessGPU);

    SAFE_DELETE(solidRS);
    SAFE_DELETE(wireframeRS);
    SAFE_DELETE(cullNoneRS);
    SAFE_DELETE(cullFrontRS);
    SAFE_DELETE(depthBiasRS);

    SAFE_DELETE(solidDSS);
    SAFE_DELETE(noneDepthDSS);

    SAFE_DELETE(opaqueBS);
    SAFE_DELETE(additiveBS);
    SAFE_DELETE(alphaBS);

    SAFE_DELETE(linearWrapSS);
    SAFE_DELETE(linearClampSS);
    SAFE_DELETE(linearBorderSS);
    SAFE_DELETE(pointWrapSS);
    SAFE_DELETE(pointClampSS);
    SAFE_DELETE(anisotropyWrapSS);
    SAFE_DELETE(shadowLinearBorderSS);

    SAFE_DELETE(hdrDSV);
    SAFE_DELETE(gbufferDSV);
    SAFE_DELETE(ambientLightingDSV);
    SAFE_DELETE(ssaoDSV);
    SAFE_DELETE(ssaoBlurDSV);
    SAFE_DELETE(pingPostprocessDSV);
    SAFE_DELETE(pongPostprocessDSV);
    SAFE_DELETE(depthMapDSV);
    SAFE_DELETE(shadowDepthMapDSV);
    SAFE_DELETE(shadowCascadeMapDSV);
    SAFE_DELETE(shadowDepthCubeMapDSV);
    SAFE_DELETE(entityIdDSV);

    SAFE_DELETE(gbufferRTV);
    SAFE_DELETE(ambientLightingRTV);
    SAFE_DELETE(ssaoRTV);
    SAFE_DELETE(ssaoBlurRTV);
    SAFE_DELETE(pingPostprocessRTV);
    SAFE_DELETE(pongPostprocessRTV);
    SAFE_DELETE(hdrRTV);
    SAFE_DELETE(entityIdRTV);
}

void Renderer::Update(float dt)
{
    m_currentDeltaTime = dt;
}

void Renderer::Render(RenderSetting& _setting)
{
    renderSetting = _setting;

    // PassForward();
    PassGBuffer();
    PassSSAO();

    PassAmbient();
    PassDeferredLighting();

    PassPostprocessing();

    PassAABB();
    PassLight();
    PassEntityID();
}

void Renderer::OnResize(uint32 width, uint32 height)
{
    if ((m_width != width || m_height != height) && width > 0 && height > 0)
    {
        m_width = width;
        m_height = height;
        CreateDepthStencilBuffers(width, height);
        CreateRenderTargets(width, height);
        CreateRenderPasses(width, height);
    }
}

void Renderer::OnLeftMouseClicked(uint32 mx, uint32 my)
{
    if (m_currentSceneViewport.isViewportFocused)
    {
        // [unused] 화면 내 영역을 통한 물체 선택 코드
        /* [unused] 화면 내 영역을 통한 물체 선택 코드
        //float cursorNdcX = m_currentSceneViewport.m_mousePositionX * 2.0f / m_currentSceneViewport.m_widthImGui - 1.0f;
        //float cursorNdcY = -m_currentSceneViewport.m_mousePositionY * 2.0f / m_currentSceneViewport.m_heightImGui + 1.0f;

        // std::cout << cursorNdcX << "," << cursorNdcY << std::endl;

        // Vector3 cursorNdcNear = Vector3(cursorNdcX, cursorNdcY, 0.0f);
        // Vector3 cursorNdcFar = Vector3(cursorNdcX, cursorNdcY, 1.0f);

        //// NDC -> World
        // Vector3 cursorNearWS = Vector3::Transform(cursorNdcNear, frameBufferCPU.invViewProj.Transpose());
        // Vector3 cursorFarWS = Vector3::Transform(cursorNdcFar, frameBufferCPU.invViewProj.Transpose());
        // Vector3 cursorDir = (cursorFarWS - cursorNearWS);
        // cursorDir.Normalize();

        // Ray cursorRay = Ray(cursorNearWS, cursorDir);
        //{
        //     auto aabbView = m_reg.view<Transform, AABB>();
        //     float dist = 0.0f;

        //    for (auto& entity : aabbView)
        //    {
        //        auto [transform, aabb] = aabbView.get<Transform, AABB>(entity);
        //        m_seleted = cursorRay.Intersects(aabb.boundingBox, dist);
        //        if (m_seleted)
        //        {
        //            if (!aabb.isDrawAABB)
        //                aabb.isDrawAABB = true;
        //            else
        //                aabb.isDrawAABB = false;
        //        }
        //    }
        //}
        */
        if (m_currentSceneViewport.m_mousePositionX < 0.0f || m_currentSceneViewport.m_mousePositionY < 0.0f ||
            m_currentSceneViewport.m_mousePositionX > m_currentSceneViewport.m_widthImGui ||
            m_currentSceneViewport.m_mousePositionY > m_currentSceneViewport.m_heightImGui)
            return;

        D3D11_TEXTURE2D_DESC stagedDesc = {
            1,                              // UINT Width;
            1,                              // UINT Height;
            1,                              // UINT MipLevels;
            1,                              // UINT ArraySize;
            DXGI_FORMAT_R32G32B32A32_FLOAT, // DXGI_FORMAT Format;
            1,
            0,                     // DXGI_SAMPLE_DESC SampleDesc;
            D3D11_USAGE_STAGING,   // D3D11_USAGE Usage;
            0,                     // UINT BindFlags;
            D3D11_CPU_ACCESS_READ, // UINT CPUAccessFlags;
            0                      // UINT MiscFlags;
        };

        ID3D11Texture2D* stagingTexture = nullptr;
        m_device->CreateTexture2D(&stagedDesc, nullptr, &stagingTexture);

        D3D11_BOX srcBox;
        srcBox.left = (UINT)(m_currentSceneViewport.m_mousePositionX / m_currentSceneViewport.m_widthImGui * m_width);
        srcBox.right = srcBox.left + 1;
        srcBox.top = (UINT)(m_currentSceneViewport.m_mousePositionY / m_currentSceneViewport.m_heightImGui * m_height);
        srcBox.bottom = srcBox.top + 1;
        srcBox.front = 0;
        srcBox.back = 1;

        std::cout << "mx : " << m_currentSceneViewport.m_mousePositionX << " my :" << srcBox.top << std::endl;

        m_context->CopySubresourceRegion(reinterpret_cast<ID3D11Resource*>(stagingTexture), 0, 0, 0, 0, entityIdRTV->GetResource(), 0,
                                         &srcBox);

        D3D11_MAPPED_SUBRESOURCE ms;
        m_context->Map(reinterpret_cast<ID3D11Resource*>(stagingTexture), 0, D3D11_MAP_READ, 0, &ms);
        auto pData = *(Vector4*)ms.pData;
        m_context->Unmap(reinterpret_cast<ID3D11Resource*>(stagingTexture), 0);

        selectedEntity = static_cast<entt::entity>(pData.x);

        SAFE_RELEASE(stagingTexture);
    }
}

void Renderer::Tick(Camera* camera)
{
    BindGlobals();

    m_camera = camera;
    frameBufferCPU.globalAmbient = m_camera->GetGlobalAmbient();

    static uint32 frameIdx = 0;
    float jitterX = 0.0f, jitterY = 0.0f;

    frameBufferCPU.cameraNear = m_camera->Near();
    frameBufferCPU.cameraFar = m_camera->Far();
    frameBufferCPU.cameraJitterX = jitterX;
    frameBufferCPU.cameraJitterY = jitterY;
    frameBufferCPU.cameraPosition = Vector4(m_camera->Position().x, m_camera->Position().y, m_camera->Position().z, 1.0f);
    frameBufferCPU.view = m_camera->GetView();
    frameBufferCPU.proj = m_camera->GetProj();
    frameBufferCPU.viewProj = m_camera->GetViewProj();
    frameBufferCPU.invView = m_camera->GetView().Invert();
    frameBufferCPU.invProj = m_camera->GetProj().Invert();
    frameBufferCPU.invViewProj = m_camera->GetViewProj().Invert();
    frameBufferCPU.screenResolutionX = m_currentSceneViewport.GetWidth();
    frameBufferCPU.screenResolutionY = m_currentSceneViewport.GetHeight();
    frameBufferCPU.cameraFrustumX = m_camera->AspectRatio() * std::tan(m_camera->Fov() * 0.5f);
    frameBufferCPU.cameraFrustumY = std::tan(m_camera->Fov() * 0.5f);

    frameBufferGPU->Update(m_context, frameBufferCPU, sizeof(frameBufferCPU));

    // Set for next frame
    frameBufferCPU.prevView = m_camera->GetView();
    frameBufferCPU.prevProj = m_camera->GetProj();
    frameBufferCPU.prevViewProj = m_camera->GetViewProj();
    ++frameIdx;
}

void Renderer::SetSceneViewport(const float& width, const float& height, const float& minDepth, const float& maxDepth,
                                const float& topLeftX, const float& topLeftY)
{
    m_currentSceneViewport.SetWidth(width);
    m_currentSceneViewport.SetHeight(height);
    m_currentSceneViewport.SetMinDepth(minDepth);
    m_currentSceneViewport.SetMaxDepth(maxDepth);
    m_currentSceneViewport.SetTopLeftX(topLeftX);
    m_currentSceneViewport.SetTopLeftY(topLeftY);
    m_currentSceneViewport.BindViewport(m_context);
}

void Renderer::SetSceneViewport(SceneViewport const& viewport)
{
    m_currentSceneViewport = viewport;
    m_currentSceneViewport.BindViewport(m_context);
}

void Renderer::CreateBuffers()
{
    frameBufferGPU = new DXConstantBuffer<FrameBufferConsts>(m_device, true);
    objectConstsGPU = new DXConstantBuffer<ObjectConsts>(m_device, true);
    materialConstsGPU = new DXConstantBuffer<MaterialConsts>(m_device, true);
    lightConstsGPU = new DXConstantBuffer<LightConsts>(m_device, true);
    shadowConstsGPU = new DXConstantBuffer<ShadowConsts>(m_device, true);
    postProcessGPU = new DXConstantBuffer<PostprocessConsts>(m_device, true);
    entityIdConstsGPU = new DXConstantBuffer<EntityIdConsts>(m_device, true);
}

void Renderer::CreateRenderStates()
{
    /// Rasterize State
    // Default RS
    solidRS = new DXRasterizerState(m_device, CullCCWDesc());
    wireframeRS = new DXRasterizerState(m_device, WireframeDesc());
    cullNoneRS = new DXRasterizerState(m_device, CullNoneDesc());
    cullFrontRS = new DXRasterizerState(m_device, CullCWDesc());

    // Depth RS
    DXRasterizerStateDesc shadow_depth_bias_state{};
    shadow_depth_bias_state.cull_mode = DXCullMode::Front;
    shadow_depth_bias_state.fill_mode = DXFillMode::Solid;
    shadow_depth_bias_state.depth_clip_enable = true;
    shadow_depth_bias_state.depth_bias = 8500;
    shadow_depth_bias_state.depth_bias_clamp = 0.0f;
    shadow_depth_bias_state.slope_scaled_depth_bias = 1.0f;
    depthBiasRS = new DXRasterizerState(m_device, shadow_depth_bias_state);

    /// DepthStencil State
    solidDSS = new DXDepthStencilState(m_device, DefaultDepthDesc());
    noneDepthDSS = new DXDepthStencilState(m_device, NoneDepthDesc());

    /// Blend State
    opaqueBS = new DXBlendState(m_device, OpaqueBlendStateDesc());
    additiveBS = new DXBlendState(m_device, AdditiveBlendStateDesc());
    alphaBS = new DXBlendState(m_device, AlphaBlendStateDesc());

    /// Sampler State
    linearWrapSS = new DXSampler(m_device, SamplerDesc(DXFilter::MIN_MAG_MIP_LINEAR, DXTextureAddressMode::Wrap));
    linearClampSS = new DXSampler(m_device, SamplerDesc(DXFilter::MIN_MAG_MIP_LINEAR, DXTextureAddressMode::Clamp));
    pointWrapSS = new DXSampler(m_device, SamplerDesc(DXFilter::MIN_MAG_MIP_POINT, DXTextureAddressMode::Wrap));
    pointClampSS = new DXSampler(m_device, SamplerDesc(DXFilter::MIN_MAG_MIP_POINT, DXTextureAddressMode::Clamp));
    anisotropyWrapSS = new DXSampler(m_device, SamplerDesc(DXFilter::ANISOTROPIC, DXTextureAddressMode::Wrap));

    DXSamplerDesc shadowPointDesc{};
    shadowPointDesc.filter = DXFilter::MIN_MAG_MIP_LINEAR;
    shadowPointDesc.addressU = shadowPointDesc.addressV = shadowPointDesc.addressW = DXTextureAddressMode::Border;
    shadowPointDesc.borderColor[0] = 1.0f;
    shadowPointDesc.borderColor[1] = 1.0f;
    shadowPointDesc.borderColor[2] = 1.0f;
    shadowPointDesc.borderColor[3] = 1.0f;
    linearBorderSS = new DXSampler(m_device, shadowPointDesc);

    DXSamplerDesc comparisonSamplerDesc{};
    comparisonSamplerDesc.filter = DXFilter::COMPARISON_MIN_MAG_MIP_LINEAR;
    comparisonSamplerDesc.addressU = comparisonSamplerDesc.addressV = comparisonSamplerDesc.addressW = DXTextureAddressMode::Border;
    comparisonSamplerDesc.borderColor[0] = comparisonSamplerDesc.borderColor[1] = comparisonSamplerDesc.borderColor[2] =
        comparisonSamplerDesc.borderColor[3] = 1.0f;
    comparisonSamplerDesc.comparisonFunc = DXComparisonFunc::LessEqual;
    shadowLinearBorderSS = new DXSampler(m_device, comparisonSamplerDesc);
}

void Renderer::CreateDepthStencilBuffers(uint32 width, uint32 height)
{
    /// DXDepthStencilBuffer Initialize
    SAFE_DELETE(hdrDSV);
    hdrDSV = new DXDepthStencilBuffer(m_device, width, height, true);

    SAFE_DELETE(gbufferDSV);
    gbufferDSV = new DXDepthStencilBuffer(m_device, width, height);
    D3D11_SHADER_RESOURCE_VIEW_DESC gbufferSRVDesc;
    ZeroMemory(&gbufferSRVDesc, sizeof(gbufferSRVDesc));
    gbufferSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    gbufferSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    gbufferSRVDesc.Texture2D.MipLevels = 1;
    gbufferDSV->CreateSRV(m_device, &gbufferSRVDesc);

    SAFE_DELETE(ambientLightingDSV);
    ambientLightingDSV = new DXDepthStencilBuffer(m_device, width, height);

    SAFE_DELETE(depthMapDSV);
    depthMapDSV = new DXDepthStencilBuffer(m_device, width, height, false);
    D3D11_SHADER_RESOURCE_VIEW_DESC defaultDepthMapDesc;
    ZeroMemory(&defaultDepthMapDesc, sizeof(defaultDepthMapDesc));
    defaultDepthMapDesc.Format = DXGI_FORMAT_R32_FLOAT;
    defaultDepthMapDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    defaultDepthMapDesc.Texture2D.MipLevels = 1;
    depthMapDSV->CreateSRV(m_device, &defaultDepthMapDesc);

    SAFE_DELETE(entityIdDSV);
    entityIdDSV = new DXDepthStencilBuffer(m_device, width, height, false);

    // AO
    SAFE_DELETE(ssaoDSV);
    ssaoDSV = new DXDepthStencilBuffer(m_device, width, height);
    SAFE_DELETE(ssaoBlurDSV);
    ssaoBlurDSV = new DXDepthStencilBuffer(m_device, width, height);

    // SHADOW MAP
    {
        SAFE_DELETE(shadowDepthMapDSV);
        shadowDepthMapDSV = new DXDepthStencilBuffer(m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, false, false);
        shadowDepthMapDSV->CreateSRV(m_device, &defaultDepthMapDesc);

        SAFE_DELETE(shadowCascadeMapDSV);
        shadowCascadeMapDSV = new DXDepthStencilBuffer();
        D3D11_TEXTURE2D_DESC textureArrayDesc{};
        ZeroMemory(&textureArrayDesc, sizeof(textureArrayDesc));
        textureArrayDesc.Width = SHADOW_CASCADE_SIZE;
        textureArrayDesc.Height = SHADOW_CASCADE_SIZE;
        textureArrayDesc.MipLevels = 1;
        textureArrayDesc.ArraySize = CASCADE_COUNT;
        textureArrayDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        textureArrayDesc.SampleDesc.Count = 1;
        textureArrayDesc.SampleDesc.Quality = 0;
        textureArrayDesc.Usage = D3D11_USAGE_DEFAULT;
        textureArrayDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        textureArrayDesc.CPUAccessFlags = 0;
        textureArrayDesc.MiscFlags = 0;
        D3D11_DEPTH_STENCIL_VIEW_DESC cascadeDsvDesc{};
        ZeroMemory(&cascadeDsvDesc, sizeof(cascadeDsvDesc));
        cascadeDsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        cascadeDsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        cascadeDsvDesc.Texture2DArray.ArraySize = CASCADE_COUNT;
        cascadeDsvDesc.Texture2DArray.FirstArraySlice = 0;
        cascadeDsvDesc.Texture2DArray.MipSlice = 0;
        D3D11_SHADER_RESOURCE_VIEW_DESC texArraySrvDesc{};
        ZeroMemory(&texArraySrvDesc, sizeof(texArraySrvDesc));
        texArraySrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        texArraySrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        texArraySrvDesc.Texture2DArray.ArraySize = CASCADE_COUNT;
        texArraySrvDesc.Texture2DArray.MipLevels = 1;
        shadowCascadeMapDSV->Initialize(m_device, SHADOW_CASCADE_SIZE, SHADOW_CASCADE_SIZE, false, &textureArrayDesc, &cascadeDsvDesc);
        shadowCascadeMapDSV->CreateSRV(m_device, &texArraySrvDesc);

        SAFE_DELETE(shadowDepthCubeMapDSV);
        shadowDepthCubeMapDSV = new DXDepthStencilBuffer(m_device, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, false, true);
        D3D11_SHADER_RESOURCE_VIEW_DESC shadowCubeMapDesc;
        ZeroMemory(&shadowCubeMapDesc, sizeof(shadowCubeMapDesc));
        shadowCubeMapDesc.Format = DXGI_FORMAT_R32_FLOAT;
        shadowCubeMapDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        shadowCubeMapDesc.TextureCube.MostDetailedMip = 0;
        shadowCubeMapDesc.TextureCube.MipLevels = 1;
        shadowDepthCubeMapDSV->CreateSRV(m_device, &shadowCubeMapDesc);
    }

    // Postprocess
    {
        SAFE_DELETE(pingPostprocessDSV);
        SAFE_DELETE(pongPostprocessDSV);
        pingPostprocessDSV = new DXDepthStencilBuffer(m_device, width, height);
        pongPostprocessDSV = new DXDepthStencilBuffer(m_device, width, height);
    }
}

void Renderer::CreateRenderTargets(uint32 width, uint32 height)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC tex2dSRVDesc;
    ZeroMemory(&tex2dSRVDesc, sizeof(tex2dSRVDesc));
    tex2dSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex2dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    tex2dSRVDesc.Texture2D.MipLevels = 1;

    SAFE_DELETE(hdrRTV);
    hdrRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, hdrDSV);
    hdrRTV->CreateSRV(m_device, nullptr);

    // GBuffer Pass
    {
        SAFE_DELETE(gbufferRTV);
        gbufferRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, gbufferDSV);
        gbufferRTV->CreateSRV(m_device, &tex2dSRVDesc);
        gbufferRTV->DestoryResource();
        gbufferRTV->CreateRenderTarget(m_device);
        gbufferRTV->CreateSRV(m_device, &tex2dSRVDesc);
        gbufferRTV->DestoryResource();
        gbufferRTV->CreateRenderTarget(m_device);
        gbufferRTV->CreateSRV(m_device, &tex2dSRVDesc);
    }

    // DeferredLighting
    {
        SAFE_DELETE(ambientLightingRTV);
        ambientLightingRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, ambientLightingDSV);
        ambientLightingRTV->CreateSRV(m_device, &tex2dSRVDesc);
    }

    // SSAO
    {
        SAFE_DELETE(ssaoRTV);
        ssaoRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, ssaoDSV);
        ssaoRTV->CreateSRV(m_device, nullptr);
        ssaoRTV->CreateUAV(m_device, nullptr);

        SAFE_DELETE(ssaoBlurRTV);
        ssaoBlurRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, ssaoBlurDSV);
        ssaoBlurRTV->CreateSRV(m_device, &tex2dSRVDesc);
        ssaoBlurRTV->CreateUAV(m_device, nullptr);
    }

    // Postprocess
    {
        SAFE_DELETE(pingPostprocessRTV);
        SAFE_DELETE(pongPostprocessRTV);
        pingPostprocessRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, pingPostprocessDSV);
        pongPostprocessRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, pongPostprocessDSV);
        pingPostprocessRTV->CreateSRV(m_device, &tex2dSRVDesc);
        pongPostprocessRTV->CreateSRV(m_device, &tex2dSRVDesc);
    }

    // Entity ID
    {
        SAFE_DELETE(entityIdRTV);
        entityIdRTV = new DXRenderTarget(m_device, width, height, DXFormat::R32G32B32A32_FLOAT, entityIdDSV);
    }
}

void Renderer::CreateOtherResources()
{
    // AO random
    std::vector<float> randomTextureData;
    randomTextureData.reserve(AO_NOISE_DIM * AO_NOISE_DIM * 4);
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;
    for (uint32 i = 0; i < SSAO_KERNEL_SIZE; ++i)
    {
        Vector4 offset(randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, randomFloats(generator), 1.0f);
        offset.Normalize();
        offset *= randomFloats(generator);
        float scale = static_cast<float>(i) / ssaoKernel.size();
        scale = std::lerp(0.1f, 1.0f, scale * scale);
        offset *= scale;
        ssaoKernel[i] = offset;
    }
    for (int32 i = 0; i < AO_NOISE_DIM * AO_NOISE_DIM; ++i)
    {
        randomTextureData.push_back(randomFloats(generator));
        randomTextureData.push_back(randomFloats(generator));
        randomTextureData.push_back(0.0f);
        randomTextureData.push_back(1.0f);
    }

    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = randomTextureData.data();
    initData.SysMemPitch = AO_NOISE_DIM * 4 * sizeof(float);
    ssaoNoiseTex = new DXResource();
    ssaoNoiseTex->Initialize(m_device, AO_NOISE_DIM, AO_NOISE_DIM, DXFormat::R32G32B32A32_FLOAT, &initData);
    D3D11_SHADER_RESOURCE_VIEW_DESC tex2dSRVDesc;
    ZeroMemory(&tex2dSRVDesc, sizeof(tex2dSRVDesc));
    tex2dSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    tex2dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    tex2dSRVDesc.Texture2D.MipLevels = 1;
    ssaoNoiseTex->CreateSRV(m_device, &tex2dSRVDesc);
}

void Renderer::CreateRenderPasses(uint32 width, uint32 height)
{
    static constexpr float clearBlack[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    // Forward
    {
        forwardPass.attachmentRTVs = hdrRTV;
        forwardPass.attachmentDSVs = hdrDSV;
        forwardPass.attachmentRS = solidRS;
        forwardPass.attachmentDSS = solidDSS;
        forwardPass.clearColor = clearBlack;
        forwardPass.width = width;
        forwardPass.height = height;
    }

    // GBuffer
    {
        gbufferPass.attachmentRTVs = gbufferRTV;
        gbufferPass.attachmentDSVs = gbufferDSV;
        gbufferPass.attachmentRS = solidRS;
        gbufferPass.attachmentDSS = solidDSS;
        gbufferPass.clearColor = clearBlack;
        gbufferPass.width = width;
        gbufferPass.height = height;
    }

    // Deffered Lighting
    {
        deferredLightingPass.attachmentRTVs = ambientLightingRTV;
        deferredLightingPass.attachmentDSVs = ambientLightingDSV;
        deferredLightingPass.attachmentRS = solidRS;
        deferredLightingPass.attachmentDSS = solidDSS;
        deferredLightingPass.clearColor = clearBlack;
        deferredLightingPass.width = width;
        deferredLightingPass.height = height;
    }

    // AO PASSES
    {
        ssaoPass.attachmentRTVs = ssaoRTV;
        ssaoPass.attachmentDSVs = ssaoDSV;
        ssaoPass.attachmentRS = solidRS;
        ssaoPass.attachmentDSS = noneDepthDSS;
        ssaoPass.clearColor = clearBlack;
        ssaoPass.width = width;
        ssaoPass.height = height;
    }

    // SHADOW MAP PASSES
    {
        shadowMapPass.attachmentDSVs = shadowDepthMapDSV;
        shadowMapPass.attachmentRS = depthBiasRS;
        shadowMapPass.attachmentDSS = solidDSS;
        shadowMapPass.width = SHADOW_MAP_SIZE;
        shadowMapPass.height = SHADOW_MAP_SIZE;

        shadowCascadeMapPass.attachmentDSVs = shadowCascadeMapDSV;
        shadowCascadeMapPass.attachmentRS = depthBiasRS;
        shadowCascadeMapPass.attachmentDSS = solidDSS;
        shadowCascadeMapPass.width = SHADOW_CASCADE_SIZE;
        shadowCascadeMapPass.height = SHADOW_CASCADE_SIZE;

        shadowCubeMapPass.attachmentDSVs = shadowDepthCubeMapDSV;
        shadowCubeMapPass.attachmentRS = depthBiasRS;
        shadowCubeMapPass.attachmentDSS = solidDSS;
        shadowCubeMapPass.width = SHADOW_CUBE_SIZE;
        shadowCubeMapPass.height = SHADOW_CUBE_SIZE;
    }

    // POSTPROCESS PASSES
    {
        postprocessPasses[0].attachmentRTVs = pingPostprocessRTV;
        postprocessPasses[0].attachmentDSVs = pingPostprocessDSV;
        postprocessPasses[0].width = width;
        postprocessPasses[0].height = height;
        postprocessPasses[0].attachmentRS = solidRS;
        postprocessPasses[0].attachmentDSS = noneDepthDSS;
        postprocessPasses[0].clearColor = clearBlack;

        postprocessPasses[1].attachmentRTVs = pongPostprocessRTV;
        postprocessPasses[1].attachmentDSVs = pongPostprocessDSV;
        postprocessPasses[1].width = width;
        postprocessPasses[1].height = height;
        postprocessPasses[1].attachmentRS = solidRS;
        postprocessPasses[1].attachmentDSS = noneDepthDSS;
        postprocessPasses[1].clearColor = clearBlack;
    }
}

void Renderer::BindGlobals()
{
    static bool called = false;
    if (!called)
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
        postProcessGPU->Bind(m_context, DXShaderStage::PS, 4);

        // GS
        shadowConstsGPU->Bind(m_context, DXShaderStage::GS, 3);

        // Samplers
        linearWrapSS->Bind(m_context, 0, DXShaderStage::PS);
        linearClampSS->Bind(m_context, 1, DXShaderStage::PS);
        linearBorderSS->Bind(m_context, 2, DXShaderStage::PS);
        pointWrapSS->Bind(m_context, 3, DXShaderStage::PS);
        pointClampSS->Bind(m_context, 4, DXShaderStage::PS);
        anisotropyWrapSS->Bind(m_context, 5, DXShaderStage::PS);
        shadowLinearBorderSS->Bind(m_context, 6, DXShaderStage::PS);

        called = true;
    }
}

void Renderer::LightFrustumCulling(const Light& light)
{
    auto visibiltiyView = m_reg.view<AABB>();
    for (auto& e : visibiltiyView)
    {
        if (m_reg.try_get<Light>(e))
            continue;

        auto& aabb = visibiltiyView.get<AABB>(e);

        switch (light.type)
        {
        case LightType::Directional:
            aabb.isLightVisible = aabb.isLightVisible ? true : lightBoundingBox.Intersects(aabb.boundingBox);
            break;
        case LightType::Point:
        case LightType::Spot:
            aabb.isLightVisible = aabb.isLightVisible ? true : lightBoundingFrustum.Intersects(aabb.boundingBox);
            break;
        default:
            assert(false && "LightFrustumCulling Error!");
        }
    }
}

void Renderer::PassForward()
{
    forwardPass.BeginRenderPass(m_context);
    PassForwardPhong();
    forwardPass.EndRenderPass(m_context);
}

void Renderer::PassForwardPhong()
{
    auto lightView = m_reg.view<Light>();
    for (auto& e : lightView)
    {
        auto lightData = lightView.get<Light>(e);
        if (!lightData.active)
            continue;

        // Update Cbuffer
        {
            lightConstsCPU.position = lightData.position;
            lightConstsCPU.direction = lightData.direction;
            lightConstsCPU.range = lightData.range;
            lightConstsCPU.lightColor = lightData.color * lightData.energy;
            lightConstsCPU.type = static_cast<int32>(lightData.type);
            lightConstsCPU.innerCosine = lightData.inner_cosine;
            lightConstsCPU.outerCosine = lightData.outer_cosine;
            lightConstsCPU.castShadows = 1;

            Matrix cameraView = m_camera->GetView();
            lightConstsCPU.position = Vector4::Transform(lightConstsCPU.position, cameraView.Transpose());
            lightConstsCPU.direction = Vector4::Transform(lightConstsCPU.direction, cameraView.Transpose());
            lightConstsGPU->Update(m_context, lightConstsCPU, sizeof(lightConstsCPU));
        }

        if (lightData.type == LightType::Spot)
        {
            PassShadowMapSpot(lightData);
        }
        else if (lightData.type == LightType::Directional)
        {
            PassShadowMapDirectional(lightData);
        }
        else if (lightData.type == LightType::Point)
        {
            PassShadowMapPoint(lightData);
        }

        additiveBS->Bind(m_context);
        SetSceneViewport(static_cast<float>(forwardPass.width), static_cast<float>(forwardPass.height));
        forwardPass.BeginRenderPass(m_context, false, false);

        // Render Mesh
        {
            shadowMapPass.attachmentDSVs->BindSRV(m_context, 0, DXShaderStage::PS);
            shadowCubeMapPass.attachmentDSVs->BindSRV(m_context, 1, DXShaderStage::PS);

            auto entityView = m_reg.view<Mesh, Material, Transform>(entt::exclude<Light>);
            for (auto& entity : entityView)
            {
                auto [mesh, material, transform] = entityView.get<Mesh, Material, Transform>(entity);

                ShaderManager::GetShaderProgram(material.shader)->Bind(m_context);

                objectConstsCPU.world = transform.currentTransform.Transpose();
                objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
                objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

                materialConstsCPU.diffuse = material.diffuse;
                materialConstsCPU.albedoFactor = material.albedoFactor;
                materialConstsCPU.ambient = material.diffuse;
                materialConstsGPU->Update(m_context, materialConstsCPU, sizeof(materialConstsCPU));

                mesh.Draw(m_context);
            }

            shadowMapPass.attachmentDSVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
            shadowCubeMapPass.attachmentDSVs->UnbindSRV(m_context, 1, DXShaderStage::PS);
        }
    }
    additiveBS->Unbind(m_context);
}

void Renderer::PassGBuffer()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "GBuffer Pass");
    SetSceneViewport(static_cast<float>(gbufferPass.width), static_cast<float>(gbufferPass.height));
    gbufferPass.BeginRenderPass(m_context);

    // Mesh Render
    int draw = 0, total = 0;
    auto entityView = m_reg.view<Mesh, Material, Transform, AABB>();
    for (auto& entity : entityView)
    {
        auto [mesh, material, transform, aabb] = entityView.get<Mesh, Material, Transform, AABB>(entity);

        Matrix parent = Matrix::Identity;
        if (auto root = m_reg.try_get<Relationship>(entity))
        {
            if (auto rootTransform = m_reg.try_get<Transform>(root->parent))
                parent = rootTransform->currentTransform;
        }
        aabb.boundingBox = aabb.orginalBox;
        aabb.boundingBox.Transform(aabb.boundingBox, transform.currentTransform * parent);
        aabb.UpdateBuffer(m_device);

        if (m_camera->Frustum().Contains(aabb.boundingBox)) // m_camera->Frustum().Contains(aabb.boundingBox)
        {
            if (material.albedoTexture != INVALID_TEXTURE_HANDLE)
                g_TextureManager.GetTextureView(material.albedoTexture)->BindSRV(m_context, 0, DXShaderStage::PS);
            if (material.normalTexture != INVALID_TEXTURE_HANDLE)
                g_TextureManager.GetTextureView(material.normalTexture)->BindSRV(m_context, 1, DXShaderStage::PS);
            if (material.metallicRoughnessTexture != INVALID_TEXTURE_HANDLE)
                g_TextureManager.GetTextureView(material.metallicRoughnessTexture)->BindSRV(m_context, 2, DXShaderStage::PS);
            if (material.emissiveTexture != INVALID_TEXTURE_HANDLE)
                g_TextureManager.GetTextureView(material.emissiveTexture)->BindSRV(m_context, 3, DXShaderStage::PS);

            draw++;
            ShaderManager::GetShaderProgram(ShaderProgram::GBuffer)->Bind(m_context);

            objectConstsCPU.world = (transform.currentTransform * parent).Transpose();
            objectConstsCPU.worldInvTranspose = objectConstsCPU.world.Invert();
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            materialConstsCPU.diffuse = material.diffuse;
            materialConstsCPU.albedoFactor = material.albedoFactor;
            materialConstsCPU.useNormalMap = material.useNormalMap;
            materialConstsCPU.metallicFactor = material.metallicFactor;
            materialConstsCPU.roughnessFactor = 0.3f;
            materialConstsCPU.emissiveFactor = material.emissiveFactor;
            materialConstsCPU.ambient = material.diffuse;
            materialConstsGPU->Update(m_context, materialConstsCPU, sizeof(materialConstsCPU));

            mesh.Draw(m_context);

            if (material.albedoTexture != INVALID_TEXTURE_HANDLE)
                g_TextureManager.GetTextureView(material.albedoTexture)->UnbindSRV(m_context, 0, DXShaderStage::PS);
            if (material.normalTexture != INVALID_TEXTURE_HANDLE)
                g_TextureManager.GetTextureView(material.normalTexture)->UnbindSRV(m_context, 1, DXShaderStage::PS);
            if (material.metallicRoughnessTexture != INVALID_TEXTURE_HANDLE)
                g_TextureManager.GetTextureView(material.metallicRoughnessTexture)->UnbindSRV(m_context, 2, DXShaderStage::PS);
            if (material.emissiveTexture != INVALID_TEXTURE_HANDLE)
                g_TextureManager.GetTextureView(material.emissiveTexture)->UnbindSRV(m_context, 3, DXShaderStage::PS);
            ShaderManager::GetShaderProgram(ShaderProgram::GBuffer)->Unbind(m_context);
        }
        total++;
    }
    // std::cout << draw << "," << total << std::endl;
    gbufferPass.EndRenderPass(m_context);
}

void Renderer::PassAmbient()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "AmbientEmissive Pass");
    SetSceneViewport(static_cast<float>(deferredLightingPass.width), static_cast<float>(deferredLightingPass.height));
    deferredLightingPass.BeginRenderPass(m_context);

    // Mesh Render
    {
        gbufferPass.attachmentRTVs->BindSRV(m_context, 0, DXShaderStage::PS);
        ssaoRTV->BindSRV(m_context, 3, DXShaderStage::PS);

        ShaderManager::GetShaderProgram(ShaderProgram::Ambient)->Bind(m_context);

        m_context->IASetInputLayout(nullptr);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        m_context->Draw(4, 0);

        gbufferPass.attachmentRTVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
        ssaoRTV->UnbindSRV(m_context, 3, DXShaderStage::PS);
    }
    deferredLightingPass.EndRenderPass(m_context);
}

void Renderer::PassDeferredLighting()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "DeferredLighting Pass");
    auto lightView = m_reg.view<Light>();
    for (auto& e : lightView)
    {
        auto lightData = lightView.get<Light>(e);
        if (!lightData.active)
            continue;

        // Update Cbuffer
        {
            lightConstsCPU.position = lightData.position;
            lightConstsCPU.direction = lightData.direction;
            lightConstsCPU.range = lightData.range;
            lightConstsCPU.lightColor = lightData.color * lightData.energy;
            lightConstsCPU.type = static_cast<int32>(lightData.type);
            lightConstsCPU.innerCosine = lightData.inner_cosine;
            lightConstsCPU.outerCosine = lightData.outer_cosine;
            lightConstsCPU.castShadows = lightData.castShadows;
            lightConstsCPU.useCascades = lightData.useCascades;
            lightConstsCPU.radius = lightData.radius;
            lightConstsCPU.haloStrength = lightData.haloStrength;

            Matrix cameraView = m_camera->GetView();
            lightConstsCPU.position = Vector4::Transform(lightConstsCPU.position, cameraView.Transpose());
            lightConstsCPU.direction = Vector4::Transform(lightConstsCPU.direction, cameraView.Transpose());
            lightConstsGPU->Update(m_context, lightConstsCPU, sizeof(lightConstsCPU));
        }

        if (lightData.type == LightType::Spot)
        {
            PassShadowMapSpot(lightData);
        }
        else if (lightData.type == LightType::Directional)
        {
            lightData.useCascades ? PassShadowMapCascade(lightData) : PassShadowMapDirectional(lightData);
        }
        else if (lightData.type == LightType::Point)
        {
            PassShadowMapPoint(lightData);
        }

        additiveBS->Bind(m_context);
        SetSceneViewport(static_cast<float>(deferredLightingPass.width), static_cast<float>(deferredLightingPass.height));
        deferredLightingPass.BeginRenderPass(m_context, false, false, 0);

        // Render Mesh
        {
            gbufferPass.attachmentRTVs->BindSRV(m_context, 0, DXShaderStage::PS);
            gbufferPass.attachmentDSVs->BindSRV(m_context, 3, DXShaderStage::PS);
            shadowMapPass.attachmentDSVs->BindSRV(m_context, 4, DXShaderStage::PS);
            shadowCubeMapPass.attachmentDSVs->BindSRV(m_context, 5, DXShaderStage::PS);
            shadowCascadeMapPass.attachmentDSVs->BindSRV(m_context, 6, DXShaderStage::PS);

            ShaderManager::GetShaderProgram(ShaderProgram::DeferredLighting)->Bind(m_context);

            m_context->IASetInputLayout(nullptr);
            m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            m_context->Draw(4, 0);

            gbufferPass.attachmentRTVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
            gbufferPass.attachmentDSVs->UnbindSRV(m_context, 3, DXShaderStage::PS);
            shadowMapPass.attachmentDSVs->UnbindSRV(m_context, 4, DXShaderStage::PS);
            shadowCubeMapPass.attachmentDSVs->UnbindSRV(m_context, 5, DXShaderStage::PS);
            shadowCascadeMapPass.attachmentDSVs->UnbindSRV(m_context, 6, DXShaderStage::PS);
        }
        deferredLightingPass.EndRenderPass(m_context);
        // Halo Pass
        PassHalo();
    }
    additiveBS->Unbind(m_context);
}

void Renderer::PassHalo()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "Halo Pass");

    SetSceneViewport(static_cast<float>(deferredLightingPass.width), static_cast<float>(deferredLightingPass.height));
    deferredLightingPass.BeginRenderPass(m_context, false, false, 0);
    additiveBS->Bind(m_context);
    {
        gbufferPass.attachmentDSVs->BindSRV(m_context, 0, DXShaderStage::PS);

        ShaderManager::GetShaderProgram(ShaderProgram::Halo)->Bind(m_context);

        m_context->IASetInputLayout(nullptr);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        m_context->Draw(4, 0);

        gbufferPass.attachmentDSVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
    }
    deferredLightingPass.EndRenderPass(m_context);
    additiveBS->Unbind(m_context);
}

void Renderer::PassPostprocessing()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "Postprocessing Pass");

    if (renderSetting.ssr)
    {
        postprocessPasses[postprocessIndex].BeginRenderPass(m_context);
        PassSSR();
        postprocessPasses[postprocessIndex].EndRenderPass(m_context);

        postprocessIndex = !postprocessIndex;
    }
}

void Renderer::PassSSAO()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "SSAO Pass");
    // 1. SSAO
    SetSceneViewport(static_cast<float>(ssaoPass.width), static_cast<float>(ssaoPass.height));
    ssaoPass.BeginRenderPass(m_context);

    // Render Mesh
    {
        gbufferPass.attachmentRTVs->BindSRV(m_context, 0, DXShaderStage::PS);
        gbufferPass.attachmentDSVs->BindSRV(m_context, 3, DXShaderStage::PS);
        ssaoNoiseTex->BindSRV(m_context, 4, DXShaderStage::PS);

        ShaderManager::GetShaderProgram(ShaderProgram::SSAO)->Bind(m_context);

        for (uint32 i = 0; i < ssaoKernel.size(); ++i)
            postProcessCPU.samples[i] = ssaoKernel[i];
        postProcessCPU.AO = static_cast<int32>(renderSetting.ao);
        postProcessCPU.noiseScale = Vector2(m_width / 8.0f, m_height / 8.0f);
        postProcessCPU.ssaoPower = renderSetting.ssaoPower;
        postProcessCPU.ssaoRadius = renderSetting.ssaoRadius;
        postProcessGPU->Update(m_context, postProcessCPU, sizeof(postProcessCPU));

        m_context->IASetInputLayout(nullptr);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        m_context->Draw(4, 0);

        gbufferPass.attachmentRTVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
        gbufferPass.attachmentDSVs->UnbindSRV(m_context, 3, DXShaderStage::PS);
        ssaoNoiseTex->UnbindSRV(m_context, 4, DXShaderStage::PS);
    }

    // 2. SSAO Blur
    {
        // static float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        // ssaoBlurDSV->Clear(m_context, 1.0f, 0);
        // ssaoBlurRTV->Clear(m_context, clearColor);
        // ssaoBlurRTV->BindRenderTargets(m_context);

        // ssaoPass.attachmentRTVs->BindSRV(m_context, 0, DXShaderStage::PS);

        // ShaderManager::GetShaderProgram(ShaderProgram::SSAOBlur)->Bind(m_context);

        // m_context->IASetInputLayout(nullptr);
        // m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        // m_context->Draw(4, 0);

        // ssaoPass.attachmentRTVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
    }
    ssaoPass.EndRenderPass(m_context);

    ShaderManager::GetShaderProgram(ShaderProgram::BlurX)->Bind(m_context);
    ssaoRTV->BindSRV(m_context, 0, DXShaderStage::CS);
    ssaoBlurRTV->BindUAV(m_context, 0);
    m_context->Dispatch((uint32)std::ceil(ssaoPass.width / 1024.0f), ssaoPass.height, 1);
    ssaoRTV->UnbindSRV(m_context, 0, DXShaderStage::CS);
    ssaoBlurRTV->UnbindUAV(m_context, 0);

    ShaderManager::GetShaderProgram(ShaderProgram::BlurY)->Bind(m_context);
    ssaoBlurRTV->BindSRV(m_context, 0, DXShaderStage::CS);
    ssaoRTV->BindUAV(m_context, 0);
    m_context->Dispatch(ssaoPass.width, (uint32)std::ceil(ssaoPass.height / 1024.0f), 1);
    ssaoBlurRTV->UnbindSRV(m_context, 0, DXShaderStage::CS);
    ssaoRTV->UnbindUAV(m_context, 0);
}

void Renderer::PassSSR()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "SSR Pass");

    {
        gbufferPass.attachmentRTVs->BindSRV(m_context, 0, DXShaderStage::PS);
        gbufferPass.attachmentDSVs->BindSRV(m_context, 3, DXShaderStage::PS);
        deferredLightingPass.attachmentRTVs->BindSRV(m_context, 4, DXShaderStage::PS);

        ShaderManager::GetShaderProgram(ShaderProgram::SSR)->Bind(m_context);

        // cbuffer update
        postProcessCPU.ssrRayStep = renderSetting.ssrRayStep;
        postProcessCPU.ssrThickness = renderSetting.ssrThickness;
        postProcessGPU->Update(m_context, postProcessCPU, sizeof(postProcessCPU));

        m_context->IASetInputLayout(nullptr);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        m_context->Draw(4, 0);

        gbufferPass.attachmentRTVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
        gbufferPass.attachmentDSVs->UnbindSRV(m_context, 3, DXShaderStage::PS);
        deferredLightingPass.attachmentRTVs->UnbindSRV(m_context, 4, DXShaderStage::PS);

        ShaderManager::GetShaderProgram(ShaderProgram::SSR)->Unbind(m_context);
    }
}

void Renderer::PassShadowMapDirectional(const Light& light)
{
    assert(light.type == LightType::Directional);
    RILEY_SCOPED_ANNOTATION(m_annotation, "Directional Shadow Map Pass");
    // ShadowConstantBuffer Update about Light View
    {
        const auto& [V, P] = LightFrustum::DirectionalLightViewProjection(light, m_camera, lightBoundingBox);

        shadowConstsCPU.lightViewProj = (V * P).Transpose();
        shadowConstsCPU.lightView = V.Transpose();
        shadowConstsCPU.shadowMapSize = SHADOW_MAP_SIZE;
        shadowConstsCPU.shadowMatrices[0] = shadowConstsCPU.lightViewProj * m_camera->GetView().Invert();
        shadowConstsGPU->Update(m_context, shadowConstsCPU, sizeof(shadowConstsCPU));
    }

    SetSceneViewport(static_cast<float>(shadowMapPass.width), static_cast<float>(shadowMapPass.height));
    shadowMapPass.BeginRenderPass(m_context);
    LightFrustumCulling(light);

    auto entityView = m_reg.view<Mesh, Material, Transform, AABB>(entt::exclude<Light>);
    for (auto& e : entityView)
    {
        auto [mesh, material, transform, aabb] = entityView.get<Mesh, Material, Transform, AABB>(e);
        if (aabb.isLightVisible)
        {
            ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthMap)->Bind(m_context);

            Matrix parent = Matrix::Identity;
            if (auto root = m_reg.try_get<Relationship>(e))
            {
                if (auto rootTransform = m_reg.try_get<Transform>(root->parent))
                    parent = rootTransform->currentTransform;
            }

            objectConstsCPU.world = (transform.currentTransform * parent).Transpose();
            objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert();
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            mesh.Draw(m_context);
            aabb.isLightVisible = false;
        }
    }
    ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthMap)->Unbind(m_context);
    shadowMapPass.EndRenderPass(m_context);
}

void Renderer::PassShadowMapCascade(const Light& light)
{
    assert(light.type == LightType::Directional);
    RILEY_SCOPED_ANNOTATION(m_annotation, "Directional Shadow Cascade Map Pass");
    // Light Frustum Part
    std::array<float, CASCADE_COUNT> splitDistances{};
    std::array<Matrix, CASCADE_COUNT> cascadeFrustumProjRow =
        LightFrustum::RecalcProjectionMatrices(m_camera, SPLIT_LAMBDA, splitDistances);

    for (uint32 i = 0; i < CASCADE_COUNT; ++i)
    {
        const auto& [V, P] =
            LightFrustum::CascadeDirectionalLightViewProjection(light, m_camera, cascadeFrustumProjRow[i], lightBoundingBox);

        shadowConstsCPU.shadowCascadeMapViewProj[i] = (V * P).Transpose();
        shadowConstsCPU.shadowMapSize = SHADOW_CASCADE_SIZE;
        shadowConstsCPU.shadowMatrices[i] = shadowConstsCPU.shadowCascadeMapViewProj[i] * m_camera->GetView().Invert();
        shadowConstsCPU.splits[i] = splitDistances[i];
        LightFrustumCulling(light);
    }
    shadowConstsGPU->Update(m_context, shadowConstsCPU, sizeof(shadowConstsCPU));

    // Mesh Render Part
    SetSceneViewport(static_cast<float>(shadowCascadeMapPass.width), static_cast<float>(shadowCascadeMapPass.height));
    shadowCascadeMapPass.BeginRenderPass(m_context);

    auto entitiesView = m_reg.view<Mesh, Transform, AABB>(entt::exclude<Light>);
    for (auto& e : entitiesView)
    {
        auto [mesh, transform, aabb] = entitiesView.get<Mesh, Transform, AABB>(e);
        if (aabb.isLightVisible)
        {

            ShaderManager::GetShaderProgram(ShaderProgram::ShadowCascadeMap)->Bind(m_context);

            Matrix parent = Matrix::Identity;
            if (auto root = m_reg.try_get<Relationship>(e))
            {
                if (auto rootTransform = m_reg.try_get<Transform>(root->parent))
                    parent = rootTransform->currentTransform;
            }

            objectConstsCPU.world = (transform.currentTransform * parent).Transpose();
            objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            mesh.Draw(m_context);
            aabb.isLightVisible = false;
        }
    }
    ShaderManager::GetShaderProgram(ShaderProgram::ShadowCascadeMap)->Unbind(m_context);
    shadowCascadeMapPass.EndRenderPass(m_context);
}

void Renderer::PassShadowMapSpot(const Light& light)
{
    assert(light.type == LightType::Spot);
    RILEY_SCOPED_ANNOTATION(m_annotation, "Spot Shadow Map Pass");

    // ShadowConstantBuffer Update about Light View
    {
        Vector3 lightDir = Vector3(light.direction);
        lightDir.Normalize();
        Vector3 lightPos = Vector3(light.position);
        Vector3 targetPos = lightPos + lightDir * light.range;
        Vector3 up = Vector3::Up;
        if (abs(up.Dot(lightDir) + 1.0f) < 1e-5)
            up = Vector3(1.0f, 0.0f, 0.0f);

        Matrix lightViewRow = DirectX::XMMatrixLookAtLH(lightPos, targetPos, up);
        float fovAngle = 2.0f * acos(light.outer_cosine);
        Matrix lightProjRow = DirectX::XMMatrixPerspectiveFovLH(fovAngle, 1.0f, 0.5f, light.range);

        shadowConstsCPU.lightViewProj = (lightViewRow * lightProjRow).Transpose();
        shadowConstsCPU.lightView = lightViewRow.Transpose();
        shadowConstsCPU.shadowMapSize = SHADOW_MAP_SIZE;
        shadowConstsCPU.shadowMatrices[0] = shadowConstsCPU.lightViewProj * m_camera->GetView().Invert();
        shadowConstsGPU->Update(m_context, shadowConstsCPU, sizeof(shadowConstsCPU));

        lightBoundingFrustum = BoundingFrustum(lightProjRow);
        lightBoundingFrustum.Transform(lightBoundingFrustum, lightViewRow.Invert());
    }

    SetSceneViewport(static_cast<float>(shadowMapPass.width), static_cast<float>(shadowMapPass.height));
    shadowMapPass.BeginRenderPass(m_context);
    LightFrustumCulling(light);

    // Render Mesh
    auto entityView = m_reg.view<Mesh, Material, Transform, AABB>(entt::exclude<Light>);
    for (auto& e : entityView)
    {
        auto [mesh, material, transform, aabb] = entityView.get<Mesh, Material, Transform, AABB>(e);
        if (aabb.isLightVisible)
        {

            ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthMap)->Bind(m_context);

            Matrix parent = Matrix::Identity;
            if (auto root = m_reg.try_get<Relationship>(e))
            {
                if (auto rootTransform = m_reg.try_get<Transform>(root->parent))
                    parent = rootTransform->currentTransform;
            }

            objectConstsCPU.world = (transform.currentTransform * parent).Transpose();
            objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            mesh.Draw(m_context);
            aabb.isLightVisible = false;
        }
    }
    ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthMap)->Unbind(m_context);
    shadowMapPass.EndRenderPass(m_context);
}

void Renderer::PassShadowMapPoint(const Light& light)
{
    assert(light.type == LightType::Point);
    RILEY_SCOPED_ANNOTATION(m_annotation, "Point Shadow Cube Map Pass");

    {
        Matrix lightViewRow = Matrix::Identity;
        Matrix lightProjRow = Matrix::Identity;

        Vector3 directions[6] = {{1.0f, 0.0f, 0.0f},  {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                                 {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, -1.0f}};
        Vector3 up[6] = {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
                         {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};

        const float fov = 2.0f * acos(light.outer_cosine);
        lightProjRow = DirectX::XMMatrixPerspectiveFovLH(fov, 1.0f, 0.5f, light.range);

        for (uint32 face = 0; face < 6; ++face)
        {
            lightViewRow = DirectX::XMMatrixLookAtLH(light.position, light.position + directions[face] * light.range, up[face]);
            shadowConstsCPU.shadowCubeMapViewProj[face] = (lightViewRow * lightProjRow).Transpose();

            lightBoundingFrustumCube[face] = BoundingFrustum(lightProjRow);
            lightBoundingFrustumCube[face].Transform(lightBoundingFrustumCube[face], lightViewRow.Invert());
            lightBoundingFrustum = BoundingFrustum(lightProjRow);
            lightBoundingFrustum.Transform(lightBoundingFrustum, lightViewRow.Invert());
            LightFrustumCulling(light);
        }
        shadowConstsCPU.shadowMapSize = SHADOW_CUBE_SIZE;
        shadowConstsGPU->Update(m_context, shadowConstsCPU, sizeof(shadowConstsCPU));
    }

    SetSceneViewport(static_cast<float>(shadowCubeMapPass.width), static_cast<float>(shadowCubeMapPass.height));
    shadowCubeMapPass.BeginRenderPass(m_context);

    // Render Mesh
    auto entityView = m_reg.view<Mesh, Material, Transform, AABB>(entt::exclude<Light>);
    for (auto& e : entityView)
    {
        auto [mesh, material, transform, aabb] = entityView.get<Mesh, Material, Transform, AABB>(e);

        if (aabb.isLightVisible)
        {
            ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthCubeMap)->Bind(m_context);

            Matrix parent = Matrix::Identity;
            if (auto root = m_reg.try_get<Relationship>(e))
            {
                if (auto rootTransform = m_reg.try_get<Transform>(root->parent))
                    parent = rootTransform->currentTransform;
            }

            objectConstsCPU.world = (transform.currentTransform * parent).Transpose();
            objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            mesh.Draw(m_context);
            aabb.isLightVisible = false;
        }
    }
    ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthCubeMap)->Unbind(m_context);
    shadowCubeMapPass.EndRenderPass(m_context);
}

void Renderer::PassAABB()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "AABB Pass");

    SetSceneViewport(static_cast<float>(deferredLightingPass.width), static_cast<float>(deferredLightingPass.height));
    postprocessPasses[0].BeginRenderPass(m_context, false, false);
    wireframeRS->Bind(m_context);
    noneDepthDSS->Bind(m_context, 0);
    // Mesh Render
    {
        auto aabb = m_reg.try_get<AABB>(selectedEntity);

        if (aabb)
        {
            ShaderManager::GetShaderProgram(ShaderProgram::Solid)->Bind(m_context);

            aabb->SetAABBIndexBuffer(m_device);

            objectConstsCPU.world = Matrix::Identity;
            objectConstsCPU.worldInvTranspose = Matrix::Identity;
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            materialConstsCPU.diffuse = Vector3(0.0f, 1.0f, 0.0f);
            materialConstsGPU->Update(m_context, materialConstsCPU, sizeof(materialConstsCPU));

            m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
            BindVertexBuffer(m_context, aabb->aabbVertexBuffer.get());
            BindIndexBuffer(m_context, aabb->aabbIndexBuffer.get());
            m_context->DrawIndexed(aabb->aabbIndexBuffer->GetCount(), 0, 0);
        }
    }
    postprocessPasses[0].EndRenderPass(m_context);
}

void Renderer::PassLight()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "Solid Light Pass");

    SetSceneViewport(static_cast<float>(deferredLightingPass.width), static_cast<float>(deferredLightingPass.height));
    postprocessPasses[0].BeginRenderPass(m_context, false, false);
    noneDepthDSS->Bind(m_context, 0);
    // Mesh Render
    {
        auto lightView = m_reg.view<Mesh, Material, Transform, Light>();
        for (auto& entity : lightView)
        {
            auto [mesh, material, transform, light] = lightView.get<Mesh, Material, Transform, Light>(entity);

            ShaderManager::GetShaderProgram(ShaderProgram::Solid)->Bind(m_context);

            objectConstsCPU.world = transform.currentTransform.Transpose();
            objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            materialConstsCPU.diffuse = Vector3(light.color);
            materialConstsGPU->Update(m_context, materialConstsCPU, sizeof(materialConstsCPU));

            mesh.Draw(m_context);
        }
    }
    deferredLightingPass.EndRenderPass(m_context);
}

void Renderer::PassEntityID()
{
    RILEY_SCOPED_ANNOTATION(m_annotation, "Entity ID Pass");

    SetSceneViewport(static_cast<float>(m_width), static_cast<float>(m_height));
    static float color[4] = {(float)uint32(-1), 0.0f, 0.0f, 0.0f};
    entityIdDSV->Clear(m_context, 1.0f, 0);
    entityIdRTV->Clear(m_context, color);
    entityIdRTV->BindRenderTargets(m_context);
    solidRS->Bind(m_context);
    solidDSS->Bind(m_context, 0);

    entityIdConstsGPU->Bind(m_context, DXShaderStage::PS, 10);

    auto entityView = m_reg.view<Mesh, Transform, AABB>();
    for (auto& e : entityView)
    {
        auto [mesh, transform] = entityView.get<Mesh, Transform>(e);
        ShaderManager::GetShaderProgram(ShaderProgram::Picking)->Bind(m_context);

        objectConstsCPU.world = transform.currentTransform.Transpose();
        objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
        objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

        entityIdConstsCPU.entityID = entt::to_entity(e);
        entityIdConstsGPU->Update(m_context, entityIdConstsCPU, sizeof(entityIdConstsCPU));

        mesh.Draw(m_context);
    }
}

} // namespace Riley