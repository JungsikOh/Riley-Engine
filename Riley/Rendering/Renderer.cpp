#include "Renderer.h"
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXShaderCompiler.h"
#include "../Graphics/DXShaderProgram.h"
#include "../Graphics/DXStates.h"
#include "../Math/ComputeVectors.h"
#include "../Math/MathTypes.h"
#include "ModelImporter.h"
#include "spdlog\spdlog.h"

namespace Riley {

Renderer::Renderer(Window* window, entt::registry& reg, ID3D11Device* device,
                   ID3D11DeviceContext* context, IDXGISwapChain* swapChain,
                   Camera* camera, uint32 width, uint32 height)
    : m_window(window), m_reg(reg), m_camera(camera), m_device(device),
      m_context(context), m_swapChain(swapChain), m_width(width),
      m_height(height) {

    CreateBackBufferResources();
    ShaderManager::Initialize(m_device);
    m_solidRS = new DXRasterizerState(m_device, CullCCWDesc());
    m_solidDSS = new DXDepthStencilState(m_device, DefaultDepthDesc());
    CreateBuffers();

    SetSceneViewport(m_width, m_height);

    // Logger
    spdlog::info("Renderer init complete {:f}s", timer.MarkInSeconds());
    timer.Mark();
}

Renderer::~Renderer() {
    SAFE_DELETE(m_backBufferDepthStencil);
    SAFE_DELETE(m_backBufferRTV);
    SAFE_DELETE(frameBufferGPU);
    SAFE_DELETE(objectConstsGPU);
}

void Renderer::Update(float dt) { m_currentDeltaTime = dt; }

void Renderer::Render() {
    float clearColor[] = {0.5f, 0.2f, 0.2f, 1.0f};
    m_backBufferDepthStencil->Clear(m_context, 1.0f, 0);
    m_backBufferRTV->Clear(m_context, clearColor);
    m_currentSceneViewport.Bind(m_context);
    m_backBufferRTV->BindRenderTargetView(m_context);
    PassSolid();
}

void Renderer::OnResize(uint32 width, uint32 height) {
    if ((m_width != width || m_height != height) && width > 0 && height > 0) {
        m_width = width;
        m_height = height;
        if (m_swapChain != nullptr) {
            m_swapChain->ResizeBuffers(0, m_width, m_height,
                                       DXGI_FORMAT_UNKNOWN, 0);
            CreateBackBufferResources();
        }
        SetSceneViewport(width, height);
    }
}

void Renderer::Tick(Camera* camera) {
    BindGlobals();

    m_camera = camera;
    frameBufferCPU.globalAmbient = Vector4(0.2f, 0.3f, 0.8f, 1.0f);

    static uint32 frameIdx = 0;
    float jitterX = 0.0f, jitterY = 0.0f;

    frameBufferCPU.cameraNear = m_camera->Near();
    frameBufferCPU.cameraFar = m_camera->Far();
    frameBufferCPU.cameraJitterX = jitterX;
    frameBufferCPU.cameraJitterY = jitterY;
    frameBufferCPU.cameraPosition =
        Vector4(m_camera->Position().x, m_camera->Position().y,
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
    frameBufferGPU->Bind(m_context, DXShaderStage::VS, 0);

    // Set for next frame
    frameBufferCPU.prevView = m_camera->GetView();
    frameBufferCPU.prevProj = m_camera->GetProj();
    frameBufferCPU.prevViewProj = m_camera->GetViewProj();
    ++frameIdx;
}

void Renderer::BindGlobals() {
    static bool called = false;
    if (!called) {
        // VS
        frameBufferGPU->Bind(m_context, DXShaderStage::VS, 0);
        objectConstsGPU->Bind(m_context, DXShaderStage::VS, 1);

        // PS
        materialConstsGPU->Bind(m_context, DXShaderStage::PS, 0);
        called = true;
    }
}

void Renderer::SetSceneViewport(const float& width, const float& height,
                                const float& minDepth, const float& maxDepth,
                                const float& topLeftX, const float& topLeftY) {
    m_currentSceneViewport.SetWidth(width);
    m_currentSceneViewport.SetHeight(height);
    m_currentSceneViewport.SetMinDepth(minDepth);
    m_currentSceneViewport.SetMaxDepth(maxDepth);
    m_currentSceneViewport.SetTopLeftX(topLeftX);
    m_currentSceneViewport.SetTopLeftY(topLeftY);
}

void Renderer::CreateBackBufferResources() {
    if (m_backBufferRTV)
        SAFE_DELETE(m_backBufferRTV);
    if (m_backBufferDepthStencil)
        SAFE_DELETE(m_backBufferDepthStencil);

    ID3D11Texture2D* backBuffer = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                              (void**)&backBuffer));

    HR(m_device->CreateRenderTargetView(backBuffer, nullptr, &rtv));
    SAFE_RELEASE(backBuffer);

    m_backBufferDepthStencil =
        new DXDepthStencilBuffer(m_device, m_width, m_height);
    m_backBufferRTV =
        new DXRenderTarget(m_device, rtv, m_backBufferDepthStencil);
}

void Renderer::CreateBuffers() {
    frameBufferGPU = new DXConstantBuffer<FrameBufferConsts>(m_device, true);
    objectConstsGPU = new DXConstantBuffer<ObjectConsts>(m_device, true);
    materialConstsGPU = new DXConstantBuffer<MaterialConsts>(m_device, true);
}

void Renderer::PassSolid() {
    m_solidRS->Bind(m_context);
    m_solidDSS->Bind(m_context, 0);

    auto entity_view = m_reg.view<Mesh, Material, Transform>();
    for (auto& entity : entity_view) {
        auto [mesh, material, transform] =
            entity_view.get<Mesh, Material, Transform>(entity);

        ShaderManager::GetShaderProgram(material.shader)->Bind(m_context);
        objectConstsCPU.world = transform.currentTransform.Transpose();
        objectConstsCPU.worldInvTranspose =
            transform.currentTransform.Invert().Transpose();
        objectConstsGPU->Update(m_context, objectConstsCPU,
                                sizeof(objectConstsCPU));

        materialConstsCPU.diffuse = material.diffuse;
        materialConstsCPU.albedoFactor = material.albedoFactor;
        materialConstsGPU->Update(m_context, materialConstsCPU,
                                  sizeof(materialConstsCPU));

        mesh.Draw(m_context);
    }
}

} // namespace Riley