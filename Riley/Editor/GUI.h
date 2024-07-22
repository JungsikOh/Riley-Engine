#pragma once
#include <string>

namespace Riley {

class Window;
struct WindowEventData;

class GUI {
  public:
    explicit GUI(ID3D11Device* device, ID3D11DeviceContext* context,
                 Window* window);
    ~GUI();
    void Begin() const;
    void End() const;
    void HandleWindowMessage(WindowEventData const&) const;
    void ToggleVisibility();
    bool IsVisible() const;

  private:
    bool isVisible = true;
};
} // namespace Riley