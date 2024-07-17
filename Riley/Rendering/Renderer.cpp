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

Renderer::Renderer(Window* window, uint32 width, uint32 height)
    : m_window(window), m_width(width), m_height(height) {

    InitDirectX();
    ShaderManager::Initialize(m_device);

    ModelImporter mi;
    m_mesh = mi.LoadSquare(*m_device, 0.3f);

    objectConstsCPU.worldRow =
        Matrix::CreateTranslation(Vector3(0.3f, 0.2f, 0.0f)).Transpose();
    objectConstsCPU.worldInvTransposeRow = objectConstsCPU.worldRow.Invert();
    objectConstsGPU =
        new DXConstantBuffer<ObjectConsts>(m_device, objectConstsCPU, true);

    objectConstsCPU.worldRow =
        Matrix::CreateTranslation(Vector3(0.7f, 0.2f, 0.0f)).Transpose();
    objectConstsGPU->Update(m_context, objectConstsCPU,
                            sizeof(objectConstsCPU));

    CameraParameters cp;
    cp.fov = 90.0f;
    cp.aspectRatio = 16.0f / 9.0f;
    cp.nearPlane = 0.05f;
    cp.farPlane = 50.0f;
    cp.transform = Transform(Vector3(0.5f, 0.0f, -1.0f),
                             Quaternion(Quaternion::CreateFromAxisAngle(
                                 Vector3(-1.0f, 0.0f, 0.0f), 0.0f)));
    m_camera = new Camera(cp);

    frameBufferGPU =
        new DXConstantBuffer<FrameBufferConsts>(m_device, frameBufferCPU);

    SetSceneViewport(m_window->Width(), m_window->Height());

    // Rasterizer States
    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;
    rastDesc.MultisampleEnable = false;
    HR(m_device->CreateRasterizerState(&rastDesc, &solidRS));

    D3D11_DEPTH_STENCIL_DESC dsDesc;
    ZeroMemory(&dsDesc, sizeof(dsDesc));
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = false; // Stencil 불필요
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    // 앞면에 대해서 어떻게 작동할지 설정
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    // 뒷면에 대해 어떻게 작동할지 설정 (뒷면도 그릴 경우)
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    HR(m_device->CreateDepthStencilState(&dsDesc, &solidDSS));

    // Logger
    spdlog::info("Renderer init complete {:f}s", timer.MarkInSeconds());
    timer.Mark();
}

Renderer::~Renderer() {
    SAFE_DELETE(m_backBufferDepthStencil);
    SAFE_DELETE(m_backBufferRTV);
    SAFE_RELEASE(m_swapChain);

    SAFE_DELETE(frameBufferGPU);
}

void Renderer::Update(float dt) {
    m_currentDeltaTime = dt;
    m_camera->Tick(dt);
    Tick(m_camera);
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

void Renderer::Render() {
    float clearColor[] = {0.5f, 0.2f, 0.2f, 1.0f};
    m_backBufferDepthStencil->Clear(m_context, 1.0f, 0);
    m_backBufferRTV->Clear(m_context, clearColor);
    m_currentSceneViewport.Bind(m_context);
    m_backBufferRTV->BindRenderTargetView(m_context);
    m_context->RSSetState(solidRS);
    m_context->OMSetDepthStencilState(solidDSS, 0);
    PassSolid();
}

void Renderer::Present(bool vsync) {
    if (m_swapChain != nullptr) {
        m_swapChain->Present(vsync, 0);
    }
}

void Renderer::InitDirectX() {
    /* Initalize swapChain */
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = m_window->Width();
    sd.BufferDesc.Height = m_window->Height();
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

    ID3D11Texture2D* backBuffer = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    HR(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
    HR(m_device->CreateRenderTargetView(backBuffer, nullptr, &rtv));

    m_backBufferDepthStencil = new DXDepthStencilBuffer(
        m_device, m_window->Width(), m_window->Height());
    m_backBufferRTV =
        new DXRenderTarget(m_device, rtv, m_backBufferDepthStencil);
}

void Renderer::OnResize(uint32 width, uint32 height) {
    m_width = width;
    m_height = height;
    // @todo : 렌더러의 크기가 달라지면서 사용된 버퍼의 크기도 달라져야하므로,
    // 버퍼 추가 시 함수 만들 것.
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
    ShaderManager::GetShaderProgram(ShaderProgram::Solid)->Bind(m_context);
    m_mesh.Draw(m_context);
}
} // namespace Riley