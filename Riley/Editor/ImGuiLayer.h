#pragma once
#include "../Core/Input.h"
#include "../Core/Layer.h"

namespace Riley {

class Engine;

class ImGuiLayer : public Layer {
  public:
    ImGuiLayer() = default;
    ImGuiLayer(Engine* engine);
    ~ImGuiLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    void Begin();
    void End();

    void SetDarkThemeColors();

    void HandleWindowMessage(WindowEventData const& data) const;

    uint32_t GetActiveWidgetID() const;

  private:
    Engine* engine;
};
} // namespace Riley