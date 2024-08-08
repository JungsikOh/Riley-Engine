#include "Renderer.h"
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXShaderCompiler.h"
#include "../Graphics/DXShaderProgram.h"
#include "../Graphics/DXStates.h"
#include "../Math/ComputeVectors.h"
#include "Camera.h"
#include "ModelImporter.h"
#include <random>

namespace Riley
{
constexpr uint32 SHADOW_MAP_SIZE = 2048;
constexpr uint32 SHADOW_CUBE_SIZE = 512;

namespace
{
using namespace DirectX;

std::pair<Matrix, Matrix> DirectionalLightViewProjection(Light const& light, Camera* camera, BoundingBox& cullBox)
{
    // [1] Camera view frustum
    BoundingFrustum frustum = camera->Frustum();
    std::array<Vector3, BoundingFrustum::CORNER_COUNT> corners = {};
    frustum.GetCorners(corners.data());

    //
    BoundingSphere frustumSphere;
    BoundingSphere::CreateFromFrustum(frustumSphere, frustum);

    // 
    Vector3 frustumCenter(0.0f, 0.0f, 0.0f);
    for (uint32 i = 0; i < corners.size(); ++i)
    {
        frustumCenter = frustumCenter + corners[i];
    }
    frustumCenter /= static_cast<float>(corners.size());
    // 
    float radius = 0.0f;
    for (Vector3 const& corner : corners)
    {
        float distance = Vector3::Distance(corner, frustumCenter);
        radius = std::max(radius, distance);
    }
    radius = std::ceil(radius * 8.0f) / 8.0f; // ±×¸²ÀÚ ¸Ê ÇØ»óµµ¿¡ ¸ÂÃã.

    const Vector3 max_extents(radius, radius, radius);
    const Vector3 min_extents = -max_extents;
    const Vector3 cascade_extents = max_extents - min_extents;

    Vector4 lightDir = light.direction;
    lightDir.Normalize();
    Vector3 up = Vector3::Up;

     if (abs(up.Dot(Vector3(lightDir)) + 1.0f) < 1e-5)
         up = Vector3(1.0f, 0.0f, 0.0f);

    Matrix lightViewRow = XMMatrixLookAtLH(frustumCenter, frustumCenter + 1.0f * lightDir * radius, up);

    float l = min_extents.x;
    float b = min_extents.y;
    float n = min_extents.z;
    float r = max_extents.x;
    float t = max_extents.y;
    float f = max_extents.z * 1.5f; // far´Â Ãß°¡ÀûÀÎ ¿©À¯¸¦ ÁÖ¾î ¼³Á¤

    Matrix lightProjRow = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
    // Matrix lightProjRow = XMMatrixPerspectiveFovLH(fovAngle, 1.0f, 0.05f, f);
    Matrix lightViewProjRow = lightViewRow * lightProjRow;

    // viewport °æ°è¸¦ Á¤ÀÇÇÏ´Â bounding box »ý¼º
    //BoundingBox::CreateFromPoints(cullBox, Vector4(l, b, n, 1.0f), Vector4(r, t, f, 1.0f));
    //cullBox.Transform(cullBox, lightViewRow.Invert()); // Camera View Space -> world Space
    return {lightViewRow, lightProjRow};
}
} // namespace

Renderer::Renderer(entt::registry& reg, ID3D11Device* device, ID3D11DeviceContext* context, Camera* camera, uint32 width,
                   uint32 height)
    : m_reg(reg), m_camera(camera), m_device(device), m_context(context), m_width(width), m_height(height)
{
    timer.Mark();
    ShaderManager::Initialize(m_device);

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
    SAFE_DELETE(entityIDConstsGPU);
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
    SAFE_DELETE(shadowLinearBorderSS);

    SAFE_DELETE(hdrDSV);
    SAFE_DELETE(gbufferDSV);
    SAFE_DELETE(ambientLightingDSV);
    SAFE_DELETE(ssaoDSV);
    SAFE_DELETE(ssaoBlurDSV);
    SAFE_DELETE(depthMapDSV);
    SAFE_DELETE(shadowDepthMapDSV);
    SAFE_DELETE(shadowDepthCubeMapDSV);

    SAFE_DELETE(gbufferRTV);
    SAFE_DELETE(ambientLightingRTV);
    SAFE_DELETE(ssaoRTV);
    SAFE_DELETE(ssaoBlurRTV);
    SAFE_DELETE(postProcessRTV);
    SAFE_DELETE(hdrRTV);
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
    PassAABB();
    PassLight();
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
        float cursorNdcX = m_currentSceneViewport.m_mousePositionX * 2.0f / m_currentSceneViewport.m_widthImGui - 1.0f;
        float cursorNdcY = -m_currentSceneViewport.m_mousePositionY * 2.0f / m_currentSceneViewport.m_heightImGui + 1.0f;

        std::cout << cursorNdcX << "," << cursorNdcY << std::endl;

        Vector3 cursorNdcNear = Vector3(cursorNdcX, cursorNdcY, 0.0f);
        Vector3 cursorNdcFar = Vector3(cursorNdcX, cursorNdcY, 1.0f);

        //// NDC -> World
        Vector3 cursorNearWS = Vector3::Transform(cursorNdcNear, frameBufferCPU.invViewProj.Transpose());
        Vector3 cursorFarWS = Vector3::Transform(cursorNdcFar, frameBufferCPU.invViewProj.Transpose());
        Vector3 cursorDir = (cursorFarWS - cursorNearWS);
        cursorDir.Normalize();

        Ray cursorRay = Ray(cursorNearWS, cursorDir);
        {
            auto aabbView = m_reg.view<Transform, AABB>();
            float dist = 0.0f;

            for (auto& entity : aabbView)
            {
                auto [transform, aabb] = aabbView.get<Transform, AABB>(entity);
                m_seleted = cursorRay.Intersects(aabb.boundingBox, dist);
                if (m_seleted)
                {
                    if (!aabb.isDrawAABB)
                        aabb.isDrawAABB = true;
                    else
                        aabb.isDrawAABB = false;
                    return;
                }
            }
        }
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
    frameBufferCPU.cameraPosition = Vector4(m_camera->Position().x, m_camera->Position().y, m_camera->Position().z, 1.0f);
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
    entityIDConstsGPU = new DXConstantBuffer<EntityIDConsts>(m_device, true);
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

    DXSamplerDesc pointWrapDesc{};
    pointWrapDesc.filter = DXFilter::MIN_MAG_MIP_POINT;
    pointWrapDesc.addressU = pointWrapDesc.addressV = pointWrapDesc.addressW = DXTextureAddressMode::Wrap;
    pointWrapDesc.borderColor[0] = 1.0f;
    pointWrapSS = new DXSampler(m_device, pointWrapDesc);

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
    SAFE_DELETE(ambientLightingDSV);
    ambientLightingDSV = new DXDepthStencilBuffer(m_device, width, height);
    SAFE_DELETE(ssaoDSV);
    ssaoDSV = new DXDepthStencilBuffer(m_device, width, height);
    SAFE_DELETE(ssaoBlurDSV);
    ssaoBlurDSV = new DXDepthStencilBuffer(m_device, width, height);
    SAFE_DELETE(depthMapDSV);
    depthMapDSV = new DXDepthStencilBuffer(m_device, width, height, false);
    SAFE_DELETE(shadowDepthMapDSV);
    shadowDepthMapDSV = new DXDepthStencilBuffer(m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, false, false);
    SAFE_DELETE(shadowDepthCubeMapDSV);
    shadowDepthCubeMapDSV = new DXDepthStencilBuffer(m_device, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, false, true);

    // SRV about Texture2D DSV
    D3D11_SHADER_RESOURCE_VIEW_DESC gbufferSRVDesc;
    ZeroMemory(&gbufferSRVDesc, sizeof(gbufferSRVDesc));
    gbufferSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    gbufferSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    gbufferSRVDesc.Texture2D.MipLevels = 1;
    gbufferDSV->CreateSRV(m_device, &gbufferSRVDesc);

    D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(shadowMapDesc));
    shadowMapDesc.Format = DXGI_FORMAT_R32_FLOAT;
    shadowMapDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shadowMapDesc.Texture2D.MipLevels = 1;
    depthMapDSV->CreateSRV(m_device, &shadowMapDesc);
    shadowDepthMapDSV->CreateSRV(m_device, &shadowMapDesc);

    // SRV about TextureCube DSV
    D3D11_SHADER_RESOURCE_VIEW_DESC shadowCubeMapDesc;
    ZeroMemory(&shadowCubeMapDesc, sizeof(shadowCubeMapDesc));
    shadowCubeMapDesc.Format = DXGI_FORMAT_R32_FLOAT;
    shadowCubeMapDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    shadowCubeMapDesc.TextureCube.MostDetailedMip = 0;
    shadowCubeMapDesc.TextureCube.MipLevels = 1;
    shadowDepthCubeMapDSV->CreateSRV(m_device, &shadowCubeMapDesc);
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
    SAFE_DELETE(gbufferRTV);
    gbufferRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, gbufferDSV);
    gbufferRTV->CreateSRV(m_device, &tex2dSRVDesc);
    gbufferRTV->CreateRenderTarget(m_device);
    gbufferRTV->CreateSRV(m_device, &tex2dSRVDesc);
    gbufferRTV->CreateRenderTarget(m_device);
    gbufferRTV->CreateSRV(m_device, &tex2dSRVDesc);

    // DeferredLighting
    SAFE_DELETE(ambientLightingRTV);
    ambientLightingRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, ambientLightingDSV);
    ambientLightingRTV->CreateSRV(m_device, &tex2dSRVDesc);

    SAFE_DELETE(ssaoRTV);
    ssaoRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, ssaoDSV);
    ssaoRTV->CreateSRV(m_device, &tex2dSRVDesc);

    SAFE_DELETE(ssaoBlurRTV);
    ssaoBlurRTV = new DXRenderTarget(m_device, width, height, DXFormat::R8G8B8A8_UNORM, ssaoBlurDSV);
    ssaoBlurRTV->CreateSRV(m_device, &tex2dSRVDesc);
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
        Vector4 offset(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator), 0.0f);
        offset.Normalize();
        offset *= randomFloats(generator);
        float scale = static_cast<float>(i) / ssaoKernel.size();
        offset *= scale;
        ssaoKernel[i] = offset;
    }
    for (uint32 i = 0; i < AO_NOISE_DIM * AO_NOISE_DIM; ++i)
    {
        randomTextureData.push_back(randomFloats(generator) * 2.0f - 1.0f);
        randomTextureData.push_back(randomFloats(generator) * 2.0f - 1.0f);
        randomTextureData.push_back(0.0f);
        randomTextureData.push_back(1.0f);
    }

    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = (void*)randomTextureData.data();
    initData.SysMemPitch = AO_NOISE_DIM * 4 * sizeof(float);
    ssaoNoiseTex = new DXResource();
    ssaoNoiseTex->Initialize(m_device, AO_NOISE_DIM, AO_NOISE_DIM, DXFormat::R32G32B32A32_FLOAT, &initData);
    ssaoNoiseTex->CreateSRV(m_device, nullptr);
}

void Renderer::CreateRenderPasses(uint32 width, uint32 height)
{
    static constexpr float clearBlack[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    forwardPass.attachmentRTVs = hdrRTV;
    forwardPass.attachmentDSVs = hdrDSV;
    forwardPass.attachmentRS = solidRS;
    forwardPass.attachmentDSS = solidDSS;
    forwardPass.clearColor = clearBlack;
    forwardPass.width = width;
    forwardPass.height = height;

    gbufferPass.attachmentRTVs = gbufferRTV;
    gbufferPass.attachmentDSVs = gbufferDSV;
    gbufferPass.attachmentRS = solidRS;
    gbufferPass.attachmentDSS = solidDSS;
    gbufferPass.clearColor = clearBlack;
    gbufferPass.width = width;
    gbufferPass.height = height;

    deferredLightingPass.attachmentRTVs = ambientLightingRTV;
    deferredLightingPass.attachmentDSVs = ambientLightingDSV;
    deferredLightingPass.attachmentRS = solidRS;
    deferredLightingPass.attachmentDSS = solidDSS;
    deferredLightingPass.clearColor = clearBlack;
    deferredLightingPass.width = width;
    deferredLightingPass.height = height;

    ssaoPass.attachmentRTVs = ssaoRTV;
    ssaoPass.attachmentDSVs = ssaoDSV;
    ssaoPass.attachmentRS = solidRS;
    ssaoPass.attachmentDSS = solidDSS;
    ssaoPass.clearColor = clearBlack;
    ssaoPass.width = width;
    ssaoPass.height = height;

    shadowMapPass.attachmentDSVs = shadowDepthMapDSV;
    shadowMapPass.attachmentRS = depthBiasRS;
    shadowMapPass.attachmentDSS = solidDSS;
    shadowMapPass.width = SHADOW_MAP_SIZE;
    shadowMapPass.height = SHADOW_MAP_SIZE;

    shadowCubeMapPass.attachmentDSVs = shadowDepthCubeMapDSV;
    shadowCubeMapPass.attachmentRS = depthBiasRS;
    shadowCubeMapPass.attachmentDSS = solidDSS;
    shadowCubeMapPass.width = SHADOW_CUBE_SIZE;
    shadowCubeMapPass.height = SHADOW_CUBE_SIZE;
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
        shadowLinearBorderSS->Bind(m_context, 4, DXShaderStage::PS);

        called = true;
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
    SetSceneViewport(static_cast<float>(gbufferPass.width), static_cast<float>(gbufferPass.height));
    gbufferPass.BeginRenderPass(m_context);

    // Mesh Render
    {
        int draw = 0, total = 0;
        auto entityView = m_reg.view<Mesh, Material, Transform, AABB>();
        for (auto& entity : entityView)
        {
            auto [mesh, material, transform, aabb] = entityView.get<Mesh, Material, Transform, AABB>(entity);

            if (m_camera->Frustum().Contains(aabb.boundingBox)) // m_camera->Frustum().Contains(aabb.boundingBox)
            {
                draw++;
                ShaderManager::GetShaderProgram(ShaderProgram::GBuffer)->Bind(m_context);

                objectConstsCPU.world = transform.currentTransform.Transpose();
                objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
                objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

                materialConstsCPU.diffuse = material.diffuse;
                materialConstsCPU.albedoFactor = material.albedoFactor;
                materialConstsCPU.metallicFactor = 0.5f;
                materialConstsCPU.roughnessFactor = 0.3f;
                materialConstsCPU.emissiveFactor = material.emissiveFactor;
                materialConstsCPU.ambient = material.diffuse;
                materialConstsGPU->Update(m_context, materialConstsCPU, sizeof(materialConstsCPU));

                mesh.Draw(m_context);
            }
            total++;
        }
        std::cout << draw << "," << total << std::endl;
    }
    gbufferPass.EndRenderPass(m_context);
}

void Renderer::PassAmbient()
{
    SetSceneViewport(static_cast<float>(deferredLightingPass.width), static_cast<float>(deferredLightingPass.height));
    deferredLightingPass.BeginRenderPass(m_context);

    // Mesh Render
    {
        gbufferPass.attachmentRTVs->BindSRV(m_context, 0, DXShaderStage::PS);
        ssaoBlurRTV->BindSRV(m_context, 3, DXShaderStage::PS);

        ShaderManager::GetShaderProgram(ShaderProgram::Ambient)->Bind(m_context);

        m_context->IASetInputLayout(nullptr);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        m_context->Draw(4, 0);

        gbufferPass.attachmentRTVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
        ssaoBlurRTV->UnbindSRV(m_context, 3, DXShaderStage::PS);
    }
    deferredLightingPass.EndRenderPass(m_context);
}

void Renderer::PassDeferredLighting()
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
        SetSceneViewport(static_cast<float>(deferredLightingPass.width), static_cast<float>(deferredLightingPass.height));
        deferredLightingPass.BeginRenderPass(m_context, false, false);

        // Render Mesh
        {
            gbufferPass.attachmentRTVs->BindSRV(m_context, 0, DXShaderStage::PS);
            gbufferPass.attachmentDSVs->BindSRV(m_context, 3, DXShaderStage::PS);
            shadowMapPass.attachmentDSVs->BindSRV(m_context, 4, DXShaderStage::PS);
            shadowCubeMapPass.attachmentDSVs->BindSRV(m_context, 5, DXShaderStage::PS);

            ShaderManager::GetShaderProgram(ShaderProgram::DeferredLighting)->Bind(m_context);

            m_context->IASetInputLayout(nullptr);
            m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            m_context->Draw(4, 0);

            gbufferPass.attachmentRTVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
            gbufferPass.attachmentDSVs->UnbindSRV(m_context, 3, DXShaderStage::PS);
            shadowMapPass.attachmentDSVs->UnbindSRV(m_context, 4, DXShaderStage::PS);
            shadowCubeMapPass.attachmentDSVs->UnbindSRV(m_context, 5, DXShaderStage::PS);
        }
    }
    additiveBS->Unbind(m_context);
}

void Renderer::PassSSAO()
{
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
    ssaoPass.EndRenderPass(m_context);

    // 2. SSAO Blur
    {
        static float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        ssaoBlurDSV->Clear(m_context, 1.0f, 0);
        ssaoBlurRTV->Clear(m_context, clearColor);
        ssaoBlurRTV->BindRenderTargets(m_context);

        ssaoPass.attachmentRTVs->BindSRV(m_context, 0, DXShaderStage::PS);

        ShaderManager::GetShaderProgram(ShaderProgram::SSAOBlur)->Bind(m_context);

        m_context->IASetInputLayout(nullptr);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        m_context->Draw(4, 0);

        ssaoPass.attachmentRTVs->UnbindSRV(m_context, 0, DXShaderStage::PS);
    }
}

void Renderer::PassShadowMapDirectional(Light const& light)
{
    assert(light.type == LightType::Directional);

    // ShadowConstantBuffer Update about Light View
    {
        auto const& [V, P] = DirectionalLightViewProjection(light, m_camera, lightBoundingBox);

        shadowConstsCPU.lightViewProj = (V * P).Transpose();
        shadowConstsCPU.lightView = V.Transpose();
        shadowConstsCPU.shadow_map_size = SHADOW_MAP_SIZE;
        shadowConstsCPU.shadow_matrices[0] = shadowConstsCPU.lightViewProj * m_camera->GetView().Invert();
        shadowConstsGPU->Update(m_context, shadowConstsCPU, sizeof(shadowConstsCPU));
    }

    SetSceneViewport(static_cast<float>(shadowMapPass.width), static_cast<float>(shadowMapPass.height));
    shadowMapPass.BeginRenderPass(m_context);

    {
        auto entityView = m_reg.view<Mesh, Material, Transform>(entt::exclude<Light>);
        for (auto& entity : entityView)
        {
            auto [mesh, material, transform] = entityView.get<Mesh, Material, Transform>(entity);
            ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthMap)->Bind(m_context);

            objectConstsCPU.world = transform.currentTransform.Transpose();
            objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            mesh.Draw(m_context);
        }
        ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthMap)->Unbind(m_context);
    }
    shadowMapPass.EndRenderPass(m_context);
}

void Renderer::PassShadowMapSpot(Light const& light)
{
    assert(light.type == LightType::Spot);

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
        Matrix lightProjRow = DirectX::XMMatrixPerspectiveFovLH(fovAngle, 1.0f, 1.0f, light.range);

        shadowConstsCPU.lightViewProj = (lightViewRow * lightProjRow).Transpose();
        shadowConstsCPU.lightView = lightViewRow.Transpose();
        shadowConstsCPU.shadow_map_size = SHADOW_MAP_SIZE;
        shadowConstsCPU.shadow_matrices[0] = shadowConstsCPU.lightViewProj * m_camera->GetView().Invert();
        shadowConstsGPU->Update(m_context, shadowConstsCPU, sizeof(shadowConstsCPU));
    }

    SetSceneViewport(static_cast<float>(shadowMapPass.width), static_cast<float>(shadowMapPass.height));
    shadowMapPass.BeginRenderPass(m_context);

    // Render Mesh
    {
        auto entityView = m_reg.view<Mesh, Material, Transform>(entt::exclude<Light>);
        for (auto& entity : entityView)
        {
            auto [mesh, material, transform] = entityView.get<Mesh, Material, Transform>(entity);
            ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthMap)->Bind(m_context);

            objectConstsCPU.world = transform.currentTransform.Transpose();
            objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            mesh.Draw(m_context);
        }
        ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthMap)->Unbind(m_context);
    }
    shadowMapPass.EndRenderPass(m_context);
}

void Renderer::PassShadowMapPoint(Light const& light)
{
    assert(light.type == LightType::Point);

    {
        Matrix lightViewRow = Matrix::Identity;
        Matrix lightProjRow = Matrix::Identity;

        Vector3 directions[6] = {{1.0f, 0.0f, 0.0f},  {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                                 {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, -1.0f}};
        Vector3 up[6] = {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f},
                         {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}};

        float fovAngle = 2.0f * acos(light.outer_cosine);
        lightProjRow = DirectX::XMMatrixPerspectiveFovLH(fovAngle, 1.0f, 0.5f, light.range);

        for (uint32 face = 0; face < 6; ++face)
        {
            lightViewRow = DirectX::XMMatrixLookAtLH(light.position, light.position + directions[face] * light.range, up[face]);
            shadowConstsCPU.shadowCubeMapViewProj[face] = (lightViewRow * lightProjRow).Transpose();
        }
        shadowConstsCPU.shadow_map_size = SHADOW_CUBE_SIZE;
        shadowConstsGPU->Update(m_context, shadowConstsCPU, sizeof(shadowConstsCPU));
    }

    SetSceneViewport(static_cast<float>(shadowCubeMapPass.width), static_cast<float>(shadowCubeMapPass.height));
    shadowCubeMapPass.BeginRenderPass(m_context);

    // Render Mesh
    {
        auto entityView = m_reg.view<Mesh, Material, Transform>(entt::exclude<Light>);
        for (auto& entity : entityView)
        {
            auto [mesh, material, transform] = entityView.get<Mesh, Material, Transform>(entity);
            ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthCubeMap)->Bind(m_context);

            objectConstsCPU.world = transform.currentTransform.Transpose();
            objectConstsCPU.worldInvTranspose = transform.currentTransform.Invert().Transpose();
            objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

            mesh.Draw(m_context);
        }
        ShaderManager::GetShaderProgram(ShaderProgram::ShadowDepthCubeMap)->Unbind(m_context);
    }
    shadowCubeMapPass.EndRenderPass(m_context);
}

void Renderer::PassAABB()
{
    SetSceneViewport(static_cast<float>(deferredLightingPass.width), static_cast<float>(deferredLightingPass.height));
    deferredLightingPass.BeginRenderPass(m_context, false, false);
    wireframeRS->Bind(m_context);
    noneDepthDSS->Bind(m_context, 0);
    // Mesh Render
    {
        auto aabbView = m_reg.view<AABB>();
        for (auto& entity : aabbView)
        {
            auto& aabb = aabbView.get<AABB>(entity);

            if (aabb.isDrawAABB)
            {
                ShaderManager::GetShaderProgram(ShaderProgram::Solid)->Bind(m_context);

                aabb.SetAABBIndexBuffer(m_device);

                objectConstsCPU.world = Matrix::Identity;
                objectConstsCPU.worldInvTranspose = Matrix::Identity;
                objectConstsGPU->Update(m_context, objectConstsCPU, sizeof(objectConstsCPU));

                materialConstsCPU.diffuse = Vector3(0.0f, 1.0f, 0.0f);
                materialConstsGPU->Update(m_context, materialConstsCPU, sizeof(materialConstsCPU));

                m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
                BindVertexBuffer(m_context, aabb.aabbVertexBuffer.get());
                BindIndexBuffer(m_context, aabb.aabbIndexBuffer.get());
                m_context->DrawIndexed(aabb.aabbIndexBuffer->GetCount(), 0, 0);
            }
        }
    }
    deferredLightingPass.EndRenderPass(m_context);
}

void Renderer::PassLight()
{
    SetSceneViewport(static_cast<float>(deferredLightingPass.width), static_cast<float>(deferredLightingPass.height));
    deferredLightingPass.BeginRenderPass(m_context, false, false);

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

} // namespace Riley