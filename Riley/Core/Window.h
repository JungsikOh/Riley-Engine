#pragma once
#include "../Utilities/Delegate.h"
#include "Windows.h"

namespace Riley {
struct WindowEventData {
    void* handle = nullptr;
    uint32 msg = 0;
    uint64 wparam = 0;
    int64 lparam = 0;
    float width = 0.0f;
    float height = 0.0f;
};

struct WindowInit {
    HINSTANCE instance;
    char const* title;
    uint32 width, height;
    bool maximize;
};

// WindowEvent Class
DECLARE_EVENT(WindowEvent, Window, WindowEventData const&);

class Window {
    friend LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam,
                                    LPARAM lParam);

  public:
    Window(WindowInit const& init);
    ~Window();

    uint32 Width() const;
    uint32 Height() const;

    uint32 PositionX() const;
    uint32 PositionY() const;

    bool Loop();
    void Quit(int32 exit_code);

    void* Handle() const;
    bool IsActive() const;

    WindowEvent& GetWindowEvent() { return window_event; }

  private:
    HINSTANCE hinstance = nullptr;
    HWND hwnd = nullptr;
    WindowEvent window_event;

  private:
    void BroadcastEvent(WindowEventData const&);
};

} // namespace Riley