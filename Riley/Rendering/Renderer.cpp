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

Renderer::Renderer(Window* window, Camera* camera, uint32 width, uint32 height)
    : m_window(window), m_camera(camera), m_width(width), m_height(height) {

    CreateSwapChainAndDevice();
    CreateBackBufferResources();
    ShaderManager::Initialize(m_device);
    m_solidRS = new DXRasterizerState(m_device, CullCCWDesc());
    m_solidDSS = new DXDepthStencilState(m_device, DefaultDepthDesc());

    ModelImporter mi;
    m_mesh = mi.LoadSquare(*m_device, 0.3f);

    objectConstsCPU.worldRow =
        Matrix::CreateTranslation(Vector3(0.3f, 0.2f, 0.0f)).Transpose();
    objectConstsCPU.worldInvTransposeRow = objectConstsCPU.worldRow.Invert();
    objectConstsGPU =
        new DXConstantBuffer<ObjectConsts>(m_device, objectConstsCPU, true);

    frameBufferGPU =
        new DXConstantBuffer<FrameBufferConsts>(m_device, frameBufferCPU, true);

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

    SAFE_RELEASE(m_swapChain);
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

void Renderer::Present(bool vsync) {
    if (m_swapChain != nullptr) {
        m_swapChain->Present(vsync, 0);
    }
}

void Renderer::OnResize(uint32 width, uint32 height) {
    if ((m_width != width || m_height != height) && width > 0 && height > 0) {
        m_width = width;
        m_height = height;
        if (m_swapChain != nullptr) {
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

    frameBufferCPU.cameraNear = m_camera->GetNear();
    frameBufferCPU.cameraFar = m_camera->GetFar();
    frameBufferCPU.cameraJitterX = jitterX;
    frameBufferCPU.cameraJitterY = jitterY;
    frameBufferCPU.cameraPosition =
        Vector4(m_camera->GetTransform().GetPosition().x,
                m_camera->GetTransform().GetPosition().y,
                m_camera->GetTransform().GetPosition().z, 1.0f);
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
        frameBufferGPU->Bind(m_context, DXShaderStage::VS, 0);
        objectConstsGPU->Bind(m_context, DXShaderStage::VS, 1);
        called = true;
    }
}

void Renderer::PassSolid() {
    m_solidRS->Bind(m_context);
    m_solidDSS->Bind(m_context, 0);
    ShaderManager::GetShaderProgram(ShaderProgram::Solid)->Bind(m_context);
    m_mesh.Draw(m_context);
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

void Renderer::CreateSwapChainAndDevice() {
    /* Initalize swapChain */
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = m_width;
    sd.BufferDesc.Height = m_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.OutputWindow = static_cast<HWND>(m_window->Handle());
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = 0;

    UINT swapChainCreateFlags = 0;
#if defined(_DEBUG)
    swapChainCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;

    HR(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                     swapChainCreateFlags, featureLevels, 1,
                                     D3D11_SDK_VERSION, &sd, &m_swapChain,
                                     &m_device, &featureLevel, &m_context));
}

void Renderer::CreateBackBufferResources() {
    if (m_backBufferRTV) {
        SAFE_DELETE(m_backBufferRTV);
    }
    if (m_backBufferDepthStencil)
        SAFE_DELETE(m_backBufferDepthStencil);
    
    m_swapChain->ResizeBuffers(0, m_width, m_height, DXGI_FORMAT_UNKNOWN, 0);
    std::cout << m_width << std::endl;

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

} // namespace Riley