#include "Engine.h"
#include "../Utilities/Timer.h"
#include "Window.h"

namespace Riley {
Engine::Engine(EngineInit const& init)
    : window(init.window), vsync{init.vsync} {}

Engine::~Engine() {}

void Engine::OnWindowEvent(WindowEventData const& data) {
    g_Input.OnWindowEvent(data);
}

void Engine::Run() {
    static RileyTimer timer;
    float const dt = timer.MarkInSeconds();

    g_Input.Tick();
    if (window->IsActive()) {
        Update(dt);
        Render();
    }
}

void Engine::Update(float dt) {}

void Engine::Render() {}

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//                   LPSTR lpCmdLine, int nShowCmd) {
//    WindowInit window_init{};
//    window_init.instance = hInstance;
//    window_init.width = 1080;
//    window_init.height = 720;
//    std::string window_title = "Riley";
//    window_init.title = window_title.c_str();
//    Window window(window_init);
//    g_Input.Initialize(&window);
//
//    while (window.Loop()) {
//    
//    }
//
//    return 0;
//}

} // namespace Riley