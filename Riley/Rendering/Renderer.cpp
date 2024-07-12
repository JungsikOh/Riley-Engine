#include "Renderer.h"
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXStates.h"
#include "../Math/MathTypes.h"
#include "spdlog\spdlog.h"

namespace Riley {

Renderer::Renderer(Window* window, uint32 width, uint32 height)
    : m_window(window), m_width(width), m_height(height) {

    InitDirectX();

    // Logger
    spdlog::info("Renderer init complete {:f}s", timer.MarkInSeconds());
    timer.Mark();
}

Renderer::~Renderer() {
    SAFE_DELETE(m_backBufferDepthStencil);
    SAFE_DELETE(m_backBufferRTV);
    SAFE_RELEASE(m_swapChain);
}

void Renderer::Update(float dt) { m_currentDeltaTime = dt; }

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
    float clearColor[] = {0.1f, 0.2f, 0.6f, 1.0f};
    m_backBufferDepthStencil->Clear(m_context, 1.0f, 0);
    m_backBufferRTV->Clear(m_context, clearColor);
    m_backBufferRTV->BindRenderTargetView(m_context);
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
    sd.BufferDesc.RefreshRate.Numerator = 120;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.OutputWindow = (HWND)m_window->Handle();
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = 0;

    uint32 swapChainCreateFlags = 0;
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
    HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                              (void**)&backBuffer));
    HR(m_device->CreateRenderTargetView(backBuffer, nullptr, &rtv));

    SAFE_RELEASE(backBuffer);

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

} // namespace Riley