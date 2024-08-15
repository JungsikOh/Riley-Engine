#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/Log.h"
#include "Editor/Editor.h"
#include <assert.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog\spdlog.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Riley;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
   // memory new check, you can use writing memory leak number.
   //_CrtSetBreakAlloc(1243);

   Log::Initialize();

   WindowInit window_init{};
   window_init.instance = hInstance;
   window_init.width = (uint32)1080;
   window_init.height = (uint32)720;
   std::string window_title = std::string("Riley");
   window_init.title = window_title.c_str();
   window_init.maximize = false;

   Window window(window_init);
   g_Input.Initialize(&window);

   EngineInit engineInit{};
   engineInit.vsync = true;
   engineInit.window = &window;

   Editor editor(engineInit);
   window.GetWindowEvent().Add([&](WindowEventData const& msg_data) {
      editor.OnWindowEvent(msg_data);
   });

   // Main Loop
   while (window.Loop())
      {
         editor.Run();
      }
   return 0;
}