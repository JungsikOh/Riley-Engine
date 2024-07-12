#include "Core/Engine.h"
#include "Core/Window.h"
#include "Rendering/Renderer.h"
#include <assert.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog\spdlog.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Riley;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {

    WindowInit window_init{};
    window_init.instance = hInstance;
    window_init.width = (uint32)1080;
    window_init.height = (uint32)720;
    std::string window_title = std::string("Riley");
    window_init.title = window_title.c_str();
    window_init.maximize = false;
    Window window(window_init);
    g_Input.Initialize(&window);

    Renderer* renderer = new Renderer(&window, window.Width(), window.Height());

    // Main Loop
    while (window.Loop()) {
        renderer->Render();
        renderer->Present(true);
    }

    return 0;
}